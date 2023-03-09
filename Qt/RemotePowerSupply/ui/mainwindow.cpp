#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Logger.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->ui->comboBox_BaudRates->addItems({"1200", "2400", "4800", "9600", "14400", "19200", "38400", "57600", "115200"});
    this->ui->comboBox_BaudRates->setCurrentIndex(3);
    this->baud = QSerialPort::Baud9600;

    this->on_pushButton_updateList_clicked();

    // Select last index from the list as this is most usual
    this->ui->comboBox_serialSlots->setCurrentIndex(this->ui->comboBox_serialSlots->count() - 1);

    this->setFixedSize(this->width(), this->height());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLogs_triggered()
{
    loggerContentGui->setWindowTitle("Application logs");
    loggerContentGui->setReadOnly(true);
    loggerContentGui->setMinimumSize(QSize(800, 400));
    loggerContentGui->showNormal();
}

void MainWindow::on_actionExit_triggered()
{
    qApp->quit();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, tr("About"), tr("Hacked PeakTech 6225A to make it remote controllable"), QMessageBox::Ok);
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
        qInfo().nospace().noquote() << "[SERIAL PORT] Disconnected";
        this->ui->connectionStatusLabel->setText("NOT CONNECTED");
        this->ui->connectionStatusLabel->setStyleSheet("QLabel { color : red; }");
    }
}

void MainWindow::showPowerSupplyState(power_supply_state_t state)
{
    if( state == POWER_ON )
    {
        this->ui->label_PowerSupplyStatus->setText("ON");
        this->ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : green; }");
        this->ui->radioButton_powerON->setChecked(true);

        // Show light bulf OFF
        QImage img(":/on.png");
        QPixmap pm = QPixmap::fromImage(img);
        this->ui->label_LightBulb->setPixmap(pm);
    }
    else if( state == POWER_OFF )
    {
        this->ui->label_PowerSupplyStatus->setText("OFF");
        this->ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : red; }");
        this->ui->radioButton_powerOFF->setChecked(true);

        // Show light bulf OFF
        QImage img(":/off.png");
        QPixmap pm = QPixmap::fromImage(img);
        this->ui->label_LightBulb->setPixmap(pm);
    }
    else
    {
        this->ui->label_PowerSupplyStatus->setText("N/A");
        this->ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : black; }");
        this->ui->radioButton_powerON->setChecked(false);
        this->ui->radioButton_powerOFF->setChecked(false);
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
    qDebug().nospace().noquote() << "[SERIAL PORT]: Serial ports list updated!";
}

void MainWindow::on_pushButton_connectSerialPort_clicked()
{
    //Getting portname and baudrate
    QString portName = ui->comboBox_serialSlots->currentText();

    if(serialPort != Q_NULLPTR)
    {
        if(serialPort->isOpen())
        {
            qWarning().nospace().noquote()<< ("[SERIAL PORT] ERROR: You must disconnect first!");
            return;
        }
    }

    serialPort = new SerialComM();

    //connecting serial signals
    connect(this->serialPort, SIGNAL(connectionStatusChanged(bool)), this, SLOT(showSerialPortStatus(bool)));

    if(serialPort->connect(portName, this->baud))
    {
        qInfo().nospace().noquote()<< "[SERIAL PORT] SUCCESS: CONNECTED to " << portName;
        this->showSerialPortStatus(true);
    }
    else
    {
        qWarning().nospace().noquote()<< "[SERIAL PORT] FAILED2CONN: " << serialPort->getLastError();
        if(serialPort != Q_NULLPTR || serialPort)
        {
            delete serialPort;
            serialPort = Q_NULLPTR;
        }
    }

    // Create power supply handler
    if( this->serialPort )
    {
        this->powerSupply = new PowerSupplyM(this->serialPort);
        connect(this->powerSupply, SIGNAL(PowerStatusChanged(power_supply_state_t)), this, SLOT(showPowerSupplyState(power_supply_state_t)));
    }
}

void MainWindow::on_pushButton_disconnectSerialPort_clicked()
{
    if(serialPort == Q_NULLPTR || serialPort == 0 || !this->serialPort->isOpen())
    {
        qWarning().nospace().noquote()<< "[SERIAL PORT] FAILED: You're not connected to serial port!";
        return;
    }    

    if(this->serialPort->isOpen())
    {
        if(this->serialPort->disconnect())
        {
            this->showSerialPortStatus(false);
        }
        else
        {
            qWarning().nospace().noquote()<< "[SERIAL PORT] FAILED: " << serialPort->getLastError();
            return;
        }
    }
    else
    {
        qWarning().nospace().noquote()<< "[SERIAL PORT] FAILED: " << serialPort->getLastError();
    }

    delete this->serialPort;
    this->serialPort = Q_NULLPTR;

    delete this->powerSupply;
    this->powerSupply = Q_NULLPTR;
}

void MainWindow::on_comboBox_BaudRates_currentIndexChanged(int index)
{
    this->baud = (SerialComM::BaudRate) ui->comboBox_BaudRates->itemText(index).toInt();
}

