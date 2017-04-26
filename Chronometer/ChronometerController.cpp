#include "ChronometerController.h"

#include <QThread>
#include <QTimer>
#include "commands_consts.h"

CChronometerController::CChronometerController(QObject *parent) :
  QObject(parent),
  m_is_running(false),
  m_current_ms(0u),
  m_timer(nullptr),
  m_time0_ms(0u),
  m_time1_ms(0u),
  m_time0_stopped(true),
  m_time1_stopped(true) {

  m_timer = new QTimer(this);
  m_timer->setInterval(10);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(ms_timer_timeout()));
}

CChronometerController::~CChronometerController() {
}
//////////////////////////////////////////////////////////////

void
CChronometerController::start() {
  m_time0_ms = m_time1_ms = m_current_ms = 0u;
  m_is_running = true;
  m_time0_stopped = m_time1_stopped = false;
  m_time_start = std::chrono::high_resolution_clock::now();
  m_timer->start();
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_all() {
  m_is_running = false;
  m_time0_stopped = m_time1_stopped = true;
  m_timer->stop();
  m_time_stop = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ms = diff.count() / 1000000u;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_time0() {
  if (m_time0_stopped) return;
  m_time0_stopped = true;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_time1() {
  if (m_time1_stopped) return;
  m_time1_stopped = true;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::handle_rx(uint8_t rx) {
  switch (rx) {
    case ( BC_BTN0 << (BC_ORDER_COEFF-1) ):
      stop_time0();
      break;
    case ( (BC_BTN0 << (BC_ORDER_COEFF-1)) + (BC_BTN1 << (BC_ORDER_COEFF-2)) ):
      stop_time0();
      stop_time1();
      break;
    case ( BC_BTN1 << (BC_ORDER_COEFF-1) ):
      stop_time1();
      break;
    case ( (BC_BTN1 << (BC_ORDER_COEFF-1)) + (BC_BTN0 << (BC_ORDER_COEFF-2)) ):
      stop_time1();
      stop_time0();
      break;
    default:
      break;
  }
}
//////////////////////////////////////////////////////////////

void
CChronometerController::ms_timer_timeout() {
  m_time_stop = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ms = diff.count() / 1000000u;
  if (!m_time0_stopped) m_time0_ms = m_current_ms;
  if (!m_time1_stopped) m_time1_ms = m_current_ms;
}
//////////////////////////////////////////////////////////////
