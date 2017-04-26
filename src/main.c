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

#define enable_usart_rx_int() (UCSRB |= (1 << RXCIE))
#define disable_usart_rx_int() (UCSRB &= ~(1 << RXCIE))

#define btn1_led_turn_on() PORTB |= (1 << PB1)
#define btn1_led_turn_off() PORTB &= ~(1 << PB1)
#define btn0_led_turn_on() PORTB |= (1 << PB0)
#define btn0_led_turn_off() PORTB &= ~(1 << PB0)

volatile uint8_t rx_buffer = 0;

ISR(USART_RX_vect) {
  rx_buffer = UDR;
}
//////////////////////////////////////////////////////////////

static inline void wait_for_transmitter() {
  while ( !(UCSRA & (1 << UDRE)) ) ;
}

static void send_tx(uint8_t val) {
  wait_for_transmitter();
  UDR = val;
}

int
main(void) {
  register uint8_t tx_buffer = 0; //contains just info about which key was pressed first
  register uint8_t tx_current_coeff = BC_ORDER_COEFF;
  register uint8_t btn0_pressed = 0;
  register uint8_t btn1_pressed = 0;

  DDRD = (1 << PD1);
  DDRB = (1 << PB0) | (1 << PB1);

  MCUCR = (1 << ISC11) | (1 << ISC01); //interrupt int0 on falling edge. interrupt int1 on falling edge.
  GIMSK = 0;

  UCSRC = (1 << UCSZ0) | (1 << UCSZ1); //8bit, 1stop, async
  UBRRH = (uint8_t) (UBRR_VAL >> 8);
  UBRRL = (uint8_t) UBRR_VAL;
  UCSRB = (1 << RXEN) | (1 << TXEN); //enable receiver and transmitter

  enable_usart_rx_int();
  sei();

  while(1) {
    if ( !btn0_pressed && !(PIND & (1 << PD2)) ) {
      btn0_pressed = 1;
      tx_buffer += (BC_BTN0 << (--tx_current_coeff));
      send_tx(tx_buffer);
      btn0_led_turn_on();
    }

    if ( !btn1_pressed && !(PIND & (1 << PD3)) ) {
      btn1_pressed = 1;
      tx_buffer += (BC_BTN1 << (--tx_current_coeff));
      send_tx(tx_buffer);
      btn1_led_turn_on();
    }

    switch (rx_buffer) {
      case BCMD_RESTART:
        tx_current_coeff = BC_ORDER_COEFF;
        btn0_pressed = btn1_pressed = 0;
        btn0_led_turn_off();
        btn1_led_turn_off();
        rx_buffer = tx_buffer = 0x00;
        break;
      case BCMD_INIT:
        tx_buffer = BCMD_INIT_ACK;
        send_tx(tx_buffer);
        rx_buffer = tx_buffer = 0x00;
        break;
      default:
        break;
    }
  } //while 1
}
//////////////////////////////////////////////////////////////
