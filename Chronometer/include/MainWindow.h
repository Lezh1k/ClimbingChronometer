#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class CChronometerController;
class QTimer;
class QSerialPort;
class QStandardItemModel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  CChronometerController* m_chronometer_controller;
  QTimer* m_refresh_timer;  
  QStandardItemModel* m_model_ports;

private slots:
  void btn_start_stop_released();
  void refresh_timer_timeout();
  void cb_serial_ports_index_changed(int ix);
  void chronometer_controller_state_changed(bool running);

  void btn_fall0_released();
  void btn_fall1_released();

};

#endif // MAINWINDOW_H
