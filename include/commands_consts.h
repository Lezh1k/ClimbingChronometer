#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum btn_commands {
  BCMD_INIT     = 0xa8,
  BCMD_INIT_ACK = 0x8a,
  BCMD_RESTART  = 0xae,
} btn_commands_t;

typedef enum btn_consts{
  BC_ORDER_COEFF = 3,
  BC_BTN0 = 4,
  BC_BTN1 = 6
} btn_consts_t;

#endif
