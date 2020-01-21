#ifndef EVENT_H
#define EVENT_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>

#define TICK unsigned long

enum EVENT_KIND {
  ACTIVATION,
  DEAD,
  RUNNING,
  BLOCKED,
  DEADLINE,
  MISS,
  CONFIGURATION,
  FREQUENCY_CHANGE,
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

    /// tasks that the CPU holds at a given time, for each time (i.e., tick)
    QMap<TICK, QList<Task*>> tasksOverTime;

    /// CPU utilization and active utilizations over time. Active utilization defaults to 0.0
    QMap<TICK, QPair<double, double>> _utils;

    CPU(QString name) : name(name) {
        id = currentid;
        currentid++;
    }

    /// List of tasks at given tick t
    QList<Task*> getTasksAtTime(TICK tick) {
        return tasksOverTime.lowerBound(tick).value();
    }

    /// The utilization and active utilization (default 0.0) at tick t or the closest tick from the past
    QPair<double, double> getUtilizationAt(TICK t) { if (_utils.isEmpty()) return QPair<double, double>(0.0, 0.0); else return _utils.lowerBound(t).value(); }

    /// reads both utilizations and active utilizations over time
    void readUtilizationsOverTime(QString filename);

    QString print() const {
        return "CPU " + name + ", id: " + QString::number(id);
    }

    bool operator==(const CPU& other) { return id == other.id; }

    bool operator<(const  CPU& other) { return id < other.id; }
};


class Island_BL;
/// CPU big-little
class CPU_BL : public CPU {
protected:
    /// Island this core belongs to
    Island_BL* _island;

public:
    CPU_BL(QString name, Island_BL* island)
        : CPU(name), _island(island) {}

    Island_BL* getIsland() { return _island; }
    void setIsland(Island_BL* i) { _island = i; }

//    QVector<QPair<TICK, double>> getFrequenciesOverTimeInRange(TICK t1, TICK t2) { return _island->getFrequenciesOverTimeInRange(t1, t2); }

//    double getSpeed(double freq) const { return _island->getSpeed(freq); }
};

/// Island big-little
class Island_BL {
protected:
    /// Frequency over time for the island
    QMap<TICK, double> _frequencies;

    /// 200 MHz -> 0.00021, 1200 MHz -> 0.7777, etc.
    static QMap<double, double> _speeds_big, _speeds_little;

    /// CPUs composing this island
    QVector<CPU_BL*> _cpus;

    bool _isBig = true;

public:
    Island_BL() {}

    void setBig(bool isbig) { _isBig = isbig; }

    void setCPUs(QVector<CPU_BL*> cpus) { _cpus = cpus; }

    static void readFrequencySpeed(QString filenameSpeeds, QString island_name);

    void readFrequenciesOverTime(QString filenameFrequenciesOverTime);

    double getFrequencyAt(TICK t) const {
        return _frequencies.lowerBound(t).value();
    }

    QMap<TICK, double> getFrequencies() const { return _frequencies; }

    QVector<CPU_BL*> getProcessors() const { return _cpus; }

    bool isBig() const { return _isBig; }

    double getSpeed(double freq) const {
        Q_ASSERT(_speeds_big.size() > 0 && _speeds_little.size() > 0);
        double f = _speeds_big.toStdMap().at(freq);
        if (!isBig())
            f = _speeds_little.toStdMap().at(freq);
        return f;
    }

    void moveBackTicks(TICK minTick);

    QVector<QPair<TICK, double>> getFrequenciesOverTimeInRange(TICK t1, TICK t2);
};

class Event : public QObject
{
  Q_OBJECT

  TICK time_start;
  TICK duration;
  CPU* cpu;
  unsigned int row, column; ///row and column of the event
  Task* task;
  QString event;
  EVENT_KIND kind;
  QString status;

  // In case of bugs, tasks are scheduled but they don't end
  bool _hasFinished = true;

  qreal magnification;

  bool correct;
  bool pending;
  bool range;

  bool parseLine(QByteArray b);

public:
  Event();
  Event(const Event &o);
  Event(TICK time_start, TICK duration, CPU* cpu, Task* task, QString event, EVENT_KIND kind) {
      this->time_start = time_start;
      this->duration   = duration;
      this->cpu        = cpu;
      this->task       = task;
      this->event      = event;
      this->kind       = kind;
  }

  unsigned int getColumn() const { return column; }
  unsigned int getRow() const { return row; }
  void setColumn(unsigned int c) { column = c; }
  void setRow(unsigned int r) { row = r; }
  void setEvent(QString e) { event = e; }
  QString getEvent() const { return event; }
  void setStatus(QString s) { status = s; }
  QString getStatus() const { return status; }
  void setHasFinished(bool f) { _hasFinished = f; }
  bool hasFinished() const { return _hasFinished; }
  void setMagnification(qreal magnification) { this->magnification = magnification; }
  qreal getMagnification() const { return magnification; }
  bool isCorrect() const { return correct; }
  bool isPending() const { return pending; }
  bool isRange() const { return range; }
  TICK getStart() const { return time_start; }
  TICK getDuration() const { return duration; }
  void setStart(TICK t) { time_start = t; }
  void setDuration(TICK t) { duration = t; }
  Task* getTask() const { return task; }
  CPU* getCPU() const { return cpu; }
  EVENT_KIND getKind() const { return kind; }


  /// Fill this event fields given a line (string)
  void parse(QByteArray line);

  Event& operator=(const Event &o);

  QString print() const {
      QString s = QString("t=%1, %2 %3 for %4 on %5 dur %6").arg(QString::number(time_start), event, status, (task == NULL ? "":task->print()), (cpu == NULL?"":cpu->print()), QString::number(duration));
      return s;
  }

};


#endif // EVENT_H
