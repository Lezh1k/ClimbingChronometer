/*
 * ChronoButtons.c
 *
 * Created: 21:49 12.04.2017
 * Author : Lezh1k
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "commands_consts.h"

#define F_CPU       8000000UL
#define BAUD_RATE   9600UL
#define UBRR_VAL    (F_CPU / (16 * BAUD_RATE) - 1)

#define PIN_BTN0      (1 << PIND2)
#define PIN_BTN1      (1 << PIND3)
#define PIN_PLATFORM0 (1 << PIND4)
#define PIN_PLATFORM1 (1 << PIND5)
#define PIN_BTN_START (1 << PIND6)

#define PORT_LED0 (1 << PB0)
#define PORT_LED1 (1 << PB1)
#define PORT_LED2 (1 << PB2)
#define PORT_LED3 (1 << PB3)

static inline void led0_turn_on() { PORTB |= PORT_LED0 ; }
static inline void led0_turn_off() { PORTB &= ~PORT_LED0 ;}

static inline void led1_turn_on() { PORTB |= PORT_LED1 ; }
static inline void led1_turn_off() { PORTB &= ~PORT_LED1 ;}

static inline void led2_turn_on() { PORTB |= PORT_LED0 ; }
static inline void led2_turn_off() { PORTB &= ~PORT_LED0 ;}

static inline void led3_turn_on() { PORTB |= PORT_LED0 ; }
static inline void led3_turn_off() { PORTB &= ~PORT_LED0 ;}

static inline uint8_t btn0_is_down() { return !(PIND & PIN_BTN0); }
static inline uint8_t btn1_is_down() { return !(PIND & PIN_BTN1); }
static inline uint8_t plt0_is_down() { return !(PIND & PIN_PLATFORM0); }
static inline uint8_t plt1_is_down() { return !(PIND & PIN_PLATFORM1); }

static inline void enable_usart_rx_int() { UCSRB |= (1 << RXCIE); }
static inline void disable_usart_rx_int() { UCSRB &= ~(1 << RXCIE); }

static inline void wait_for_transmitter() { while ( !(UCSRA & (1 << UDRE)) ) ; }

//volatile uint8_t rx_buffer = 0;
register uint8_t rx_buffer asm("r4");

ISR(USART_RX_vect) {
  rx_buffer = UDR;
}
//////////////////////////////////////////////////////////////


static void send_tx(uint8_t val) {
  wait_for_transmitter();
  UDR = val;
}

int
main(void) {
  register uint8_t btn01_coeff = 2;
  register uint8_t plt01_coeff = 2;
  register uint8_t tx_buffer = 0; //contains just info about which key was pressed first

  register uint8_t btn0_pressed = 0;
  register uint8_t btn1_pressed = 0;
  register uint8_t plt0_pressed = 0;
  register uint8_t plt1_pressed = 0;
  rx_buffer = 0;

  DDRD = (1 << PD1) | PIN_BTN0 | PIN_BTN1 | PIN_PLATFORM0 | PIN_PLATFORM1;
  DDRB = PORT_LED0 | PORT_LED1 | PORT_LED2 | PORT_LED3;

  MCUCR = (1 << ISC11) | (1 << ISC01); //interrupt int0 on falling edge. interrupt int1 on falling edge.
  GIMSK = 0;
  UCSRC = (1 << UCSZ0) | (1 << UCSZ1); //8bit, 1stop, async
  UBRRH = (uint8_t) (UBRR_VAL >> 8);
  UBRRL = (uint8_t) UBRR_VAL;
  UCSRB = (1 << RXEN) | (1 << TXEN); //enable receiver and transmitter

  enable_usart_rx_int();
  sei();

  while(1) {
    switch (rx_buffer) {
      case BCMD_RESTART:
        btn01_coeff = plt01_coeff = 2;
        btn0_pressed = btn1_pressed =
            plt0_pressed = plt1_pressed = 0x00;
        led0_turn_off();
        led1_turn_off();
        led2_turn_off();
        led3_turn_off();
        rx_buffer = tx_buffer = 0x00;
        break;
      case BCMD_INIT:
        send_tx(BCMD_INIT_ACK);
        break;
      case BCMD_START_COUNTDOWN:
        //todo handle
        send_tx(BCMD_START_COUNTDOWN_ACK);
        break;
      default:
        break;
    } //switch

    if ( !btn0_pressed && btn0_is_down() ) {
      btn0_pressed = 1;
      tx_buffer |= (BC_BTN0 << --btn01_coeff);
      send_tx(tx_buffer);
      led0_turn_on();
    }

    if ( !btn1_pressed && btn1_is_down() ) {
      btn1_pressed = 1;
      tx_buffer |= (BC_BTN1 << --btn01_coeff);
      send_tx(tx_buffer);
      led1_turn_on();
    }

    if ( !plt0_pressed && plt0_is_down() ) {
      plt0_pressed = 1;
      tx_buffer |= (BC_PLATFORM0 << --plt01_coeff);
      send_tx(tx_buffer);
      led2_turn_on();
    }

    if ( !plt1_pressed && plt1_is_down() ) {
      plt1_pressed = 3;
      tx_buffer |= (BC_PLATFORM1 << --plt01_coeff);
      send_tx(tx_buffer);
      led3_turn_on();
    }
  } //while 1
} //main
//////////////////////////////////////////////////////////////
