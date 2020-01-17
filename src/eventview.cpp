#include "eventview.h"

#include <QGraphicsLineItem>
#include <QBrush>
#include <QGraphicsSimpleTextItem>
#include <QPen>

QColor EventView::eventToColor(EVENT_KIND e)
{
  switch (e) {
    case RUNNING : return Qt::green;
    case BLOCKED : return Qt::red;
    case CONFIGURATION : return Qt::yellow;
    default: return Qt::white;
  }
}

EventView::EventView(const Event* e, qreal offset, QGraphicsItem * parent) :
  QGraphicsItemGroup(parent)
{
  height = 30;
  vertical_offset = offset;

  setEvent(const_cast<Event*>(e));
}

EventView::~EventView()
{

}

/// Represent the event with a rectangle
void EventView::setEvent(Event* e)
{
  qDeleteAll(this->childItems());

  e_ = e;

  drawText();

  switch (e_->getKind()) {
    case RUNNING :
      rect = drawRect(e_->getDuration() * e_->getMagnification(), eventToColor(e_->getKind()));
      updateFgText();
      break;
    case BLOCKED :
      rect = drawRect(e_->getDuration() * e_->getMagnification(), eventToColor(e_->getKind()));
      updateFgText();
      break;
    case ACTIVATION :
      drawArrowUp();
      break;
    case DEADLINE :
      drawArrowDown();
      break;
    case CONFIGURATION :
      drawRectH(e_->getDuration() * e_->getMagnification(), eventToColor(e_->getKind()));
      break;
    case MISS :
      drawArrowDownRed();
      break;
    case DEAD :
      drawCircle();
      break;
    default: return;
  }

  this->moveBy(e_->getStart() * e_->getMagnification(), vertical_offset * e_->getRow());
}

void EventView::updateFgText() {
    if (_fgText == FG_FIELD::TASKANME)
        drawTextInRect(rect, e_->getTask()->name);
    else if (_fgText == FG_FIELD::CPUNAME)
        drawTextInRect(rect, e_->getCPU()->name);
}

void EventView::drawCircle()
{
  QGraphicsEllipseItem * body = new QGraphicsEllipseItem(-4,
                                                         -4,
                                                         4 * 2,
                                                         4 * 2,
                                                         this);
  this->addToGroup(body);
}


void EventView::drawArrowUp()
{
  /******************************
   *
   *                / (x2, y2)
   *              ///
   *            / / /
   *   (left) /  /  / (right)
   *            /
   *           /
   *          /
   *         / (x1, y1)
   *
   ******************************/

  // First of all, create an arrow with (x1, y1) = (0, 0)

  QGraphicsLineItem * body = new QGraphicsLineItem(0,
                                                   0,
                                                   0,
                                                   height,
                                                   this);
  QGraphicsLineItem * left = new QGraphicsLineItem(0,
                                                   0,
                                                   height / 5.0,
                                                   height / 4.0,
                                                   this);
  QGraphicsLineItem * right = new QGraphicsLineItem(0,
                                                    0,
                                                    -height / 5.0,
                                                    height / 4.0,
                                                    this);

  body->moveBy(0, -height);
  left->moveBy(0, -height);
  right->moveBy(0, -height);

  this->addToGroup(body);
  this->addToGroup(left);
  this->addToGroup(right);

  body->setPen(QPen(Qt::white));
  right->setPen(QPen(Qt::white));
  left->setPen(QPen(Qt::white));
}


void EventView::drawArrowDown()
{
  QGraphicsLineItem * body = new QGraphicsLineItem(0,
                                                   0,
                                                   0,
                                                   height,
                                                   this);
  QGraphicsLineItem * left = new QGraphicsLineItem(0,
                                                   height,
                                                   height / 5.0,
                                                   height * 3.0 / 4.0,
                                                   this);
  QGraphicsLineItem * right = new QGraphicsLineItem(0,
                                                    height,
                                                    -height / 5.0,
                                                    height * 3.0 / 4.0,
                                                    this);

  body->moveBy(0, -height);
  left->moveBy(0, -height);
  right->moveBy(0, -height);

  this->addToGroup(body);
  this->addToGroup(left);
  this->addToGroup(right);
}


void EventView::drawArrowDownRed()
{
  QGraphicsLineItem * body = new QGraphicsLineItem(0,
                                                   0,
                                                   0,
                                                   height,
                                                   this);
  QGraphicsLineItem * left = new QGraphicsLineItem(0,
                                                   height,
                                                   height / 5.0,
                                                   height * 3.0 / 4.0,
                                                   this);
  QGraphicsLineItem * right = new QGraphicsLineItem(0,
                                                    height,
                                                    -height / 5.0,
                                                    height * 3.0 / 4.0,
                                                    this);

  QPen p;
  p.setColor(Qt::red);
  p.setWidth(2);

  body->setPen(p);
  left->setPen(p);
  right->setPen(p);

  body->moveBy(0, -height);
  left->moveBy(0, -height);
  right->moveBy(0, -height);

  this->addToGroup(body);
  this->addToGroup(left);
  this->addToGroup(right);
}


QGraphicsRectItem *EventView::drawRect(qreal duration, QColor color)
{
  /******************************
   *
   *          ____________
   *         |            |
   *         |            | height
   *         |____________|
   *  (x1, y1)  duration
   *
   ******************************/

  qreal rectHeight = height / 1.9;

  QGraphicsRectItem * r = new QGraphicsRectItem(0,
                                                0,
                                                duration,
                                                rectHeight,
                                                this);
  r->setBrush(QBrush(color));

  r->moveBy(0, -rectHeight);

  this->addToGroup(r);
  return r;
}


void EventView::drawRectH(qreal duration, QColor color)
{
  /******************************
   *
   *          ____________
   *         |            |
   *         |            | height
   *         |____________|
   *  (x1, y1)  duration
   *
   ******************************/

  qreal rectHeight = height / 1.5;

  QGraphicsRectItem * r = new QGraphicsRectItem(0,
                                                0,
                                                duration,
                                                rectHeight,
                                                this);
  r->setBrush(QBrush(color));

  r->moveBy(0, -rectHeight);

  this->addToGroup(r);
}

/// Texts outside the rectangles
void EventView::drawText()
{
  QGraphicsSimpleTextItem * start = new QGraphicsSimpleTextItem(QString::number(e_->getStart()),
                                                                this);
  start->setPos(0,0);
  start->setPen(QPen(Qt::white));
  this->addToGroup(start);

  if (e_->getDuration() > 0) {
    QGraphicsSimpleTextItem * end = new QGraphicsSimpleTextItem(QString::number(e_->getStart() + e_->getDuration()),
                                                                this);
    end->setPos(0,0);
    end->moveBy(e_->getDuration() * e_->getMagnification(), 0);
    this->addToGroup(end);
  }
}

// adds text inside rect
void EventView::drawTextInRect(QGraphicsRectItem *rect, const QString& text)
{
    QGraphicsSimpleTextItem * start = new QGraphicsSimpleTextItem(text, this);
    start->setPos(rect->pos().x() + 3, rect->pos().y());
    this->addToGroup(start);
}
