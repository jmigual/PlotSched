#include "eventsmanager.h"
#include "mainwindow.h"
#include <eventview.h>

#include <QDebug>

EventsManager::EventsManager()
{
  last_event = 0;
  last_magnification = 1;
}


void EventsManager::clear()
{
//  events_container.clear();
  _tasksEvents.clear();
  _cpusEvents.clear();
  _tasks.clear();
  _cpus.clear();
  last_magnification = 1;
  last_event = 0;
}

void EventsManager::addFrequencyChangeEvents()
{
    for (const Island_BL* isl : _islands) {
        for (const auto& fc : isl->getFrequencies().toStdMap()) {
//            qDebug() << __func__ << " " << (isl->isBig() ? "big " : "little ") << fc.first << " " << fc.second << endl;
            for (CPU_BL* cpu : isl->getProcessors())
            {
                Event* e = new Event(fc.first, 0, cpu, NULL, "", FREQUENCY_CHANGE);
                _cpusEvents[cpu].push_back(e);
            }
        }
    }
}

void EventsManager::readTasks()
{
    QString filename = _folder_generaldata + "/tasks.txt";
    qDebug() << "Trying to read tasks generalities from " + filename;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error while reading from " << filename;
    }

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        QString name        = fields.at(0);
        unsigned int q      = QString(fields.at(1)).toUInt();
        unsigned int wcet   = QString(fields.at(2)).toUInt();
        unsigned int period = QString(fields.at(3)).toUInt();
        _tasks.push_back(new Task(name, q, wcet, period));
    }
    file.close();
}

void EventsManager::readCPUs()
{
    QString filename = _folder_generaldata + "/cpus.txt";
    qDebug() << "Trying to read CPUs generalities from " + filename;
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error while reading from " << filename;
    }

    Island_BL* lastIsland;
    QVector<CPU_BL*> cpus;

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        if (line.startsWith("BEGIN ISLAND")) {
            bool isbig = true;
            if (QString(fields.at(2)).trimmed() == "LITTLE")
                isbig = false;
            lastIsland = new Island_BL();
            lastIsland->setBig(isbig);
            _islands.push_back(lastIsland);
        }
        else if (line.startsWith("END ISLAND")) {
            lastIsland->setCPUs(cpus);
            cpus.clear();
        }
        else { // cpu
            QString name        = QString(fields.at(0)).trimmed();
            CPU_BL* c           = new CPU_BL(name, lastIsland);
            _cpus.push_back(c);
            cpus.push_back(c);
        }
    }
    file.close();
}


void EventsManager::newEventArrived(Event* e)
{
  Q_ASSERT (e->getTask() != NULL); Q_ASSERT (e->getCPU() != NULL);

  // is it the last event I found until now?
  if (e->getStart() > last_event)
    last_event = e->getStart();

  // store tasks and CPU
  if (_tasks.contains(e->getTask()) == false)
    _tasks.append(e->getTask());
  if (_cpus.contains(e->getCPU()) == false)
      _cpus.append(e->getCPU());

  // add event to queue. task -> list<events>
  _tasksEvents[e->getTask()].push_back(e); // todo can be optimized by using id instead of task
  _cpusEvents[e->getCPU()].push_back(e);
}

qreal EventsManager::magnify(qreal start, qreal end, qreal width)
{
  qreal new_center;
  qreal fraction;
  qreal size = end - start;
  qreal magnification = width / size;

  new_center = (start + end) / 2 / last_magnification;

  if (size > 0)
    fraction = magnification;
  else
    fraction = -last_magnification / magnification;

  last_magnification = fraction;

  for (QList<Event*> e : _tasksEvents.values())
      for (Event* ee : e)
          ee->setMagnification(fraction);

  qDebug() << "The magnification is : " << fraction;
  if (size > 0)
    qDebug() << "The new margins are : " << start * fraction << " " << end * fraction;

  new_center *= last_magnification;

  return new_center;
}
