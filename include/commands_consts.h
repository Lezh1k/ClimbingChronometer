#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum btn_commands {
  BCMD_INIT     = 0xa8,
  BCMD_INIT_ACK = 0x8a,
  BCMD_RESTART  = 0xae,
} btn_commands_t;

typedef enum btn_consts{
  BC_BTN_COUNT = 5,
  BC_BTN0 = 3,
  BC_BTN1 = 7,
  BC_BTN2 = 11,
  BC_BTN3 = 13,
} btn_consts_t;

#endif
