#ifndef SERIALCONN_H
#define SERIALCONN_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QList>
#include <QByteArray>
#include <QDebug>
#include <QTime>

class SerialClient: public QObject
{
	Q_OBJECT

public:
	typedef QSerialPort::BaudRate BaudRate;	//let user choose his own boudrate

    SerialClient();
    ~SerialClient();

private:
	//Serial port handler
	QSerialPort *pSerialPort = Q_NULLPTR;

	//Store last error
	QString lastError;

public:
	bool connect(QString portName, BaudRate baudRate);
	bool disconnect();

    qint64 write(QByteArray data);
    qint64 read(char *buffer, qint64 max_len);
	QString readString();
    QString readLine();

	static QList<QString> getSerialPorts();
	bool isOpen();

	//Other methods
	QString getLastError();

private:
	void setLastError(QString error);

private slots:
	void connectionStatusChanged(QSerialPort::SerialPortError errNo);

signals:
	void readyRead();
	void connectionStatusChanged(bool connected);
};

#endif // SERIALCONN_H
