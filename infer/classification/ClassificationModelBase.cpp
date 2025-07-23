#include "ClassificationModelBase.h"
#include "SafeSectionLoader.h"
#include <QFile>
ClassificationModelBase *(
    *ClassificationModelBase::factory)(const QString &path) = nullptr;
QByteArray ClassificationModelBase::readNet(const QString &path) {
  using namespace std;
  QByteArray data;
#ifndef SAFE_SECTION
  QFile f(path);
  f.open(QFile::OpenModeFlag::ReadOnly);
  data = f.readAll();
#else
  SafeSectionLoader loader("", "");
  data = loader.loadData("");
#endif
  return data;
}
