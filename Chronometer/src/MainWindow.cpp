#include "ui_MainWindow.h"
#include "MainWindow.h"
#include "ChronometerController.h"
#include "commands_consts.h"

#include <QTime>
#include <QTimer>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QByteArray>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QMediaPlayer>


MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_chronometer_controller(nullptr),
  m_refresh_timer(nullptr),
  m_model_ports(nullptr)
  {

  ui->setupUi(this);
  m_chronometer_controller = new CChronometerController;
  m_refresh_timer = new QTimer(this);
  m_refresh_timer->setInterval(10);
  ui->lbl_error->setVisible(true);

  m_model_ports = new QStandardItemModel;
  QList<QSerialPortInfo> lst_ports = QSerialPortInfo::availablePorts();
  for (auto i : lst_ports) {
    QStandardItem* item = new QStandardItem(i.portName());
    m_model_ports->appendRow(item);
  }
  ui->cb_serial_ports->setModel(m_model_ports);

  connect(m_refresh_timer, &QTimer::timeout,
          this, &MainWindow::refresh_timer_timeout);
  connect(ui->btn_start_stop, &QPushButton::released,
          this, &MainWindow::btn_start_stop_released);
  connect(ui->cb_serial_ports, (void(QComboBox::*)(int)) &QComboBox::currentIndexChanged,
          this, &MainWindow::cb_serial_ports_index_changed);
  connect(m_chronometer_controller, &CChronometerController::state_changed,
          this, &MainWindow::chronometer_controller_state_changed);
  connect(ui->btn_fall1, &QPushButton::released,
          this, &MainWindow::btn_fall0_released);
  connect(ui->btn_fall2, &QPushButton::released,
          this, &MainWindow::btn_fall1_released);

  if (QSerialPortInfo::availablePorts().size() > 0)
    cb_serial_ports_index_changed(0);
}

MainWindow::~MainWindow() {
  if (m_chronometer_controller) delete m_chronometer_controller;
  if (m_model_ports) delete m_model_ports;
  delete ui;
}
//////////////////////////////////////////////////////////////

static const QString beep1 = "beep1.wav";
static const QString beep2 = "beep2.wav";

void
MainWindow::play_start_sound() {

  QString dp = QApplication::applicationDirPath();
  QDir dir(dp + QDir::separator() + "resources");
  ui->btn_start_stop->setEnabled(false);

  if (!dir.exists()) {
    ui->lbl_error->setText("resources directory doesn't exist");
    return;
  }

  QString files[2] = {beep1, beep2};
  for (int i = 0; i < 2; ++i) {
    QFile f(dir.path() + QDir::separator() + files[i]);
    if (!f.exists()) {
      ui->lbl_error->setText(QString("%1 doesn't exist").arg(files[i]));
      return;
    }
  }

  CStartSoundPlayer* player = new CStartSoundPlayer;
  QThread* th = new QThread;

  connect(th, &QThread::started, player, &CStartSoundPlayer::play);
  connect(player, &CStartSoundPlayer::finished, th, &QThread::quit);

  connect(player, &CStartSoundPlayer::start_signal, [this](){
    QString err;
    if (!m_chronometer_controller->start(err)) {
      ui->lbl_error->setVisible(true);
      ui->lbl_error->setText(err);
    }
    ui->btn_start_stop->setEnabled(true);
  });

  connect(th, &QThread::finished, player, &CStartSoundPlayer::deleteLater);
  connect(th, &QThread::finished, th, &QThread::deleteLater);

  player->moveToThread(th);
  th->start();
}
//////////////////////////////////////////////////////////////

void
MainWindow::btn_start_stop_released() {

  if (!m_chronometer_controller->is_running()) {
    play_start_sound();
  } else {
    m_chronometer_controller->stop_all();    
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::refresh_timer_timeout() {
  static const QString tf = "ss.zzz";  
  int32_t ms[2] = {m_chronometer_controller->time0_ms(),
                   m_chronometer_controller->time1_ms()};
  QLineEdit* le[2] = {ui->le_time1, ui->le_time2 };

  for (int i = 0; i < 2; ++i) {
    QTime t = QTime::fromMSecsSinceStartOfDay(ms[i]);
    QString ts = t.toString(tf);
    le[i]->setText(ms[i] == CChronometerController::FALL_TIME ? "FALL" : ts);
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::cb_serial_ports_index_changed(int ix) {
  QString err;
  ui->lbl_error->setVisible(false);
  QList<QSerialPortInfo> lst_ports = QSerialPortInfo::availablePorts();
  if (lst_ports.size() > ix) {
    m_chronometer_controller->set_serial_port(lst_ports.at(ix), err);
    ui->lbl_error->setVisible(true);
    ui->lbl_error->setText(err);
  } else {
    m_model_ports->clear();
    QList<QSerialPortInfo> lst_ports = QSerialPortInfo::availablePorts();
    for (auto i : lst_ports) {
      QStandardItem* item = new QStandardItem(i.portName());
      m_model_ports->appendRow(item);
    }
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::chronometer_controller_state_changed(bool running) {
  if (running) {
    m_refresh_timer->start();
    ui->btn_start_stop->setText("Стоп");
    ui->cb_serial_ports->setEnabled(false);
    ui->lbl_error->setEnabled(false);
  } else {
    m_refresh_timer->stop();
    ui->btn_start_stop->setText("Старт");
    ui->cb_serial_ports->setEnabled(true);
    ui->lbl_error->setEnabled(true);
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::btn_fall0_released() {
  m_chronometer_controller->fall0();
  refresh_timer_timeout();
}
//////////////////////////////////////////////////////////////

void
MainWindow::btn_fall1_released() {
  m_chronometer_controller->fall1();
  refresh_timer_timeout();
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
