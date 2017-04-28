#ifndef COMMONS_H
#define COMMONS_H

#include <stdint.h>
#include <QString>
#define UNUSED(x) ((void)x)

enum SportsCategory {
  SC_NA = 0,   SC_3U,
  SC_2U,   SC_1U,
  SC_3A,   SC_2A,
  SC_1A,   SC_CMS,
  SC_MS,   SC_MSIC,
};

const QString& sports_category_to_str(SportsCategory sc);

#endif // COMMONS_H
