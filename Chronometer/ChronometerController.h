#ifndef CHRONOMETERCONTROLLER_H
#define CHRONOMETERCONTROLLER_H

#include <QObject>
#include <chrono>

class QTimer;
class QSerialPort;
class QSerialPortInfo;

class CChronometerController : public QObject {
  Q_OBJECT
private:  
  bool      m_is_running;
  int32_t  m_current_ms;
  QTimer *  m_timer;

  int32_t  m_time0_ms;
  int32_t  m_time1_ms;
  bool m_time0_stopped;
  bool m_time1_stopped;

  std::chrono::time_point<std::chrono::high_resolution_clock> m_time_start;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_time_stop;
  QSerialPort* m_serial_port;

  void stop_time0();
  void stop_time1();
  void handle_rx(uint8_t rx);

public:
  static const int FALL_TIME = 0;

  CChronometerController(QObject* parent = nullptr);
  virtual ~CChronometerController();

  bool start();
  void stop_all();

  void fall0();
  void fall1();

  int32_t time0_ms() const {return m_time0_ms;}
  int32_t time1_ms() const {return m_time1_ms;}
  int32_t current_ms() const {return m_current_ms;}

  bool is_running() const {return m_is_running;}
  bool set_serial_port(const QSerialPortInfo& port_info, QString &err);

private slots:
  void ms_timer_timeout();
  void serial_port_ready_read();

signals:
  void state_changed(bool running);
};

#endif // CHRONOMETERCONTROLLER_H
