#ifndef EVENTSPARSER_H
#define EVENTSPARSER_H

#include "event.h"

#include <QObject>
#include <QFile>
#include <QThread>
#include <QGraphicsItem>

#include <QDebug>

#define EVENTSPARSER EventsParser::getInstance()

class EventsManager;

// Singleton
class EventsParser : public QObject
{
  Q_OBJECT
  EventsManager* _em;

  /// first tick to be parsed
  TICK _startingTick = 0;

  /// last tick to be parsed. If 0, then this parameter is discarded
  TICK _finalTick = 9999999999;

private:

  // private constructor to prevent instantiations
  EventsParser() {}

  ~EventsParser() {
    qDebug() << __func__;
  }

  void completeSchedulingEvents();

public:
  /// Number of read lines from file
  unsigned int readLines = 0;

  // --------------------------------- singleton stuff

  static EventsParser& getInstance()
  {
      static EventsParser    instance; // Guaranteed to be destroyed.
                                       // Instantiated on first use.
      return instance;
  }

  EventsParser(EventsParser const&) : QObject()  { qDebug() << "do not call"; abort(); }   // Don't Implement
  void operator=(EventsParser const&) { qDebug() << "do not call"; abort(); }   // Don't implement


  // ---------------------------------- functions to get private fields

  void setStartingTick(TICK st) { _startingTick = st; }
  unsigned long getStartingTick() { return _startingTick; }

  void setFinalTick(TICK st) { _finalTick = st; }
  unsigned long getFinalTick() { return _finalTick; }

  // ---------------------------------- other functions

  void parseFile(QString filename);

  void parseFrequencies();

  void print() const;
};

#endif // EVENTSPARSER_H
