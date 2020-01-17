#ifndef EVENTVIEW_H
#define EVENTVIEW_H

#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include "event.h"

/**
 * This class represents an event to be shown,
 * which can be a change of frequency, a task end event,
 * a job execution from t1 to t2, etc.
 */
class EventView : public QGraphicsItemGroup
{
public:
  enum FG_FIELD { TASKANME, CPUNAME, NONE };

private:
  qreal height;
  qreal vertical_offset;
  Event * e_;

  /// The information to be shown when you click on the event
  QString _info = "";

  /// The text to be shown in the foreground (the corresponding event field will be taken)
  FG_FIELD _fgText = FG_FIELD::NONE;

  /// Rectangle representing the event
  QGraphicsRectItem* rect;



  void setEvent(Event* e);
  QColor eventToColor(EVENT_KIND e);
  void drawText();

public:
  explicit EventView(const Event* e, qreal offset = 50, QGraphicsItem * parent = 0);
  ~EventView();

  void drawArrowUp();
  void drawArrowDown();
  void drawArrowDownRed();
  QGraphicsRectItem *drawRect(qreal duration, QColor color);
  void drawRectH(qreal duration, QColor color);
  void drawCircle();
  void setInfo(QString info) { _info = info; }
  void setFgTextType(FG_FIELD field) { _fgText = field; updateFgText(); }

  /// updates the text inside the rectangle representing the event
  void updateFgText();
protected:
  void drawTextInRect(QGraphicsRectItem *rect, const QString &text);
};

#endif // EVENTVIEW_H
