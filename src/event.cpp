#include "event.h"
#include "eventsparser.h"

#include <QTextStream>
#include <QMap>
#include <eventsmanager.h>

#include <QDebug>

QMap<QString, QMap<EVENT_KIND, Event *>> pending_events;

bool Event::parseLine(QByteArray line)
{
  QString taskname, cpuname;
  correct = false;

  QTextStream ss(line);


  ss >> time_start;
  ss >> taskname;
  ss >> cpuname;
  ss >> event;
  ss >> status;

  EVENTSPARSER.readLines++;

  // warning, should you delete the task or cpu of an event you delete the task itself
  task = EVENTSMANAGER.getTaskByName(taskname);
  cpu  = EVENTSMANAGER.getCPUByName(cpuname); // tasks won't have sequential ID, they're shared with CPU's
  if (task == NULL)
      task = new Task(taskname);
  if (cpu == NULL)
      cpu  = new CPU(cpuname);

  if (time_start < EVENTSPARSER.getStartingTick() || time_start > EVENTSPARSER.getFinalTick())
      return false;

  if (event == "RUNNING") {
    kind = RUNNING;
  } else if (event == "DEAD") {
    kind = DEAD;
  }  else if (event == "BLOCKED") {
    kind = BLOCKED;
  } else if (event == "ACTIVATION" || event == "CREATION") {
    kind = ACTIVATION;
  } else if (event == "CONFIGURATION") {
    kind = CONFIGURATION;
  } else if (event == "DEADLINE") {
    kind = DEADLINE;
  } else if (event == "MISS") {
    kind = MISS;
  } else if (event == "FREQ_CHANGE") { // todo maybe useless
    kind = FREQUENCY_CHANGE;
  }

  if (status == "I") {
    correct = true;
    pending = false;
  } else if (status == "E") {
    pending = false;
    // this is an end event for a job. find its corresponding scheduling event to compute the duration
    if (pending_events[task->name].find(kind) != pending_events[task->name].end()) {
      duration = time_start - pending_events[task->name].find(kind).value()->getStart();
      this->time_start = pending_events[task->name].find(kind).value()->getStart();
      correct = true;

      Event * ev = pending_events[task->name].find(kind).value();
      pending_events[task->name].remove(kind);
      delete ev;
    }
  }else if (status == "S") {
    correct = true;
    pending = true;
    Event * ev = new Event(*this);
    pending_events[task->name].insert(kind, ev);
  }

  return true;
}


bool correctLine(QByteArray line)
{
  if (line.size() < 2)
    return false;
  return true;
}


Event::Event()
{
  magnification = 1;
  duration = 0;
  correct = false;
}


void Event::parse(QByteArray line)
{
  if (correctLine(line))
    parseLine(line);
}


Event::Event(const Event &o) : QObject()
{
  time_start = o.time_start;
  duration = o.duration;
  cpu = o.cpu;
  row = o.row;
  task = o.task;
  event = o.event;
  kind = o.kind;

  magnification = o.magnification;

  correct = o.correct;
  pending = o.pending;
  range = o.range;
}


Event& Event::operator=(const Event &o)
{
  time_start = o.time_start;
  duration = o.duration;
  cpu = o.cpu;
  row = o.row;
  task= o.task;
  event = o.event;
  kind = o.kind;

  magnification = o.magnification;

  correct = o.correct;
  pending = o.pending;
  range = o.range;

  return *this;
}

// -------------------------------- Island BL, CPU BL

QMap<double, double> Island_BL::_speeds_big, Island_BL::_speeds_little;

void Island_BL::readFrequencySpeed(QString filenameSpeeds, QString island_name) {
    // f1 speed bzip2
    // ...
    // fn speed bzip2

    qDebug() << "Trying to read frequency -> speed from " << filenameSpeeds;
    QFile file(filenameSpeeds);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error while reading from " << filenameSpeeds;
    }

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        double f0 = QString(fields.at(0)).toDouble();
        double f1 = QString(fields.at(1)).toDouble();
        if (island_name == "big")
            Island_BL::_speeds_big.insert(f0, f1);
        else
            Island_BL::_speeds_little.insert(f0, f1);
    }
    file.close();
}

void Island_BL::readFrequenciesOverTime(QString filenameFrequenciesOverTime)
{
    QFile file(filenameFrequenciesOverTime);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error while reading from " << filenameFrequenciesOverTime;
    }

    TICK minTick = EVENTSMANAGER.getMinimumSchedulingTick();
    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        TICK f0 = QString(fields.at(0)).toUInt();
        double f1 = QString(fields.at(2)).toDouble();
        if (f0 >= minTick)
            this->_frequencies.insert(f0, f1);
    }
    file.close();

    qDebug() << "Read # frequencies over time: " << _frequencies.size();
}

void Island_BL::moveBackTicks(unsigned long minTick)
{
    for (const auto& elem : _frequencies.toStdMap()) {
        if (elem.first >= minTick) {
            TICK t = elem.first - minTick;
            double f = elem.second;
            _frequencies.remove(elem.first);
            _frequencies[t] = f;
        }
    }
}

QVector<QPair<TICK, double>> Island_BL::getFrequenciesOverTimeInRange(TICK t1, TICK t2)
{
    QVector<QPair<TICK, double>> res;
    QPair<TICK, double> last_freq;

    for (const auto& elem : _frequencies.toStdMap()) {
        if (elem.first >= t1 && elem.first <= t2) {
            res.push_back(QPair<TICK, double>(elem.first, elem.second));
        }
        else if (elem.first < t1)
            last_freq = QPair<TICK, double>(t1, elem.second);
    }

    if (res.size() == 0) {
        // try with the speed right before t1
        res.push_back(last_freq);
    }

    return res;
}

void CPU::readUtilizationsOverTime(QString filename)
{
    // t1 cpuid util util_active
    // ...
    // tn cpuid util util_active

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug() << "error while reading from " << filename;
    }

    QTextStream in(&file);
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(" ");
        TICK f0 = QString(fields.at(0)).toUInt();
        unsigned int  f1 = QString(fields.at(1)).toUInt();
        double f2 = QString(fields.at(2)).toDouble();
//        double f3 = QString(fields.at(3)).toDouble();

        if (f1 == this->id)
            this->_utils.insert(f0, QPair<double, double>(f1, f2));
    }
    file.close();
}
