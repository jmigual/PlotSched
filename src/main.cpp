#include "mainwindow.h"
#include <QApplication>
#include "event.h"
#include <QSettings>

#include <QDebug>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow * w;

  unsigned long startingTick = 0, finalTick = 999999999999;

  while (argc > 0) {
      if (QString(argv[0]) == "--delete-last-trace") {
          qDebug() << "deleting last selected trace";
          QSettings settings;
          settings.remove("lastPath");
      }
      else if (QString(argv[0]) == "-i") { // input trace file filename
          argv++;
          QSettings settings;
          settings.setValue("lastPath", QString(argv[0]));
      }
      else if (QString(argv[0]) == "-s")  { // starting tick to be considered in plot
          argv++;
          startingTick = QString(argv[0]).toLong();
      }
      else if (QString(argv[0]) == "-f")  { // final tick to be considered in plot
          argv++;
          finalTick = QString(argv[0]).toLong();
      }
      argc--; argv++;
  }

  qRegisterMetaType<Event>("Event");

  w = new MainWindow(startingTick, finalTick);
  w->show();

  return a.exec();
}
