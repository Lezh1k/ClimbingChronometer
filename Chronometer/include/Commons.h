#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>
#define UNUSED(x) ((void)x)

enum SportsCategory {
  SC_NA = 0,    SC_3JUN,
  SC_2JUN,      SC_1JUN,
  SC_3ADULT,    SC_2ADULT,
  SC_1ADULT,    SC_CMS,
  SC_MS,        SC_MSIC,
};

const QString& sports_category_to_str(SportsCategory sc);

#endif // COMMONS_H
