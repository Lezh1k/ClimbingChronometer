#include <QApplication>
#include <QDir>
#include <QThread>
#include <QTimer>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QDebug>

#include "ChronometerController.h"
#include "commands_consts.h"

static const QString beep1 = "beep1.wav";
static const QString beep2 = "beep2.wav";

static bool s_resources_exist = false;
static QMediaPlaylist *s_plist = nullptr;

static QString& init_plist() {
  static QString res = "";
  if (s_plist) return res;
  s_plist = new QMediaPlaylist;
  do {
    QString dp = QApplication::applicationDirPath();
    QDir dir(dp + QDir::separator() + "resources");
    if (!dir.exists()) {
      res = "resources directory doesn't exist";
      break;
    }

    s_plist = new QMediaPlaylist;
    s_plist->setPlaybackMode(QMediaPlaylist::CurrentItemOnce);

    QString files[2] = {beep1, beep2};
    for (int i = 0; i < 2; ++i) {
      QFile f(dir.path() + QDir::separator() + files[i]);
      if (!f.exists()) {
        res = QString("%1 doesn't exist").arg(files[i]);
        break;
      }
    }
    s_plist->addMedia(QUrl::fromLocalFile(dir.path() + QDir::separator() + beep1));
    s_plist->addMedia(QUrl::fromLocalFile(dir.path() + QDir::separator() + beep2));

    s_resources_exist = true;
  } while (0);

  return res;
}
//////////////////////////////////////////////////////////////////////////

CChronometerController::CChronometerController(QObject* parent)
    : QObject(parent),
      m_state(CC_STOPPED),
      m_current_ns(0),
      m_timer(nullptr),
      m_time0_ns(0),
      m_time1_ns(0),
      m_time0_stopped(true),
      m_time1_stopped(true) {
  m_timer = new QTimer(this);
  m_timer->setInterval(10);
  connect(m_timer, &QTimer::timeout, this,
          &CChronometerController::ms_timer_timeout);
  connect(this, &CChronometerController::timer_stoped,
          m_timer, &QTimer::stop);

  QString init_res = init_plist();
  if (init_res != "") {
    emit error_happened(init_res);
  }
}
//////////////////////////////////////////////////////////////////////////

CChronometerController::~CChronometerController() {  
}
//////////////////////////////////////////////////////////////////////////

void
CChronometerController::attiny_serial_cmd_received(QByteArray arr) {
  handle_rx(arr.at(0)); //;)
}
//////////////////////////////////////////////////////////////

void CChronometerController::start() {

  emit dev_start_countdown();
  play_start_sound();
}
//////////////////////////////////////////////////////////////

void CChronometerController::stop_all() {
  change_state(CC_STOPPED);
  emit timer_stoped();
  m_time0_stopped = m_time1_stopped = true;
  m_time_stop = controller_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ns = diff.count() / 1000000;
  emit dev_btn_start_enable();
}
//////////////////////////////////////////////////////////////

