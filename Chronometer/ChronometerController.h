#ifndef CHRONOMETERCONTROLLER_H
#define CHRONOMETERCONTROLLER_H

#include "Commons.h"
#include <QObject>
#include <chrono>

class QTimer;

class CChronometerController : public QObject {
  Q_OBJECT
private:  
  bool      m_is_running;
  uint32_t  m_current_ms;
  QTimer *  m_timer;

  uint32_t  m_time0_ms;
  uint32_t  m_time1_ms;
  bool m_time0_stopped;
  bool m_time1_stopped;

  std::chrono::time_point<std::chrono::high_resolution_clock> m_time_start;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_time_stop;
  std::chrono::high_resolution_clock m_hr_clock;

public:
  CChronometerController(QObject* parent = nullptr);
  virtual ~CChronometerController();

  void start();
  void stop_all();
  void stop_time0();
  void stop_time1();

  uint32_t time0_ms() const {return m_time0_ms;}
  uint32_t time1_ms() const {return m_time1_ms;}
  uint32_t current_ms() const {return m_current_ms;}

  bool is_running() const {return m_is_running;}
  void handle_rx(uint8_t rx);

private slots:
  void ms_timer_timeout();
};

#endif // CHRONOMETERCONTROLLER_H