void MainWindow::on_radioButton_powerON_clicked()
{
    if( this->powerSupply == Q_NULLPTR)
    {
        qWarning().nospace().noquote()<< "No power supply handler created.";
        return;
    }

    this->powerSupply->PowerON();
}

void MainWindow::on_radioButton_powerOFF_clicked()
{
    if( this->powerSupply == Q_NULLPTR)
    {
        qWarning().nospace().noquote()<< "No power supply handler created.";
        return;
    }

    this->powerSupply->PowerOFF();

}

void MainWindow::on_pushButton_refreshPowerState_clicked()
{
    if( this->powerSupply == Q_NULLPTR)
    {
        qWarning().nospace().noquote()<< "No power supply handler created.";
        return;
    }

    this->powerSupply->RequestPowerState();
}

void MainWindow::on_pushButton_StartTcpServer_clicked()
{
    if( this->server == Q_NULLPTR)
    {
        this->server = new TcpComM(QHostAddress(this->ui->lineEdit_TcpServerIp->text()), QString(this->ui->lineEdit_TcpServerPort->text()).toUInt(), 3000 );

        connect(this->server, SIGNAL(TcpClientConnected(QTcpSocket*)), this, SLOT(on_TcpClientConnected(QTcpSocket*)));
        connect(this->server, SIGNAL(TcpClientDisconnected(QTcpSocket*)), this, SLOT(on_TcpClientDisconnected(QTcpSocket*)));
        connect(this->server, SIGNAL(TcpClientDataReception(QTcpSocket*,QByteArray)), this, SLOT(on_TcpClientDataReception(QTcpSocket*,QByteArray)));

        this->showTcpServerState(true);
    }
}

void MainWindow::on_pushButton_StopTcpServer_clicked()
{
    if( this->server != Q_NULLPTR )
    {
        delete this->server;
        this->server = Q_NULLPTR;

        this->showTcpServerState(false);
        this->ui->label_tcpClientsConnected->setText( "0" );
    }
}

void MainWindow::on_TcpClientConnected(QTcpSocket *client)
{
    int curr_clients = this->ui->label_tcpClientsConnected->text().toInt();
    this->ui->label_tcpClientsConnected->setText( QString::number(++curr_clients) );

    qInfo().nospace().noquote() <<"[TCP SERVER] ["<<client->localAddress().toString()<<":"<<client->localPort()<<"] CONNECTED";

    client->write("Welcome to PeakTech 6225A Remote Control\n");
    client->write("Available commands: PWR_ON, PWR_OFF, ?\n\n");
}

void MainWindow::on_TcpClientDisconnected(QTcpSocket *client)
{
    int curr_clients = this->ui->label_tcpClientsConnected->text().toInt();
    this->ui->label_tcpClientsConnected->setText( QString::number(--curr_clients) );
    qInfo().nospace().noquote() <<"[TCP SERVER] ["<<client->localAddress().toString()<<":"<<client->localPort()<<"] DISCONNECTED";
}

void MainWindow::on_TcpClientDataReception(QTcpSocket *client, QByteArray bytes)
{
    qInfo().nospace().noquote() <<"[TCP SERVER] ["<<client->localAddress().toString()<<":"<<client->localPort()<<"] RECV <: " << QString(bytes).replace("\n", "\\n").replace("\r", "\\r");

    QString data = QString(bytes).trimmed();

    if( this->powerSupply == Q_NULLPTR )
    {
        qWarning().nospace().noquote() << "No power supply connected";
        client->write(" ->NOT OK\n");
        return;
    }

    if( data == "PWR_OFF" )
    {
        if( this->powerSupply->PowerOFF())
            client->write(" ->OK\n");
        else
            client->write(" ->NOT OK\n");
    }
    else if (data == "PWR_ON")
    {
        if(this->powerSupply->PowerON())
            client->write(" ->OK\n");
        else
            client->write(" ->NOT OK\n");
    }
    else if( data == "?" )
    {
        power_supply_state_t state = this->powerSupply->ReadPowerState();
        if( state == POWER_ON )
            client->write(" ->PWR_ON\n");
        else if (state == POWER_OFF)
            client->write(" ->PWR_OFF\n");
        else
            client->write(" ->PWR_INVALID\n");
    }
    else
    {
        qWarning().nospace().noquote()<< "[TCP SERVER] Invalid command received: "<<data;
        client->write(" ->INVALID COMMAND\n");
    }

    // Close connection after command was received
    client->disconnectFromHost();
    client->close();
}

void MainWindow::on_checkBox_StickToTop_stateChanged(int arg1)
{
#ifdef _WIN32
    if (arg1)
    {
        SetWindowPos(reinterpret_cast<HWND>(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    else
    {
        SetWindowPos(reinterpret_cast<HWND>(this->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
#else
    Qt::WindowFlags flags = this->windowFlags();
    if (checked)
    {
        this->setWindowFlags(flags | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
        this->show();
    }
    else
    {
        this->setWindowFlags(flags ^ (Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint));
        this->show();
    }
#endif
}

