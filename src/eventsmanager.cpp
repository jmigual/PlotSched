#include "eventsmanager.h"
#include "mainwindow.h"

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


void EventsManager::newEventArrived(Event* e)
{
//  qDebug() << QString::number(countEvents() + 1) << ". " << __func__ << " " << e->print();

//  QList<Event>::iterator i = events_container[e.getTask()->name].begin();
//  while (i != events_container[e.getTask()->name].end() && (*i).getStart() < e.getStart())
//    ++i;
//  events_container[e.getTask()->name].insert(i, e);

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

void EventsManager::onAllEventsAdded()
{
    this->mainWindow->updatePlot();
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

//  for (QMap<QString, QList<Event>>::iterator l = events_container.begin(); l != events_container.end(); ++l) {
//    for (QList<Event>::iterator i = (*l).begin(); i != (*l).end(); ++i)
//      (*i).setMagnification(fraction);
//  }
  for (QList<Event*> e : _tasksEvents.values())
      for (Event* ee : e)
          ee->setMagnification(fraction);

  qDebug() << "The magnification is : " << fraction;
  if (size > 0)
    qDebug() << "The new margins are : " << start * fraction << " " << end * fraction;

  new_center *= last_magnification;

  return new_center;
}
