#ifndef EVENT_H
#define EVENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>

enum EVENT_KIND {
  ACTIVATION,
  DEAD,
  RUNNING,
  BLOCKED,
  DEADLINE,
  MISS,
  CONFIGURATION,
  NONE
};

static unsigned int currentid = 0;

class Task : public QObject {
    Q_OBJECT

public:
    /// task name
    QString name;

    /// task unique identifier
    unsigned int id;

    /// CBS budget
    unsigned int Q = 0;

    unsigned int WCET = 0, period = 0;

    Task(QString name = "", unsigned int q = 0, unsigned int wcet = 0, unsigned int period = 0) :
        name(name), Q(q), WCET(wcet), period(period) {
        id = currentid;
        currentid++;
    }

    QString print() const {
        return name + ", id: " + QString::number(id);
    }

    bool operator==(const Task& other) { return id == other.id; }

    bool operator<(const Task& other)  { return id < other.id; }

};

class CPU : public QObject {
    Q_OBJECT

public:
    /// CPU name
    QString name;

    /// CPU unique identifier
    unsigned int id;

    double utilization, utilization_active;

    /// tasks that the CPU holds at a given time, for each time (i.e., tick)
    QMap<unsigned long, QList<Task*>> tasksOverTime;

    CPU(QString name, double util, double util_active) :
       name(name), utilization(util), utilization_active(util_active) {
        id = currentid;
        currentid++;
    }

    QList<Task*> getTasksAtTime(unsigned int tick) {
        return tasksOverTime[tick];
    }

    QString print() const {
        return "CPU " + name + ", id: " + QString::number(id);
    }

    bool operator==(const CPU& other) { return id == other.id; }

    bool operator<(const  CPU& other) { return id < other.id; }
};

class Event : public QObject
{
  Q_OBJECT

  unsigned long time_start;
  unsigned long duration;
  CPU* cpu;
  unsigned long row, column; ///row and column of the event
  Task* task;
  QString event;
  EVENT_KIND kind;
  QString status;

  qreal magnification;

  bool correct;
  bool pending;
  bool range;

  bool parseLine(QByteArray b);

public:
  Event();
  Event(const Event &o);
  Event(unsigned long time_start, unsigned long duration, CPU* cpu, Task* task, QString& event, EVENT_KIND kind) {
      this->time_start = time_start;
      this->duration   = duration;
      this->cpu        = cpu;
      this->task       = task;
      this->event      = event;
      this->kind       = kind;
  }
  Event& operator=(const Event &o);
  void parse(QByteArray line);
  bool isCorrect();
  bool isPending();
  bool isRange();
  unsigned long getColumn() { return column; }
  unsigned long getRow() {return row; }
  void setColumn(unsigned long c) { column = c; }
  void setRow(unsigned long r) { row = r; }
  void setMagnification(qreal magnification) {this->magnification = magnification; }
  qreal getMagnification() { return magnification; }
  unsigned long getStart();
  unsigned long getDuration();
  Task* getTask();
  CPU* getCPU();
  EVENT_KIND getKind();

  QString print() const {
      QString s = QString("t=%1, %2 %3 for %4 on %5").arg(QString::number(time_start), event, status, task->print(), cpu->print());
      return s;
  }

};


#endif // EVENT_H