void CChronometerController::fall0() {
  if (m_time0_stopped) return;
  m_time0_ns = FALL_TIME;
  m_time0_stopped = true;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void CChronometerController::fall1() {
  if (m_time1_stopped) return;
  m_time1_ns = FALL_TIME;
  m_time1_stopped = true;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void CChronometerController::change_state(state_t new_state) {
  emit state_changed((int)(m_state = new_state));
}
//////////////////////////////////////////////////////////////

void CChronometerController::stop_time0() {
  if (m_time0_stopped) return;
  m_time0_stopped = true;
  m_time0_ns = (m_time_stop - m_time_start).count() / 1000000;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void CChronometerController::stop_time1() {
  if (m_time1_stopped) return;
  m_time1_stopped = true;
  m_time1_ns = (m_time_stop - m_time_start).count() / 1000000;
  if (m_time0_stopped && m_time1_stopped) stop_all();
}
//////////////////////////////////////////////////////////////

void CChronometerController::handle_rx(uint8_t rx) {
  uint8_t plt01_state = (rx >> 3) & 0x07;
  uint8_t btn01_state = rx & 0x07;

  qDebug() << QString("%1").arg(rx, 8, 2);

  // handle start button
  if (rx == BC_BTN_START) {
    if (m_state != CC_RUNNING) start();
    return;
  }

  // handle stop buttons state
  switch (btn01_state) {
    case (BC_BTN0 << 1):
      stop_time0();
      break;
    case (BC_BTN0 << 1 | BC_BTN1):
      stop_time0();
      stop_time1();
      break;
    case (BC_BTN1 << 1):
      stop_time1();
      break;
    case (BC_BTN1 << 1 | BC_BTN0):
      stop_time1();
      stop_time0();
      break;
    default:
      // todo notify somehow
      break;
  }  // switch btn01_state

  // handle start platforms state
  switch (plt01_state) {
    case (BC_PLATFORM0 << 1):
      platform0_pressed();
      break;
    case (BC_PLATFORM0 << 1 | BC_PLATFORM1):
      platform0_pressed();
      platform1_pressed();
      break;
    case (BC_PLATFORM1 << 1):
      platform1_pressed();
      break;
    case (BC_PLATFORM1 << 1 | BC_PLATFORM0):
      platform1_pressed();
      platform0_pressed();
      break;
    default:
      // todo notify somehow
      break;
  }
}
//////////////////////////////////////////////////////////////

void CChronometerController::start_timer() {
  dev_init_state();
  m_time0_ns = m_time1_ns = m_current_ns = 0;
  m_time0_stopped = m_time1_stopped = false;
  m_time_start = controller_clock::now();
  m_timer->start();
  change_state(CC_RUNNING);
}
//////////////////////////////////////////////////////////////

void CChronometerController::ms_timer_timeout() {
  m_time_stop = controller_clock::now();
  std::chrono::nanoseconds diff = m_time_stop - m_time_start;
  m_current_ns = diff.count() / 1000000;
  if (!m_time0_stopped) m_time0_ns = m_current_ns;
  if (!m_time1_stopped) m_time1_ns = m_current_ns;
}
//////////////////////////////////////////////////////////////

void CChronometerController::play_start_sound() {  
  if (!s_resources_exist) {
    emit error_happened("Check resources and restart application");
    change_state(CC_STOPPED);
    return;
  }

  CStartSoundPlayer* player = new CStartSoundPlayer(nullptr, s_plist);
  QThread* th = new QThread;

  connect(th, &QThread::started, player, &CStartSoundPlayer::play);
  connect(player, &CStartSoundPlayer::start_signal, this,
          &CChronometerController::start_timer);

  connect(player, &CStartSoundPlayer::finished, th, &QThread::quit);
  connect(th, &QThread::finished, th, &QThread::deleteLater);

  player->moveToThread(th);
  th->start();
  change_state(CC_PLAYING_SOUND);
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

CStartSoundPlayer::CStartSoundPlayer(QObject *parent,
                                     QMediaPlaylist *plist) :
  QObject(parent),
  m_timer(nullptr),
  m_player(nullptr),
  m_signals_count(3) {
  m_timer = new QTimer(this);
  m_timer->setInterval(1000);
  m_player = new QMediaPlayer(this);
  m_player->setPlaylist(plist);
  connect(m_timer, &QTimer::timeout, this, &CStartSoundPlayer::timer_timeout);
}

CStartSoundPlayer::~CStartSoundPlayer() {
  if (m_timer) delete m_timer;
  if (m_player) delete m_player;
}
//////////////////////////////////////////////////////////////

void CStartSoundPlayer::abort() {
  m_timer->stop();
  m_player->stop();
  emit finished();
}
//////////////////////////////////////////////////////////////

void CStartSoundPlayer::timer_timeout() {  
  --m_signals_count;
  m_timer->stop();
  if (m_signals_count > 0) {
    m_timer->setInterval(1000);
    m_player->playlist()->setCurrentIndex(0);
    m_player->play();
    m_timer->start();
  } else {
    emit start_signal();
    m_player->playlist()->setCurrentIndex(1);
    connect(m_player, &QMediaPlayer::stateChanged,
            [this](QMediaPlayer::State newState) {
              if (newState == QMediaPlayer::StoppedState) {
                emit finished();
              }
            });
    m_player->play();
  }
}
//////////////////////////////////////////////////////////////

void CStartSoundPlayer::play() { m_timer->start(10); }
//////////////////////////////////////////////////////////////
