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
class QMediaPlayer;
class QLineEdit;
class QLabel;
class CAtTinySerial;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  virtual ~MainWindow();

signals:
  void started_new_round();
  void cc_fall0();
  void cc_fall1();
  void cc_stop();

private:
  Ui::MainWindow *ui;
  CChronometerController *m_chronometer_controller;
  CAtTinySerial *m_attiny_serial;
  QTimer *m_refresh_timer;
  QStandardItemModel *m_model_ports;

  void adjust_font_size_for_same_components(QLineEdit **le, size_t count);
  void adjust_font_for_time_lines();
  void init_chronometer();

private slots:
  void btn_refresh_com_released();
  void btn_start_stop_released();
  void refresh_timer_timeout();
  void cb_serial_ports_index_changed(int ix);

  void btn_fall0_released();
  void btn_fall1_released();  

  void chronometer_controller_state_changed(int state);
  void chronometer_controller_error_happened(QString err);

  // QWidget interface
protected:
  virtual void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H
