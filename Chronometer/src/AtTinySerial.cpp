#include <QSerialPort>
#include <QSerialPortInfo>

#include "AtTinySerial.h"
#include "commands_consts.h"


CAtTinySerial::CAtTinySerial(QObject *parent) :
  QObject(parent),
  m_serial_port(nullptr) {
}

CAtTinySerial::~CAtTinySerial() {
  if (m_serial_port) delete m_serial_port;
}
//////////////////////////////////////////////////////////////////////////

bool CAtTinySerial::set_serial_port(const QSerialPortInfo& port_info,
                                    QString& err)
{
  static const uint8_t cmd_init[1] = {BCMD_CHECK_DEV};

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

  if ((uint8_t)answer.at(0) != BCMD_CHECK_DEV_ACK) {
    err = QString(
            "Received something different (not INIT_ACK) from controller. %1")
          .arg((uint8_t)answer.at(0));
    return false;
  }

  connect(m_serial_port, &QSerialPort::readyRead, this, &CAtTinySerial::serial_port_ready_read);
  err = "Succesfully initialized";
  return true;
}
//////////////////////////////////////////////////////////////////////////

void CAtTinySerial::dev_send_cmd(uint8_t *cmd, size_t len) {
  if (!m_serial_port) return ;
  qint64 written = m_serial_port->write((char*)cmd, len);
  bool flushed = m_serial_port->flush();
  if (written != 1 || !flushed) {
    emit error_happened(m_serial_port->errorString());
  }
}
//////////////////////////////////////////////////////////////////////////

void CAtTinySerial::serial_port_ready_read() {
  QByteArray arr = m_serial_port->readAll();
  if (arr.isEmpty()) return;
  emit on_command_received(arr);
}
//////////////////////////////////////////////////////////////////////////

void CAtTinySerial::dev_btn_start_enable() {
  static uint8_t cmd[1] = {BCMD_BTN_START_ENABLE};
  dev_send_cmd(cmd, 1);
}
//////////////////////////////////////////////////////////////////////////

void CAtTinySerial::dev_init_state()
{
  static uint8_t restart_cmd[1] = {BCMD_INIT_STATE};
  dev_send_cmd(restart_cmd, 1);
}
//////////////////////////////////////////////////////////////////////////

void CAtTinySerial::dev_start_countdown() {
  static uint8_t start_countdown[1] = {BCMD_START_COUNTDOWN};
  dev_send_cmd(start_countdown, 1);
}
//////////////////////////////////////////////////////////////////////////

