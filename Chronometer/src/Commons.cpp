#include "Commons.h"

const QString &
sports_category_to_str(SportsCategory sc) {
  static const QString categories[] = {
    "Без разряда", "III юношеский", "II юношеский",
    "I юношеский", "III взрослый", "II взрослый",
    "I взрослый", "КМС", "МС", "МСМК"
  };
  return categories[sc];
}
