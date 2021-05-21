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

void MainWindow::on_pushButton_updateList_clicked()
{
    //Cleaning combolist
    ui->comboBox_serialSlots->clear();

    QList<QString> serialPorts = SerialClient::getSerialPorts();

    //Filling combo list
    foreach (QString serialPort, serialPorts)
    {
        ui->comboBox_serialSlots->addItem(serialPort);
    }

    //Send some debug info
    this->consoleLog("SUCCESS: Serial ports list updated!");
}

void MainWindow::on_pushButton_connectSerialPort_clicked()
{
    //Getting portname and baudrate
    QString portName = ui->comboBox_serialSlots->currentText();

    if(serialPort != Q_NULLPTR)
    {
        if(serialPort->isOpen())
        {
            consoleLog("ERROR: You must disconnect first!");
            return;
        }
    }

    serialPort = new SerialClient();

    //connecting serial signals
    connect(serialPort, SIGNAL(connectionStatusChanged(bool)), this, SLOT(setSerialPortStatus(bool)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(serialDataReceivingSlot()));


    if(serialPort->connect(portName, this->baud))
    {
        this->consoleLog("SUCCESS: CONNECTED!!1");
    }
    else
    {
        this->consoleLog("FAILED2CONN: " + serialPort->getLastError());
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
        this->consoleLog("FAILED: You're not connected to serial port!");
        return;
    }    

    if(this->serialPort->isOpen())
    {
        if(this->serialPort->disconnect())
        {
            this->consoleLog("SUCCESS: DISCONNECTED!");
            this->setSerialPortStatus(false);
        }
        else
        {
            this->consoleLog("FAILED: " + serialPort->getLastError());
            return;
        }
    }
    else
    {
        this->consoleLog("FAILED: " + serialPort->getLastError());
    }
}

void MainWindow::consoleLog(QString str)
{
    this->ui->testEditSerialConsole->append("[" + QTime::currentTime().toString("hh:mm:ss.zzz") + "] " +  str);
}

qint64 MainWindow::SendSerialData(QByteArray bytesToSend)
{
    if(serialPort == Q_NULLPTR)
    {
        this->consoleLog("SEND FAILED: Please connect to serial port first!");
        return -1;
    }

    /* Try send data via serial */
    qint64 bytesWritten = serialPort->write(bytesToSend);

    /* Check how many bytes were actually written */
    if( bytesWritten <= 0)
    {
        this->consoleLog("FAILED: " + serialPort->getLastError());
        return -1;
    }

    /* Display a warning if not all bytes were send */
    if(bytesWritten != bytesToSend.length())
    {
        QMessageBox::warning(this, "WARNING", "Only " + QString::number(bytesWritten) + " / " + QString::number(bytesToSend.length()) + " bytes were send!");
    }

    /* Display send data */
    QString dbgStr = "SEND (" + QString::number(bytesWritten) + "bytes): ";

    //dbgStr += QString( bytesToSend.toHex() );

    for(int i = 0; i < bytesWritten; ++i)
        dbgStr += QString("%1 ").arg(bytesToSend[i], 2, 16, QChar('0')).toUpper().remove("FFFFFFFFFFFFFF");
    dbgStr.chop(1);

    /* Send text to console */
    this->consoleLog(dbgStr);

    return bytesWritten;

}

void MainWindow::on_pushButton_Send_clicked()
{

    /* Get data from user interface parse it*/
    QByteArray bytesToSend;
    if(ui->radioButton_Hex->isChecked() == true)
    {
        bytesToSend = QByteArray::fromHex ( ui->lineEditData->text().remove(" ").toLatin1() );
    }
    else if(ui->radioButton_String->isChecked() == true)
    {
        bytesToSend = ui->lineEditData->text().toUtf8();
    }
    else
    {
        this->consoleLog("FAILED: No data format selected");
        return;
    }

    /* Try send data via serial */
    this->SendSerialData(bytesToSend);

}

void MainWindow::on_pushButton_Clear_clicked()
{
    ui->lineEditData->clear();
}

void MainWindow::setSerialPortStatus(bool connected)
{
    this->ui->connectionStatusLabel->setText( connected==true?("CONNECTED"):("NOT CONNECTED") );
}

void MainWindow::serialDataReceivingSlot()
{
    /* Received data/chunk info */
    const char *recvBuffer;
    qint64 recvBytes = 0;

    if(ui->radioButton_displayRAW->isChecked() == true || ui->radioButton_displayChunks->isChecked() == true)
    {
        char tmp[8192] = {0};
        recvBytes = serialPort->read(tmp, sizeof(tmp));
        recvBuffer = tmp;
    }
    else if(ui->radioButton_displayNewlines->isChecked() == true)
    {
        QString recvStr = serialPort->readLine().remove('\r').remove('\n');
        recvBuffer = recvStr.toLocal8Bit().constData();
        recvBytes = strlen(recvBuffer);
    }
    else if(ui->radioButton_displayStrings->isChecked() == true)
    {
        QString recvStr = serialPort->readString().remove('\r').remove('\n');
        recvBuffer = recvStr.toLocal8Bit().constData();
        recvBytes = strlen(recvBuffer);
    }
    else
    {
        consoleLog("Select reading function!");
    }

    /* If no data was received just continue */
    if( recvBytes == 0 )
        return;

    /* Select format do display data on console */
    if( ui->radioButton_String->isChecked() == true )
    {
        if(ui->radioButton_displayRAW->isChecked() == true)
        {
            this->ui->testEditSerialConsole->moveCursor(QTextCursor::End);
            this->ui->testEditSerialConsole->insertPlainText( recvBuffer );
            this->ui->testEditSerialConsole->moveCursor(QTextCursor::End);
        }
        else
        {
            consoleLog("RECV (" + QString::number(recvBytes) + " bytes): " + QString::fromLocal8Bit(recvBuffer).replace("\r\n", "\n") );
        }
    }
    else if(ui->radioButton_Hex->isChecked() == true)
    {
        QString resultHex;
        for(qint64 i = 0; i < recvBytes; ++i)
            resultHex += QString("%1 ").arg(recvBuffer[i], 2, 16, QChar('0')).toUpper().remove("FFFFFFFFFFFFFF");
        resultHex.chop(1);

        if(ui->radioButton_displayRAW->isChecked() == true)
        {
            this->ui->testEditSerialConsole->moveCursor(QTextCursor::End);
            this->ui->testEditSerialConsole->insertPlainText( resultHex + " ");
            this->ui->testEditSerialConsole->moveCursor(QTextCursor::End);
        }
        else
        {
            consoleLog("RECV (" + QString::number(recvBytes) + " bytes): " + resultHex);
        }
    }
    else
    {
        consoleLog("Output format not selected!");
    }
}

void MainWindow::on_PacketReceived(QString power_supply_packet)
{

}

void MainWindow::on_comboBox_BaudRates_currentIndexChanged(int index)
{
    this->baud = (SerialClient::BaudRate) ui->comboBox_BaudRates->itemText(index).toInt();
}

void MainWindow::on_pushButton_ClearConsole_clicked()
{
    ui->testEditSerialConsole->clear();
}

void MainWindow::on_radioButton_powerON_clicked()
{
    QByteArray data;
    data[0] = '#';
    data[1] = 0x00;
    data[2] = 0x06;

    data.append(QString("PWR_ON").toUtf8());
    SendSerialData(data);

    ui->label_PowerSupplyStatus->setText("ON");
    ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : green; }");
}

void MainWindow::on_radioButton_powerOFF_clicked()
{
    QByteArray data;
    data[0] = '#';
    data[1] = 0x00;
    data[2] = 0x07;

    data.append(QString("PWR_OFF").toUtf8());
    SendSerialData(data);

    ui->label_PowerSupplyStatus->setText("OFF");
    ui->label_PowerSupplyStatus->setStyleSheet("QLabel { color : red; }");
}

void MainWindow::on_pushButton_refreshPowerState_clicked()
{
    QByteArray data;
    data[0] = '#';
    data[1] = 0x00;
    data[2] = 0x01;

    data.append(QString("?").toUtf8());
    SendSerialData(data);
}
