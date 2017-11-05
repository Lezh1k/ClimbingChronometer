#ifndef ATTINYSERIAL_H
#define ATTINYSERIAL_H

#include <stdint.h>
#include <QObject>

class QSerialPort;
class QSerialPortInfo;

class CAtTinySerial : public QObject {
  Q_OBJECT
private:
  QSerialPort *m_serial_port;
  void dev_send_cmd(uint8_t *cmd, size_t len);

private slots:
  void serial_port_ready_read();

signals:
  void on_command_received(QByteArray);
  void error_happened(QString);

public:
  explicit CAtTinySerial(QObject *parent = nullptr);
  virtual ~CAtTinySerial();
  bool set_serial_port(const QSerialPortInfo &port_info, QString &err);

  void dev_btn_start_enable();
  void dev_init_state();
  void dev_start_countdown();
};

#endif // ATTINYSERIAL_H
