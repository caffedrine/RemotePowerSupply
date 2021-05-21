#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QTime>

#include "SerialClient.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void consoleLog(QString str);

private slots:
    void serialDataReceivingSlot();
    void setSerialPortStatus(bool);

    void on_pushButton_updateList_clicked();
    void on_pushButton_connectSerialPort_clicked();
    void on_pushButton_disconnectSerialPort_clicked();
    qint64 SendSerialData(QByteArray bytes);
    void on_pushButton_Send_clicked();
    void on_pushButton_Clear_clicked();
    void on_comboBox_BaudRates_currentIndexChanged(int index);

    void on_pushButton_ClearConsole_clicked();

    void on_PacketReceived(QString power_supply_packet);
    void on_radioButton_powerON_clicked();
    void on_radioButton_powerOFF_clicked();
    void on_pushButton_refreshPowerState_clicked();

private:
    Ui::MainWindow *ui;

    /* Serial port handler */
    class SerialClient *serialPort = Q_NULLPTR;
    SerialClient::BaudRate baud = QSerialPort::Baud115200;
};

#endif // MAINWINDOW_H
