#include "ChronometerController.h"

#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "commands_consts.h"

CChronometerController::CChronometerController(QObject *parent) :
  QObject(parent),
  m_is_running(false),
  m_current_ms(0),
  m_timer(nullptr),
  m_time0_ms(0),
  m_time1_ms(0),
  m_time0_stopped(true),
  m_time1_stopped(true),
  m_serial_port(nullptr) {

  m_timer = new QTimer(this);
  m_timer->setInterval(10);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(ms_timer_timeout()));
}

CChronometerController::~CChronometerController() {
  if (m_serial_port) delete m_serial_port;
}
//////////////////////////////////////////////////////////////

bool
CChronometerController::start(QString& err) {
  static uint8_t restart_cmd[1] = {BCMD_RESTART};

  qint64 written = m_serial_port->write((char*)restart_cmd, 1);
  bool flushed = m_serial_port->flush();

  if (written != 1 || !flushed) {
    err = m_serial_port->errorString();
    return false;
  }
  m_time0_ms = m_time1_ms = m_current_ms = 0;
  m_is_running = true;
  m_time0_stopped = m_time1_stopped = false;
  m_time_start = std::chrono::high_resolution_clock::now();
  m_timer->start();
  emit state_changed(true);
  return true;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_all() {
  m_is_running = false;
  m_time0_stopped = m_time1_stopped = true;
  m_timer->stop();
  m_time_stop = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ms = diff.count() / 1000000;
  emit state_changed(false);
}
//////////////////////////////////////////////////////////////

void
CChronometerController::fall0() {
  if (m_time0_stopped) return;
  m_time0_stopped = true;
  m_time0_ms = FALL_TIME;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void
CChronometerController::fall1() {
  if (m_time1_stopped) return;
  m_time1_stopped = true;
  m_time1_ms = FALL_TIME;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_time0() {
  if (m_time0_stopped) return;
  m_time0_stopped = true;
  m_time0_ms = (m_time_stop - m_time_start).count() / 1000000;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_time1() {
  if (m_time1_stopped) return;
  m_time1_stopped = true;
  m_time1_ms = (m_time_stop - m_time_start).count() / 1000000;
  if (m_time0_stopped && m_time1_stopped) stop_all();
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

bool
CChronometerController::set_serial_port(const QSerialPortInfo &port_info,
                                        QString& err) {
  static const uint8_t cmd_init[1] = {BCMD_INIT};

  if (m_serial_port) {
    delete m_serial_port;
    m_serial_port = nullptr;
  }

  m_serial_port = new QSerialPort(port_info);
  m_serial_port->setBaudRate(9600);
  m_serial_port->setParity(QSerialPort::NoParity);
  m_serial_port->setDataBits(QSerialPort::Data8);
  m_serial_port->setStopBits(QSerialPort::OneStop);
  m_serial_port->setFlowControl(QSerialPort::NoFlowControl);
  if (!m_serial_port->open(QSerialPort::ReadWrite)) {
    err = m_serial_port->errorString();
    return false;
  }

  m_serial_port->write((char*)cmd_init, 1);
  m_serial_port->flush();
  if (!m_serial_port->waitForReadyRead(5000)) {
    err = m_serial_port->errorString();
    return false;
  }

  QByteArray answer = m_serial_port->readAll();
  if (answer.isEmpty()) {
    err = "Didn't receive answer from buttons controller";
    return false;
  }

  if ((uint8_t)answer.at(0) != BCMD_INIT_ACK) {
    err = QString("Received something different (not INIT_ACK) from controller. %1").
        arg((uint8_t)answer.at(0));
    return false;
  }

  connect(m_serial_port, SIGNAL(readyRead()), this, SLOT(serial_port_ready_read()));
  return true;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::ms_timer_timeout() {
  m_time_stop = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ms = diff.count() / 1000000;
  if (!m_time0_stopped) m_time0_ms = m_current_ms;
  if (!m_time1_stopped) m_time1_ms = m_current_ms;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::serial_port_ready_read() {
  QByteArray arr = m_serial_port->readAll();
  if (arr.isEmpty()) return;
  handle_rx((uint8_t)arr.at(0));
}
//////////////////////////////////////////////////////////////
