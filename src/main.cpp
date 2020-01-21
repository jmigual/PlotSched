#include "mainwindow.h"
#include <QApplication>
#include "event.h"
#include <QSettings>
#include <limits>

#include <QDebug>

#define FN_SPEEDS_ODROID_XU3_BZIP2_LITTLE QCoreApplication::applicationDirPath() + "/speedsLittle_Odroid_bzip2.txt"
#define FN_SPEEDS_ODROID_XU3_BZIP2_BIG    QCoreApplication::applicationDirPath() + "/speedsBig_Odroid_bzip2.txt"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow * w;

  TICK startingTick = 0, finalTick = std::numeric_limits<unsigned int>::max();
  QString filename_frequencies_big = "",
          filename_frequencies_little = "",
          filename_tasks = "", filename_cpus = "",
          folderSpeedsLittle = FN_SPEEDS_ODROID_XU3_BZIP2_LITTLE,
          folderSpeedsBig    = FN_SPEEDS_ODROID_XU3_BZIP2_BIG;

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
      else if (QString(argv[0]) == "--filename-speed-big") {
          argv++;
          folderSpeedsBig = QString (argv[0]);
      }
      else if (QString(argv[0]) == "--filename-speed-little") {
          argv++;
          folderSpeedsLittle = QString (argv[0]);
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

  Island_BL::readFrequencySpeed(folderSpeedsBig, QString("big"));
  Island_BL::readFrequencySpeed(folderSpeedsLittle, QString("little"));

  w = new MainWindow(startingTick, finalTick);
  w->show();


  // at this point no trace file (pst) has been opened

  return a.exec();
}
