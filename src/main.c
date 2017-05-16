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

typedef enum boolean {
  cb_false = 0,
  cb_true = 1
} bool_t;

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

static inline uint8_t btn0_is_down()      { return !(PIND & PIN_BTN0); }
static inline uint8_t btn1_is_down()      { return !(PIND & PIN_BTN1); }
static inline uint8_t plt0_is_down()      { return !(PIND & PIN_PLATFORM0); }
static inline uint8_t plt1_is_down()      { return !(PIND & PIN_PLATFORM1); }
static inline uint8_t btn_start_is_down() { return !(PIND & PIN_BTN_START); }

static inline uint8_t btn0_is_up()      { return PIND & PIN_BTN0; }
static inline uint8_t btn1_is_up()      { return PIND & PIN_BTN1; }
static inline uint8_t plt0_is_up()      { return PIND & PIN_PLATFORM0; }
static inline uint8_t plt1_is_up()      { return PIND & PIN_PLATFORM1; }
static inline uint8_t btn_start_is_up() { return PIND & PIN_BTN_START; }

static inline void enable_usart_rx_int()      { UCSRB |= (1 << RXCIE); }
static inline void disable_usart_rx_int()     { UCSRB &= ~(1 << RXCIE); }
static inline void enable_timer0_ocra_int()   { TIMSK |= (1 << OCIE0A); }
static inline void disable_timer0_ocra_int()  { TIMSK &= ~(1 << OCIE0A); }

static inline void wait_for_transmitter() { while ( !(UCSRA & (1 << UDRE)) ) ; }

volatile uint8_t rx_buff = 0;
volatile uint8_t btn_start_in_progress = cb_false;

static void send_tx(uint8_t val) {
  wait_for_transmitter();
  UDR = val;
}
//////////////////////////////////////////////////////////////

ISR(TIMER0_COMPA_vect) {
  disable_timer0_ocra_int();
  if (btn_start_is_down())
    send_tx((uint8_t)BC_BTN_START);
}
//////////////////////////////////////////////////////////////

ISR(USART_RX_vect) {
  rx_buff = UDR;
}
//////////////////////////////////////////////////////////////

int
main(void) {
  register uint8_t btn01_coeff = 2;
  register uint8_t plt01_coeff = 2;

  register uint8_t tx_buff = 0; //contains just info about which key was pressed first
  register uint8_t btn0_pressed = cb_false;
  register uint8_t btn1_pressed = cb_false;

  register uint8_t is_pe = cb_false; //is platforms enabled
  register uint8_t plt0_pressed = cb_false;
  register uint8_t plt1_pressed = cb_false;

  DDRD = (1 << PD1) |
          PIN_BTN0 | PIN_BTN1 | PIN_PLATFORM0 |
          PIN_PLATFORM1 | PIN_BTN_START;
  DDRB =  PORT_LED0 | PORT_LED1 |
          PORT_LED2 | PORT_LED3;

  PORTD = PIN_BTN0 | PIN_BTN1 | PIN_PLATFORM0 |
      PIN_PLATFORM1 | PIN_BTN_START;
  PORTB = 0x00;

//  TIMER0
  TCCR0B = (1 << CS01); //use /8 prescaler
  OCR0A = 30; //30 microseconds

  //USART
  MCUCR = (1 << ISC11) | (1 << ISC01); //interrupt int0 on falling edge. interrupt int1 on falling edge.
  UCSRC = (1 << UCSZ0) | (1 << UCSZ1); //8bit, 1stop, async
  UBRRH = (uint8_t) (UBRR_VAL >> 8);
  UBRRL = (uint8_t) UBRR_VAL;
  UCSRB = (1 << RXEN) | (1 << TXEN); //enable receiver and transmitter
  GIMSK = 0;

  enable_usart_rx_int();
  sei();

  while(1) {
    switch (rx_buff) {
      case BCMD_RESTART:
        btn01_coeff = plt01_coeff = 2;
        btn_start_in_progress = is_pe =
            btn0_pressed = btn1_pressed =
            plt0_pressed = plt1_pressed = cb_false;
        led0_turn_off();
        led1_turn_off();
        led2_turn_off();
        led3_turn_off();        
        rx_buff = 0x00;
        break;
      case BCMD_INIT:
        rx_buff = 0x00;
        send_tx(BCMD_INIT_ACK);
        break;
      case BCMD_START_COUNTDOWN:        
        is_pe = 1;
        rx_buff = tx_buff = 0x00;
        send_tx(BCMD_START_COUNTDOWN_ACK);
        break;
      default:
        break;
    } //switch
    /*-------------------------------------*/

    if ( !btn0_pressed && btn0_is_down() ) {
      btn0_pressed = 1;
      tx_buff |= (BC_BTN0 << --btn01_coeff);
      send_tx(tx_buff);
      led0_turn_on();
    }

    if ( !btn1_pressed && btn1_is_down() ) {
      btn1_pressed = 1;
      tx_buff |= (BC_BTN1 << --btn01_coeff);
      send_tx(tx_buff);
      led1_turn_on();
    }
    /*-------------------------------------*/

    if ( !btn_start_in_progress && btn_start_is_down() ) {
      btn_start_in_progress = cb_true;
      TCNT0 = 0x00;
      enable_timer0_ocra_int();
    }
    /*-------------------------------------*/

    if (is_pe) {
      if ( !plt0_pressed && plt0_is_down() ) {
        plt0_pressed = 1;
        tx_buff |= (BC_PLATFORM0 << --plt01_coeff);
        send_tx(tx_buff);
        led2_turn_on();
      }

      if ( !plt1_pressed && plt1_is_down() ) {
        plt1_pressed = 3;
        tx_buff |= (BC_PLATFORM1 << --plt01_coeff);
        send_tx(tx_buff);
        led3_turn_on();
      }
    } //if (is_pe)
  } //while 1
} //main
//////////////////////////////////////////////////////////////
