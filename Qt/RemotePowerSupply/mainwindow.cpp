#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->ui->comboBox_BaudRates->addItems({"1200", "2400", "4800", "9600", "14400", "19200", "38400", "57600", "115200"});
    this->ui->comboBox_BaudRates->setCurrentIndex(3);
    this->baud = QSerialPort::Baud9600;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showSerialPortStatus(bool connected)
{
    if( connected )
    {
        this->ui->connectionStatusLabel->setText( "CONNECTED" );
        this->ui->connectionStatusLabel->setStyleSheet("QLabel { color : green; }");
    }
    else
    {
        this->ui->connectionStatusLabel->setText("NOT CONNECTED");
        this->ui->connectionStatusLabel->setStyleSheet("QLabel { color : red; }");
    }
}

void MainWindow::showPowerSupplyState(power_supply_state_t state)
{
    if( state == POWER_ON )
    {
        this->ui->label_PowerSupplyStatus->setText("ON");
        ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : green; }");
    }
    else if( state == POWER_OFF )
    {
        ui->label_PowerSupplyStatus->setText("OFF");
        ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : red; }");
    }
    else
    {
        this->ui->label_PowerSupplyStatus->setText("N/A");
        ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : black; }");
    }
}

void MainWindow::showTcpServerState(bool started)
{
    if( started )
    {
        this->ui->connectionStatusLabel_TcpServerState->setText( "STARTED" );
        this->ui->connectionStatusLabel_TcpServerState->setStyleSheet("QLabel { color : green; }");
    }
    else
    {
        this->ui->connectionStatusLabel_TcpServerState->setText("STOPPED");
        this->ui->connectionStatusLabel_TcpServerState->setStyleSheet("QLabel { color : red; }");
    }
}

void MainWindow::on_pushButton_updateList_clicked()
{
    //Cleaning combolist
    ui->comboBox_serialSlots->clear();

    QList<QString> serialPorts = SerialComM::getSerialPorts();

    //Filling combo list
    foreach (QString serialPort, serialPorts)
    {
        ui->comboBox_serialSlots->addItem(serialPort);
    }

    //Send some debug info
    qDebug() << ("SUCCESS: Serial ports list updated!");
}

void MainWindow::on_pushButton_connectSerialPort_clicked()
{
    //Getting portname and baudrate
    QString portName = ui->comboBox_serialSlots->currentText();

    if(serialPort != Q_NULLPTR)
    {
        if(serialPort->isOpen())
        {
            qDebug() << ("ERROR: You must disconnect first!");
            return;
        }
    }

    serialPort = new SerialComM();

    //connecting serial signals
//    connect(serialPort, SIGNAL(connectionStatusChanged(bool)), this, SLOT(setSerialPortStatus(bool)));
//    connect(serialPort, SIGNAL(readyRead()), this, SLOT(serialDataReceivingSlot()));


    if(serialPort->connect(portName, this->baud))
    {
        qDebug() << ("SUCCESS: CONNECTED!!1");
        this->showSerialPortStatus(true);
    }
    else
    {
        qDebug() << ("FAILED2CONN: " + serialPort->getLastError());
        if(serialPort != Q_NULLPTR || serialPort)
        {
            delete serialPort;
            serialPort = Q_NULLPTR;
        }
    }
}

void MainWindow::on_pushButton_disconnectSerialPort_clicked()
{
    if(serialPort == Q_NULLPTR || serialPort == 0 || !this->serialPort->isOpen())
    {
        qDebug() << ("FAILED: You're not connected to serial port!");
        return;
    }    

    if(this->serialPort->isOpen())
    {
        if(this->serialPort->disconnect())
        {
            qDebug() << ("SUCCESS: DISCONNECTED!");
            this->showSerialPortStatus(false);
        }
        else
        {
            qDebug() << ("FAILED: " + serialPort->getLastError());
            return;
        }
    }
    else
    {
        qDebug() << ("FAILED: " + serialPort->getLastError());
    }
}

void MainWindow::on_comboBox_BaudRates_currentIndexChanged(int index)
{
    this->baud = (SerialComM::BaudRate) ui->comboBox_BaudRates->itemText(index).toInt();
}

void MainWindow::on_radioButton_powerON_clicked()
{
    if( !this->powerSupply )
    {
        qDebug() << "No power supply handler created.";
        return;
    }

    this->powerSupply->PowerON();
}

void MainWindow::on_radioButton_powerOFF_clicked()
{
    if( !this->powerSupply )
    {
        qDebug() << "No power supply handler created.";
        return;
    }

    this->powerSupply->PowerOFF();

}

void MainWindow::on_pushButton_refreshPowerState_clicked()
{
    if( !this->powerSupply )
    {
        qDebug() << "No power supply handler created.";
        return;
    }

    this->showPowerSupplyState(this->powerSupply->GetPowerState());
}
