#include "eventview.h"

#include <QGraphicsLineItem>
#include <QBrush>
#include <QGraphicsSimpleTextItem>
#include <QPen>
#include <cmath>

#define TWISTED_ARROW_FILENAME ":/icons/assets/twisted_arrow64x.png"

QColor EventView::eventToColor(EVENT_KIND e)
{
  switch (e) {
    case RUNNING : return Qt::darkGreen;
    case BLOCKED : return Qt::darkRed;
    case CONFIGURATION : return Qt::darkYellow;
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
      {
        rect = drawRect(e_->getDuration() * e_->getMagnification(), eventToColor(e_->getKind()));
        if (e_->hasFinished() == false)
            rect->setBrush(QBrush(Qt::green));
      }
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
      rect = drawRect(e_->getDuration() * e_->getMagnification(), eventToColor(e_->getKind()));
      drawCircle();
      break;
    case FREQUENCY_CHANGE:
//      qDebug() << e_->print();
      drawTextAboveEvent(drawArrowTwisted(), QString::number(dynamic_cast<CPU_BL*>(e_->getCPU())->getIsland()->getFrequencyAt(e_->getStart())));
      break;
    default: return;
  }

  this->moveBy(e_->getStart() * e_->getMagnification(), vertical_offset * e_->getRow());
}

void EventView::updateFgText() {
    if (e_->getKind() == FREQUENCY_CHANGE)
        return;

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

QGraphicsPixmapItem* EventView::drawArrowTwisted() {
    QImage image = QImage(TWISTED_ARROW_FILENAME);
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    item->moveBy(e_->getStart(), 0);
    this->addToGroup(item);
    return item;
}

RectItemShowingInfo *EventView::drawRect(qreal duration, QColor color)
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

  RectItemShowingInfo * r = new RectItemShowingInfo(0,
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

  RectItemShowingInfo * r = new RectItemShowingInfo(0,
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

void EventView::drawTextAboveEvent(QGraphicsItem* item, const QString& text)
{
    QGraphicsSimpleTextItem * start = new QGraphicsSimpleTextItem(text, this);
    start->setPen(QPen(Qt::blue));
    start->setPos(item->pos().x(), item->pos().y());
    this->addToGroup(start);
}

// adds text inside rect
void EventView::drawTextInRect(QGraphicsRectItem *rect, const QString& text)
{
    QGraphicsSimpleTextItem * start = new QGraphicsSimpleTextItem(text, this);
    start->setPos(rect->pos().x() + 3, rect->pos().y());
    this->addToGroup(start);
}


// ------------------------------------ RectItemShowingInfo

void RectItemShowingInfo::onClicked(QGraphicsSceneMouseEvent* e, EventView* eventview)
{    
    QVector<QPair<TICK, double>> frequenciesInRange;
    QString frequenciesInRangeStr = "", info = "";
    TICK execdCycles = 0;
    double speed = 0;
    Event* e_ = eventview->getEvent();

    TICK start = e_->getStart();
    TICK end   = e_->getDuration() + start;
    TICK delta = e_->getDuration();

    if (e_->getKind() == FREQUENCY_CHANGE)
        return;

    info = e_->getTask()->name + "\n";
    info += QString("end - start = %1 - %2 = %3 (time)\n").arg(end).arg(start).arg(delta);
    info += "\n";

    CPU_BL* cpubl = dynamic_cast<CPU_BL*>(e_->getCPU());
    if (cpubl != NULL) {
        info += "time\t->\tfrequency\tspeed\n";

        frequenciesInRange = cpubl->getIsland()->getFrequenciesOverTimeInRange(start, end);
        std::reverse(frequenciesInRange.begin(), frequenciesInRange.end());
        if (frequenciesInRange.size() == 0) {
            info += "Could not find any frequency. You'll need to figure it out in your RTSim log file\n";
        }

        TICK last_time = e_->getDuration() + e_->getStart();
        for (QPair<TICK, double> freqs : frequenciesInRange) {
            speed = cpubl->getIsland()->getSpeed(freqs.second);

            //  time -> frequency
            info += QString("%1\t->\t%2\t%3\n").arg(freqs.first).arg(freqs.second).arg(speed);

            // (t_freq_change - t_freq_change2) * speed_freq2
            frequenciesInRangeStr += QString("(%1 - %2) * %3 +\n").arg(last_time).arg(freqs.first).arg(speed);

            execdCycles += std::ceil( (last_time - freqs.first) * speed );
            last_time = freqs.first;
        }
        frequenciesInRangeStr =  frequenciesInRangeStr.trimmed();
        frequenciesInRangeStr =  frequenciesInRangeStr.remove(frequenciesInRangeStr.length() - 1, 1);

        info += "\n";
        info += "execd cycles:\n" + frequenciesInRangeStr + " = " + QString::number(execdCycles);
        info += "\n";
    }

    QMessageBox msgBox;
    msgBox.setText(info);
    msgBox.exec();

    QGraphicsRectItem::mousePressEvent(e); // event propagation
}
