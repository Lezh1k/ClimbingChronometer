#ifndef ATHLETE_H
#define ATHLETE_H

#include <QString>
#include <QDate>
#include "Commons.h"

struct CAthlete {
  QString name;
  SportsCategory category;
  QDate date;
};

#endif // ATHLETE_H
