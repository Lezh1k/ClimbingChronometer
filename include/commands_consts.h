#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum btn_commands {
  BCMD_INIT                = 0xa8,
  BCMD_INIT_ACK            = 0x8a,
  BCMD_START_COUNTDOWN     = 0xcd,  
  BCMD_INIT_STATE             = 0xae,
} btn_commands_t;

typedef enum btn_consts{
  BC_BTN_COUNT = 5,
  BC_BTN0       = 0b00000001,
  BC_BTN1       = 0b00000011,
  BC_PLATFORM0  = 0b00001000,
  BC_PLATFORM1  = 0b00011000,
  BC_BTN_START  = 0b01000000
} btn_consts_t;

#endif
