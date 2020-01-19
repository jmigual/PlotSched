#include "mainwindow.h"
#include <QApplication>
#include "event.h"
#include <QSettings>
#include <limits>

#include <QDebug>

#define FN_SPEEDS_ODROID_XU3_BZIP2_LITTLE "assets/speedsLittle_Odroid_bzip2.txt"
#define FN_SPEEDS_ODROID_XU3_BZIP2_BIG    "assets/speedsBig_Odroid_bzip2.txt"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow * w;

  TICK startingTick = 0, finalTick = std::numeric_limits<unsigned int>::max();
  QString filename_frequencies_big = "",
          filename_frequencies_little = "",
          filename_tasks = "", filename_cpus = "";

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
      else if (QString(argv[0]) == "--folder-frequencies") {
          argv++;
          EVENTSPARSER.setFolderFrequencies(QString(argv[0]));
      }
      else if (QString(argv[0]) == "--folder-generaldata") {
          argv++;
          EVENTSMANAGER.setFolderGeneralData(QString(argv[0]));
      }
      else if (QString(argv[0]) == "--filename-speed-big") {
          argv++;
          Island_BL::readFrequencySpeed(QString(argv[0]), QString("big"));
      }
      else if (QString(argv[0]) == "--filename-speed-little") {
          argv++;
          Island_BL::readFrequencySpeed(QString(argv[0]), QString("little"));
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


  // at this point no trace file (pst) has been opened

  return a.exec();
}
