#include "ChronometerController.h"

#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDir>
#include <QApplication>
#include <QThread>
#include <QMediaPlayer>

#include "commands_consts.h"


CChronometerController::CChronometerController(QObject *parent) :
  QObject(parent),
  m_state(CC_STOPPED),
  m_current_ms(0),
  m_timer(nullptr),
  m_time0_ms(0),
  m_time1_ms(0),
  m_time0_stopped(true),
  m_time1_stopped(true),
  m_serial_port(nullptr) {

  m_timer = new QTimer(this);
  m_timer->setInterval(10);
  connect(m_timer, &QTimer::timeout, this, &CChronometerController::ms_timer_timeout);
}

CChronometerController::~CChronometerController() {
  if (m_serial_port) delete m_serial_port;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::start() {  
  static uint8_t start_countdown[1] = {BCMD_START_COUNTDOWN};

  qint64 written = m_serial_port->write((char*)start_countdown, 1);
  bool flushed = m_serial_port->flush();

  if (written != 1 || !flushed) {
    emit error_happened(m_serial_port->errorString());
    change_state(CC_STOPPED);
    return;
  }

  play_start_sound();
}
//////////////////////////////////////////////////////////////

void
CChronometerController::stop_all() {
  change_state(CC_STOPPED);
  m_time0_stopped = m_time1_stopped = true;
  m_timer->stop();
  m_time_stop = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ms = diff.count() / 1000000;

  static uint8_t cmd[1] = {BCMD_BTN_START_ENABLE};
  qint64 written = m_serial_port->write((char*)cmd, 1);
  bool flushed = m_serial_port->flush();

  if (written != 1 || !flushed) {
    emit error_happened(m_serial_port->errorString());
  }
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
CChronometerController::platform0_pressed() {

}
//////////////////////////////////////////////////////////////

void
CChronometerController::platform1_pressed() {

}
//////////////////////////////////////////////////////////////

void
CChronometerController::change_state(state_t new_state) {
  emit state_changed((int)(m_state = new_state));
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
  uint8_t plt01_state = (rx >> 3) & 0x07;
  uint8_t btn01_state = rx & 0x07;

  qDebug() << QString("%1").arg(rx, 8, 2);

  //handle start button
  if (rx == BC_BTN_START) {
    if (m_state != CC_RUNNING) start();
    return;
  }

  //handle stop buttons state
  switch (btn01_state) {
    case ( BC_BTN0 << 1 ) :
      stop_time0();
      break;
    case ( BC_BTN0 << 1 | BC_BTN1 ) :
      stop_time0();
      stop_time1();
      break;
    case ( BC_BTN1 << 1 ) :
      stop_time1();
      break;
    case ( BC_BTN1 << 1 | BC_BTN0 ) :
      stop_time1();
      stop_time0();
      break;
    default:
      //todo notify somehow
      break;
  } //switch btn01_state

  //handle start platforms state
  switch (plt01_state) {
    case ( BC_PLATFORM0 << 1 ) :
      platform0_pressed();
      break;
    case ( BC_PLATFORM0 << 1 | BC_PLATFORM1 ) :
      platform0_pressed();
      platform1_pressed();
      break;
    case ( BC_PLATFORM1 << 1 ) :
      platform1_pressed();
      break;
    case ( BC_PLATFORM1 << 1 | BC_PLATFORM0 ) :
      platform1_pressed();
      platform0_pressed();
      break;
    default:
      //todo notify somehow
      break;
  }
}
//////////////////////////////////////////////////////////////

void
CChronometerController::start_timer() {
  static uint8_t restart_cmd[1] = {BCMD_INIT_STATE};
  qint64 written = m_serial_port->write((char*)restart_cmd, 1);
  bool flushed = m_serial_port->flush();

  if (written != 1 || !flushed) {
    emit error_happened(m_serial_port->errorString());
    change_state(CC_STOPPED);
    return;
  }

  m_time0_ms = m_time1_ms = m_current_ms = 0;
  m_time0_stopped = m_time1_stopped = false;
  m_time_start = std::chrono::high_resolution_clock::now();
  m_timer->start();  
  change_state(CC_RUNNING);
}
//////////////////////////////////////////////////////////////

bool
CChronometerController::set_serial_port(const QSerialPortInfo &port_info,
                                        QString& err) {
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
    err = QString("Received something different (not INIT_ACK) from controller. %1").
        arg((uint8_t)answer.at(0));
    return false;
  }

  connect(m_serial_port, &QSerialPort::readyRead,
          this, &CChronometerController::serial_port_ready_read);
  err = "Succesfully initialized";
  return true;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::ms_timer_timeout() {
  m_time_stop = std::chrono::high_resolution_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ms = diff.count() / 1000000;
  if (!m_time0_stopped)
    m_time0_ms = m_current_ms;
  if (!m_time1_stopped)
    m_time1_ms = m_current_ms;
}
//////////////////////////////////////////////////////////////

void
CChronometerController::serial_port_ready_read() {
  QByteArray arr = m_serial_port->readAll();
  if (arr.isEmpty()) return;
  handle_rx((uint8_t)arr.at(0));
}
//////////////////////////////////////////////////////////////

static const QString beep1 = "beep1.wav";
static const QString beep2 = "beep2.wav";

void
CChronometerController::play_start_sound() {
  QString dp = QApplication::applicationDirPath();
  QDir dir(dp + QDir::separator() + "resources");
  if (!dir.exists()) {
    emit error_happened("resources directory doesn't exist");
    change_state(CC_STOPPED);
    return;
  }

  QString files[2] = {beep1, beep2};
  for (int i = 0; i < 2; ++i) {
    QFile f(dir.path() + QDir::separator() + files[i]);
    if (!f.exists()) {
      emit error_happened(QString("%1 doesn't exist").arg(files[i]));
      change_state(CC_STOPPED);
      return;
    }
  }

  CStartSoundPlayer* player = new CStartSoundPlayer;
  QThread* th = new QThread;

  connect(th, &QThread::started, player, &CStartSoundPlayer::play);
  connect(player, &CStartSoundPlayer::finished, th, &QThread::quit);
  connect(player, &CStartSoundPlayer::start_signal, this, &CChronometerController::start_timer);

  connect(th, &QThread::finished, player, &CStartSoundPlayer::deleteLater);
  connect(th, &QThread::finished, th, &QThread::deleteLater);

  player->moveToThread(th);
  th->start();
  change_state(CC_PLAYING_SOUND);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

CStartSoundPlayer::CStartSoundPlayer(QObject *parent) :
  QObject(parent),
  m_timer(nullptr),
  m_start_player(nullptr) ,
  m_signals_count(3) {
  m_timer = new QTimer(this);
  m_timer->setInterval(1000);
  m_start_player = new QMediaPlayer;
  connect(m_timer, &QTimer::timeout, this, &CStartSoundPlayer::timer_timeout);
}

CStartSoundPlayer::~CStartSoundPlayer() {
  if (m_timer) delete m_timer;
  if (m_start_player) delete m_start_player;
}
//////////////////////////////////////////////////////////////

void CStartSoundPlayer::abort() {
  m_timer->stop();
  m_start_player->stop();
  emit finished();
}
//////////////////////////////////////////////////////////////

void
CStartSoundPlayer::timer_timeout() {
  QString dp = QApplication::applicationDirPath();
  QDir dir(dp + QDir::separator() + "resources");
  --m_signals_count;
  m_timer->stop();
  if (m_signals_count > 0) {
    m_timer->setInterval(1000);
    m_start_player->setMedia(QUrl::fromLocalFile(dir.path() + QDir::separator() + beep1));
    m_start_player->play();
    m_timer->start();
  } else {
    emit start_signal();
    m_start_player->setMedia(QUrl::fromLocalFile(dir.path() + QDir::separator() + beep2));

    connect(m_start_player, &QMediaPlayer::stateChanged,
            [this](QMediaPlayer::State newState) {
      if (newState == QMediaPlayer::StoppedState) {
        emit finished();
      }
    });
    m_start_player->play();
  }
}
//////////////////////////////////////////////////////////////

void
CStartSoundPlayer::play() {
  m_timer->start(0);
}
//////////////////////////////////////////////////////////////
