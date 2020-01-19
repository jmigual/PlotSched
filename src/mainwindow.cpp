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

MainWindow::MainWindow(TICK startingTick, TICK finalTick, QWidget *parent) :
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
    // todo delete the plotframe if already exists
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

// the menu on the left with filenames
void MainWindow::populate_dock()
{
  tfl = new TraceFileLister(this);
  this->addDockWidget(Qt::LeftDockWidgetArea, tfl, Qt::Vertical);

  connect(this, SIGNAL(newFolderChosen(QString)), tfl, SLOT(update(QString)));
  connect(tfl, SIGNAL(traceChosen(QString)), this, SLOT(newTraceChosen(QString)));
}

// the one above with icons
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
    EVENTSMANAGER.readCPUs();
    EVENTSMANAGER.readTasks();
    EVENTSPARSER.parseFile(path);
    EVENTSPARSER.parseFrequencies();
    EVENTSMANAGER.addFrequencyChangeEvents();
    updatePlot();
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

    EVENTSPARSER.print();

    if (_plotFrames[_currentView] != NULL) {
        if (_currentView == VIEWS::GANNT) { // default
            // CPU #0 |_____t1______t2_____...
            PlotFrame* plotFrame = new PlotFrame;
            _plotFrames[_currentView] = plotFrame;

            QMap <CPU*, QList<Event*>> m = EVENTSMANAGER.getAllCPUsEvents();
            QVector<QPair<CPU*, QList<Event*>>> msorted = QVector<QPair<CPU*, QList<Event*>>>(m.size());
            for (const auto& elem : m.toStdMap()) {
                QPair<CPU*, QList<Event*>> pair = QPair<CPU*, QList<Event*>>(elem.first, elem.second);
                msorted[QString(elem.first->name).toInt()] = pair;
            }
            Q_ASSERT (msorted.size() == m.keys().size());

            for (const auto& elem : msorted) {
                QList<Event*> l = elem.second;
                plotFrame->addRow(elem.first->name);

                for (Event* e : l) {
                  e->setRow(row);
//                  if (e->getKind() != FREQUENCY_CHANGE)
                      qDebug() << "dealing with " << e->print();
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
        else { // CORES

        }
    } // if _plotframe[current view] == NULL

    qreal rightmost = plot->updateSceneView(center);

    _plotFrames[_currentView]->setWidth(rightmost);
    plot->addNewItem(_plotFrames[_currentView]);

    plot->updateSceneView(center);

    qDebug() << "MainWindow::updatePlot()";
}
