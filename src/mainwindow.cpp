#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "customtoolbar.h"
#include "eventview.h"
#include "eventsmanager.h"
#include "eventsparser.h"

#include <QToolBar>
#include <QToolButton>
#include <QIcon>
#include <QFileDialog>
#include <QSettings>
#include <QShortcut>

#include <QDebug>

MainWindow::MainWindow(unsigned long startingTick, unsigned long finalTick, QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow()";
  ui->setupUi(this);

  setupShortcut();

  plot = new Plot(this);
  this->setCentralWidget(plot);

  populate_dock();
  populate_toolbar();


  EVENTSMANAGER.setMainWindow(this);
  EVENTSPARSER.setStartingTick(startingTick);
  EVENTSPARSER.setFinalTick(finalTick);

  this->_currentView = VIEWS::GANNT;
  loadSettings();

  showMaximized();
}

void MainWindow::loadSettings()
{
    QSettings settings;
    qDebug() << "settings path: " << settings.fileName();
    QFileInfo lastPath ( settings.value("lastPath", "").toString() );
    if (lastPath.isFile()) {
        newTraceChosen(lastPath.absoluteFilePath());
        tfl->update(lastPath.absoluteDir().absolutePath());
    }
}

void MainWindow::reloadTrace()
{
    this->newTraceChosen(curTrace);
}

void MainWindow::setupShortcut()
{
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(reloadTrace()));
}

void MainWindow::zoomChanged(qreal start, qreal end, qreal windowWidth)
{
  qreal center = EVENTSMANAGER.magnify(start, end, windowWidth);
  updatePlot(center);
}

// the menu on the left
void MainWindow::populate_dock()
{
  tfl = new TraceFileLister(this);
  this->addDockWidget(Qt::LeftDockWidgetArea, tfl, Qt::Vertical);

  connect(this, SIGNAL(newFolderChosen(QString)), tfl, SLOT(update(QString)));
  connect(tfl, SIGNAL(traceChosen(QString)), this, SLOT(newTraceChosen(QString)));
}

// the one above
void MainWindow::populate_toolbar()
{
  CustomToolBar * ct = new CustomToolBar(this);

  this->addToolBar(ct);

  connect(ct, SIGNAL(openButtonClicked()), this, SLOT(on_actionOpen_Folder_triggered()));
//  connect(ct, SIGNAL(refreshButtonClicked()), this, SLOT(on_actionRefresh_Folder_triggered()));
//  connect(ct, SIGNAL(zoomInClicked()), this, SLOT(on_actionZoomInTriggered()));
//  connect(ct, SIGNAL(zoomOutClicked()), this, SLOT(on_actionZoomOutTriggered()));
//  connect(ct, SIGNAL(zoomFitClicked()), this, SLOT(on_actionZoomFitTriggered()));
  connect(ct, SIGNAL(changeViewTasksClicked()), this, SLOT(on_actionViewChangedTasksTriggered()));
  connect(ct, SIGNAL(changeViewCPUClicked()), this, SLOT(on_actionViewChangedCPUTriggered()));
  connect(ct, SIGNAL(changeViewGanntClicked()), this, SLOT(on_actionViewChangedGanntTriggered()));
}

MainWindow::~MainWindow()
{
  qDebug() << __func__;
//  delete _ep;
  delete ui;
}


void MainWindow::updateTitle()
{
  QString t = "PlotSched";
  if (filename.length() > 0) {
    t.append(" - ");
    t.append(filename);
  }
  this->setWindowTitle(t);
}


void MainWindow::on_actionOpen_triggered()
{
  QString tmpfilename = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "./",
        "Plot Sched Trace (*.pst)"
        );

  filename = tmpfilename;
  updateTitle();
}


void MainWindow::on_actionQuit_triggered()
{
  close();
}


void MainWindow::on_actionOpen_Folder_triggered()
{
  QString tmpfilename = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        "./",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

  filename = tmpfilename;
  updateTitle();

  emit newFolderChosen(filename);
}

/// user chooses a .pst file
void MainWindow::newTraceChosen(QString path)
{
  qDebug() << "Chosen new trace : " << path;

  QFileInfo f(path);
  if (f.isFile()) {
    QSettings settings;
    settings.setValue("lastPath", path);
    this->curTrace = path;

    EVENTSMANAGER.clear();
    EVENTSPARSER.parseFile(path);
  }
}

void MainWindow::on_actionViewChangedTriggered(VIEWS newView)
{
    this->_currentView = newView;
    updatePlot();
}

void MainWindow::updatePlot(qreal center)
{
    qDebug() << "View updated: " + VIEWS_STR[_currentView];

    plot->clear();

    unsigned long row = 0;
    unsigned long column = 0; // the column I am dealing with

    if (_plotFrames[_currentView] != NULL) {
        if (_currentView == VIEWS::GANNT) {
            // CPU #0 |_____t1______t2_____...
            PlotFrame* plotFrame = new PlotFrame;
            _plotFrames[_currentView] = plotFrame;
            QMap <CPU*, QList<Event*>> m = EVENTSMANAGER.getAllCPUsEvents();
            for (const auto& elem : m.toStdMap()) {
                QList<Event*> l = elem.second;
                plotFrame->addRow(elem.first->name);

                for (Event* e : l) {
                  e->setRow(row);
    //              qDebug() << "dealing with " << e->print();
                  EventView * ev = new EventView(e);
                  if (e->getKind() != EVENT_KIND::ACTIVATION)
                      ev->setFgTextType(EventView::FG_FIELD::TASKANME);
                  plot->addNewItem(ev);
                }
                ++row;
            }
        }
        else if (_currentView == VIEWS::TASKS) {
            PlotFrame* plotFrame = new PlotFrame;
            _plotFrames[_currentView] = plotFrame;
            QMap <Task*, QList<Event*>> m = EVENTSMANAGER.getAllTasksEvents();
            for (QList<Event*> l : m.values()) {
                plotFrame->addRow(l.first()->getTask()->name);

                for (Event* e : l) {
                  e->setRow(row);
//                  qDebug() << "dealing with " << e->print();
                  EventView * ev = new EventView(e);
                  if (e->getKind() != EVENT_KIND::ACTIVATION)
                      ev->setFgTextType(EventView::FG_FIELD::CPUNAME);
                  plot->addNewItem(ev);
                }
                ++row;
            }
        }
        else { // CORES. Maybe the resulting code will be mergeable with the one above.
    //        QList<QString> cores = em.getCPUList();
    //        // for each core, show its ready and running tasks at time t
    //        for (QString core : cores) {
    //            QMap <QString, QList<QString>> * m = em.getTasks(core, 0); // BIG0 -> { T0 runs, T1 ready, T2 ready }
    //            pf->addColumn(core);
    //            for (QString task : m->values()) {
    //                  e.setColumn(column);
    //                  e.setRow(row);
    //                  EventView * ev = new EventView(e);
    //                  plot->addNewItem(ev);
    //                  row++;
    //            }
    //            column++;
    //        }
        }
    } // if _plotframe[current view] == NULL

    qreal rightmost = plot->updateSceneView(center);

    _plotFrames[_currentView]->setWidth(rightmost);
    plot->addNewItem(_plotFrames[_currentView]);

    plot->updateSceneView(center);

    qDebug() << "MainWindow::updatePlot()";
}
