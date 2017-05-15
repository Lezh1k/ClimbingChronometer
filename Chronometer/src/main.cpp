#include "MainWindow.h"
#include <QApplication>
#include <math.h>
#include <QDebug>
#include <commands_consts.h>

uint8_t btns[] = {BC_BTN0, BC_BTN1, BC_PLATFORM0, BC_PLATFORM1};

uint8_t
tx_from_buttons( int8_t* buttons_in_down_state) {
  uint8_t tx_buffer = 0;

  for (int i = 0; i < BC_BTN_COUNT; ++i) {
    if (buttons_in_down_state[i] == -1) continue;
    tx_buffer |= btns[i] << (1 - buttons_in_down_state[i]);
  }
  return tx_buffer;
}
//////////////////////////////////////////////////////////////

int
main(int argc, char *argv[]) {
//  int8_t arr0[] = {0, -1, -1, -1};
//  int8_t arr1[] = {-1, 0, -1, -1};
//  int8_t arr2[] = {0, 1, -1, -1};
//  int8_t arr3[] = {1, 0, -1, -1};

//  int8_t arr4[] = {-1, -1, 0, -1};
//  int8_t arr5[] = {-1, -1, -1, 0};
//  int8_t arr6[] = {0, 1, 0, 1};
//  int8_t arr7[] = {1, 0, 1, 0};

//  int8_t* arr[] = {arr0, arr1, arr2, arr3, arr4, arr5, arr6, arr7, NULL};
//  for (int i = 0; arr[i]; ++i) {
//    qDebug() << tx_from_buttons(arr[i]) << " " << QString("%1").arg(tx_from_buttons(arr[i]), 8, 2);
//  }

  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  return a.exec();
}
