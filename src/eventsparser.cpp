#include "eventsparser.h"
#include "eventsmanager.h"
#include <QDebug>
#include <QList>

void EventsParser::parseFile(QString path)
{
    qDebug() << __func__ << "()";

  QString result;
  QFile f(path);

  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    while (!f.atEnd()) {
      Event* e = new Event();
      e->parse(f.readLine());
      if (e->isCorrect()) {
        EVENTSMANAGER.newEventArrived(e);
      }
    }
  }

//  emit fileParsed();
    qDebug() << "---------------------------------------";
    qDebug() << "Read lines from file: " << QString::number(EVENTSPARSER.readLines);

    QMap<Task*, QList<Event*>> tasksEvents = EVENTSMANAGER.getAllTasksEvents();
    qDebug() << "Parser has finished its job. Found "
             << "#tasks: "  << EVENTSMANAGER.getTasks().size()
             << "#CPU: "    << EVENTSMANAGER.getCPUs().size()
             << "#events: " << EVENTSMANAGER.countEvents();

    qDebug() << "Tasks:";
    for (Task* e : EVENTSMANAGER.getTasks())
        qDebug() << e->print();
    qDebug() << "CPUs:";
    for (CPU* e : EVENTSMANAGER.getCPUs())
        qDebug() << e->print();
//    qDebug() << "Events:"; // heavy to print them out. Use for debug
//    for (auto& elem : tasksEvents.toStdMap()) {
//        qDebug() << elem.first->print();
//        QList<Event*> evts = elem.second;
//        for (Event* e : evts)
//            qDebug() << "\t" << e->print();
//    }
    qDebug() << "---------------------------------------";

    EVENTSMANAGER.onAllEventsAdded();

}
