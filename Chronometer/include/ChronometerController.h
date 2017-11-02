#ifndef CHRONOMETERCONTROLLER_H
#define CHRONOMETERCONTROLLER_H

#include <QObject>
#include <chrono>

class QTimer;
class QSerialPort;
class QSerialPortInfo;
class QMediaPlayer;

enum state_t {
  CC_RUNNING = 1,
  CC_STOPPED = 2,
  CC_PLAYING_SOUND = 3
};


class CStartSoundPlayer : public QObject {
  Q_OBJECT
private:
  QTimer* m_timer;
  QMediaPlayer* m_start_player;
  int8_t m_signals_count;
public:
  CStartSoundPlayer(QObject* parent = nullptr);
  virtual ~CStartSoundPlayer();

  void abort();

private slots:
  void timer_timeout();
public slots:
  void play();

signals:
  void start_signal();
  void finished();
};
//////////////////////////////////////////////////////////////

class CChronometerController : public QObject {
  Q_OBJECT
private:  
  state_t   m_state;
  int32_t   m_current_ms;
  QTimer *  m_timer;

  int32_t  m_time0_ms;
  int32_t  m_time1_ms;
  bool m_time0_stopped;
  bool m_time1_stopped;

  std::chrono::time_point<std::chrono::high_resolution_clock> m_time_start;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_time_stop;
  QSerialPort* m_serial_port;

  void change_state(state_t new_state);

  void stop_time0();
  void stop_time1();
  void handle_rx(uint8_t rx);  

  void play_start_sound();

  void dev_btn_start_enable();

  void dev_init_state();
  void dev_start_countdown();
public:
  static const int FALL_TIME = 0;

  CChronometerController(QObject* parent = nullptr);
  virtual ~CChronometerController();

  void start();
  void stop_all();

  void fall0();
  void fall1();

  void platform0_pressed();
  void platform1_pressed();

  int32_t time0_ms() const {return m_time0_ms;}
  int32_t time1_ms() const {return m_time1_ms;}
  int32_t current_ms() const {return m_current_ms;}

  bool is_started() const {return m_state == CC_RUNNING || m_state == CC_PLAYING_SOUND;}
  bool set_serial_port(const QSerialPortInfo& port_info, QString &err);

private slots:
  void start_timer();
  void ms_timer_timeout();
  void serial_port_ready_read();

signals:
  void state_changed(int state);
  void error_happened(QString err);
};

#endif // CHRONOMETERCONTROLLER_H
