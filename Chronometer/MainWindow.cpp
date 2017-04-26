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
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_chronometer_controller(nullptr),
  m_refresh_timer(nullptr),
  m_serial_port(nullptr),
  m_model_ports(nullptr) {

  ui->setupUi(this);
  m_chronometer_controller = new CChronometerController;
  m_refresh_timer = new QTimer(this);
  m_refresh_timer->setInterval(10);

  m_model_ports = new QStandardItemModel;
  QList<QSerialPortInfo> lst_ports = QSerialPortInfo::availablePorts();
  for (auto i : lst_ports) {
    QStandardItem* item = new QStandardItem(i.portName());
    m_model_ports->appendRow(item);
  }
  ui->cb_serial_ports->setModel(m_model_ports);

  connect(m_refresh_timer, SIGNAL(timeout()),
          this, SLOT(refresh_timer_timeout()));
  connect(ui->btn_start_stop, SIGNAL(released()),
          this, SLOT(btn_start_stop_released()));
  connect(ui->cb_serial_ports, SIGNAL(currentIndexChanged(int)),
          this, SLOT(cb_serial_ports_index_changed(int)));

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
  if (!m_chronometer_controller->is_running()) {
    char restart_cmd[1];
    restart_cmd[0] = BCMD_RESTART;
    m_serial_port->write(restart_cmd, 1);
    m_serial_port->flush();

    m_chronometer_controller->start();
    m_refresh_timer->start();
    ui->btn_start_stop->setText("Stop");
  } else {
    m_chronometer_controller->stop_all();
    m_refresh_timer->stop();
    ui->btn_start_stop->setText("Start");
  }
}
//////////////////////////////////////////////////////////////

void
MainWindow::refresh_timer_timeout() {
  static const QString tf = "ss.zzz";  
  uint32_t ms0 = m_chronometer_controller->time0_ms();
  uint32_t ms1 = m_chronometer_controller->time1_ms();
  QTime t0 = QTime::fromMSecsSinceStartOfDay(ms0);
  QTime t1 = QTime::fromMSecsSinceStartOfDay(ms1);
  QString ts0, ts1;
  ts0 = t0.toString(tf);
  ts1 = t1.toString(tf);
  ui->le_time1->setText(ts0);
  ui->le_time2->setText(ts1);
}
//////////////////////////////////////////////////////////////

void
MainWindow::cb_serial_ports_index_changed(int ix) {
  if (m_serial_port) {
    delete m_serial_port;
    m_serial_port = nullptr;
  }

//  ui->m_lbl_error->setVisible(false);
  QList<QSerialPortInfo> lst_ports = QSerialPortInfo::availablePorts();
  if (lst_ports.size() > ix) {
    m_serial_port = new QSerialPort(lst_ports.at(ix));
    m_serial_port->setBaudRate(9600);
    m_serial_port->setParity(QSerialPort::NoParity);
    m_serial_port->setDataBits(QSerialPort::Data8);
    m_serial_port->setStopBits(QSerialPort::OneStop);
    m_serial_port->setFlowControl(QSerialPort::NoFlowControl);
    if (!m_serial_port->open(QSerialPort::ReadWrite)) {
//      ui->m_lbl_error->setVisible(true);
//      ui->m_lbl_error->setText(m_serial_port->errorString());
      qDebug() << m_serial_port->errorString();
      return;
    }

    connect(m_serial_port, SIGNAL(readyRead()),
            this, SLOT(serial_port_ready_read()));
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
MainWindow::serial_port_ready_read() {
  QByteArray arr = m_serial_port->readAll();
  if (arr.isEmpty()) return;
  m_chronometer_controller->handle_rx((uint8_t)arr.at(0));
}
//////////////////////////////////////////////////////////////
