#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QTime>

#include "SerialComM.h"
#include "PowerSupplyM.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_updateList_clicked();
    void on_pushButton_connectSerialPort_clicked();
    void on_pushButton_disconnectSerialPort_clicked();
    void on_comboBox_BaudRates_currentIndexChanged(int index);

    void on_radioButton_powerON_clicked();
    void on_radioButton_powerOFF_clicked();
    void on_pushButton_refreshPowerState_clicked();

private:
    Ui::MainWindow *ui;

    /* Serial port handler */
    class SerialComM *serialPort = Q_NULLPTR;
    SerialComM::BaudRate baud = QSerialPort::Baud115200;

    /* Power supply handler */
    class PowerSupplyM *powerSupply = Q_NULLPTR;

    void showSerialPortStatus(bool connected);
    void showPowerSupplyState(power_supply_state_t state);
    void showTcpServerState(bool started);
};

#endif // MAINWINDOW_H
