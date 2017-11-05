#ifndef CHRONOMETERCONTROLLER_H
#define CHRONOMETERCONTROLLER_H

#include <QObject>
#include <chrono>

class QTimer;
class QSerialPort;
class QSerialPortInfo;

class QMediaPlayer;
class QMediaPlaylist;

class CAtTinySerial;

enum state_t {
  CC_RUNNING = 1,
  CC_STOPPED = 2,
  CC_PLAYING_SOUND = 3
};


class CStartSoundPlayer : public QObject {
  Q_OBJECT
private:
  QTimer* m_timer;
  QMediaPlayer *m_player;
  int8_t m_signals_count;
public:
  CStartSoundPlayer(QObject *parent, QMediaPlaylist *plist);
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

typedef std::chrono::system_clock controller_clock;
class CChronometerController : public QObject {
  Q_OBJECT
private:  
  state_t m_state;

  uint64_t m_current_ns;
  uint64_t m_time0_ns;
  uint64_t m_time1_ns;
  bool m_time0_stopped;
  bool m_time1_stopped;
  CAtTinySerial *m_attiny_serial;

  void change_state(state_t new_state);
  void stop_time0();
  void stop_time1();
  void handle_rx(uint8_t rx);
  void play_start_sound();  

  void platform0_pressed(){/*todo implement*/}
  void platform1_pressed(){/*todo implement*/}

  void dev_btn_start_enable();
  void dev_init_state();
  void dev_start_countdown();

public:
  static const int FALL_TIME = 0;
  explicit CChronometerController(CAtTinySerial *attiny_serial,
                                  QObject* parent = nullptr);
  virtual ~CChronometerController();

  int32_t time0_ns() const {return m_time0_ns;}
  int32_t time1_ns() const {return m_time1_ns;}
  bool is_started() const {return m_state == CC_RUNNING || m_state == CC_PLAYING_SOUND;}  

  void start();
  void stop_all();
  void fall0();
  void fall1();

private slots:
  void attiny_serial_cmd_received(QByteArray arr);
  void start_chronometer();

signals:
  void state_changed(int state);
  void error_happened(QString err);  
};

#endif // CHRONOMETERCONTROLLER_H
