#include "ui_MainWindow.h"
#include "MainWindow.h"
#include "ChronometerController.h"
#include "commands_consts.h"
#include "AtTinySerial.h"

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
#include <assert.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_chronometer_controller(nullptr),
  m_attiny_serial(nullptr),
  m_refresh_timer(nullptr),
  m_model_ports(nullptr) {

  ui->setupUi(this);

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
  connect(ui->btn_fall1, &QPushButton::released,
          this, &MainWindow::btn_fall0_released);
  connect(ui->btn_fall2, &QPushButton::released,
          this, &MainWindow::btn_fall1_released);
  connect(ui->btn_refresh_com, &QPushButton::released,
          this, &MainWindow::btn_refresh_com_released);

  QTimer *ti = new QTimer;
  ti->setInterval(100);
  connect(ti, &QTimer::timeout, [this]() {
    adjust_font_for_time_lines();
  });
  connect(ti, &QTimer::timeout, ti, &QTimer::deleteLater);
  ti->setSingleShot(true);
  ti->start();
  m_attiny_serial = new CAtTinySerial;
  init_chronometer();
}

MainWindow::~MainWindow() {
  if (m_chronometer_controller) delete m_chronometer_controller;
  if (m_attiny_serial) delete m_attiny_serial;
  if (m_model_ports) delete m_model_ports;
  delete ui;
}
//////////////////////////////////////////////////////////////////////////

void MainWindow::init_chronometer() {
  if (m_chronometer_controller) delete m_chronometer_controller;
  m_chronometer_controller = new CChronometerController;

  connect(m_chronometer_controller, &CChronometerController::state_changed,
          this, &MainWindow::chronometer_controller_state_changed);
  connect(m_chronometer_controller, &CChronometerController::error_happened,
          this, &MainWindow::chronometer_controller_error_happened);
  connect(m_attiny_serial, &CAtTinySerial::on_command_received,
          m_chronometer_controller, &CChronometerController::attiny_serial_cmd_received);
  connect(m_chronometer_controller, &CChronometerController::dev_btn_start_enable,
          m_attiny_serial, &CAtTinySerial::dev_btn_start_enable);
  connect(m_chronometer_controller, &CChronometerController::dev_init_state,
          m_attiny_serial, &CAtTinySerial::dev_init_state);
  connect(m_chronometer_controller, &CChronometerController::dev_start_countdown,
          m_attiny_serial, &CAtTinySerial::dev_start_countdown);

  connect(this, &MainWindow::cc_stop,
          m_chronometer_controller, &CChronometerController::stop_all);
  connect(this, &MainWindow::cc_fall0,
          m_chronometer_controller, &CChronometerController::fall0);
  connect(this, &MainWindow::cc_fall1,
          m_chronometer_controller, &CChronometerController::fall1);

  if (QSerialPortInfo::availablePorts().size() > 0)
    cb_serial_ports_index_changed(0);
}
//////////////////////////////////////////////////////////////////////////

void
MainWindow::btn_refresh_com_released() {  
  m_model_ports->clear();
  QList<QSerialPortInfo> lst_ports = QSerialPortInfo::availablePorts();
  for (auto i : lst_ports) {
    QStandardItem* item = new QStandardItem(i.portName());
    m_model_ports->appendRow(item);
  }
  ui->cb_serial_ports->setModel(m_model_ports);
  if (QSerialPortInfo::availablePorts().size() > 0)
    cb_serial_ports_index_changed(0);
}
//////////////////////////////////////////////////////////////

void
MainWindow::btn_start_stop_released() {
  if (!m_chronometer_controller->is_started()) {
    ui->le_time1->setText("00.000");
    ui->le_time2->setText("00.000");
    emit started_new_round();

    init_chronometer();
    QThread *th = new QThread;
    connect(this, &MainWindow::started_new_round, th, &QThread::quit);
    connect(th, &QThread::started, m_chronometer_controller, &CChronometerController::start);
    connect(th, &QThread::finished, th, &QThread::deleteLater);
    m_chronometer_controller->moveToThread(th);
    th->start();
  } else {
    emit cc_stop();
    refresh_timer_timeout();
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::refresh_timer_timeout() {
  static const QString tf = "ss.zzz";  
  int32_t ms[2] = {m_chronometer_controller->time0_ns(),
                   m_chronometer_controller->time1_ns()};
  QLineEdit* le[2] = {ui->le_time1, ui->le_time2};

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
  if (lst_ports.size() > ix && ix >= 0) {
    m_attiny_serial->set_serial_port(lst_ports.at(ix), err);
    ui->lbl_error->setVisible(true);
    ui->lbl_error->setText(err);
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::chronometer_controller_state_changed(int state) {
  switch (state) {
    case CC_RUNNING : //running
      m_refresh_timer->start();
      ui->btn_start_stop->setEnabled(true);
      ui->btn_start_stop->setText("Стоп");
      ui->cb_serial_ports->setEnabled(false);
      ui->btn_refresh_com->setEnabled(false);
      ui->lbl_error->setEnabled(false);
      break;
    case CC_STOPPED : //stopped
      m_refresh_timer->stop();
      ui->btn_start_stop->setEnabled(true);
      ui->btn_start_stop->setText("Старт");
      ui->cb_serial_ports->setEnabled(true);
      ui->btn_refresh_com->setEnabled(true);
      ui->lbl_error->setEnabled(true);
      break;
    case CC_PLAYING_SOUND : //playing sound
      ui->btn_start_stop->setEnabled(false);
      ui->cb_serial_ports->setEnabled(false);
      ui->btn_refresh_com->setEnabled(false);
      ui->lbl_error->setEnabled(false);
      break;
    default:
      break;
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::chronometer_controller_error_happened(QString err) {
  ui->lbl_error->setVisible(true);
  ui->lbl_error->setText(err);
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
////////////////////////////////////////////////////////////////////////////

void
MainWindow::adjust_font_for_time_lines() {
  QLineEdit* les[] = {ui->le_time1, ui->le_time2};
  adjust_font_size_for_same_components(les, 2);
}
//////////////////////////////////////////////////////////////////////////


void
MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  adjust_font_for_time_lines();
}
/////////////////////////////////////////////////////////////////////////

void
MainWindow::adjust_font_size_for_same_components(QLineEdit** le,
                                                 size_t count) {
  assert(le);
  assert(count > 0);

  QString str = le[0]->text();
  QFont font = le[0]->font();
  QFontMetrics fm(font);

  int first, last, middle;
  last = 2048; first = 0; //we don't need such a big value. but I want to be sure that we will find font size.
  while (first < last) {
    middle = (first+last) >> 1;
    font.setPointSize(middle);
    fm = QFontMetrics(font);
    if (fm.height() > le[0]->height() ||
        fm.width(str) > le[0]->width()) {
      last = middle;
    } else {
      first = middle+1;
    }
  }
  font.setPointSize(last-2);
  for (size_t i = 0; i < count; ++i)
    le[i]->setFont(font);
}
