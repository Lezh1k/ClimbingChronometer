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

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_chronometer_controller(nullptr),
  m_refresh_timer(nullptr),
  m_model_ports(nullptr) {

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
  delete ui;
}
//////////////////////////////////////////////////////////////

void
MainWindow::btn_start_stop_released() {
  QString start_str;
  if (!m_chronometer_controller->is_running()) {
    if (!m_chronometer_controller->start(start_str)) {
      ui->lbl_error->setVisible(true);
      ui->lbl_error->setText(start_str);
    }
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