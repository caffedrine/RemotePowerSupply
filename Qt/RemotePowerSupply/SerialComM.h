#ifndef SERIALCONN_H
#define SERIALCONN_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QList>
#include <QByteArray>
#include <QDebug>
#include <QTime>

typedef enum
{
    WAIT_START_TAG,
    WAIT_LENGTH,
    WAIT_PACKET_TYPE,
    WAIT_PACKET_PAYLOAD
} serial_comm_packet_read_state_t;

typedef struct
{
    uint16_t Length;
    uint16_t Type;
    QByteArray Payload;
}serial_comm_packet_t;

class SerialComM: public QObject
{
	Q_OBJECT

public:
	typedef QSerialPort::BaudRate BaudRate;	//let user choose his own boudrate

    SerialComM();
    ~SerialComM();

private:
	//Serial port handler
	QSerialPort *pSerialPort = Q_NULLPTR;

	//Store last error
	QString lastError;

public:
    qint64 ReadTimeoutMs;
    serial_comm_packet_read_state_t ReadState = WAIT_START_TAG;
    QByteArray ReadDataBuffer;
    serial_comm_packet_t CurrPacket;

	bool connect(QString portName, BaudRate baudRate);
	bool disconnect();

	static QList<QString> getSerialPorts();
	bool isOpen();
    bool SendPacket(quint16 packetType, QByteArray packetBytes);
	//Other methods
	QString getLastError();

private:
	void setLastError(QString error);

private slots:
	void connectionStatusChanged(QSerialPort::SerialPortError errNo);
    void readyRead();

signals:
    void packetReceived(quint16 packetType, QByteArray packeBytes);
	void connectionStatusChanged(bool connected);
};

#endif // SERIALCONN_H
