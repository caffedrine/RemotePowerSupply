#ifndef TCPCOMM_H
#define TCPCOMM_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>

class TcpComM: public QObject
{
        Q_OBJECT
    public:
        TcpComM(QHostAddress address, quint16 tcp_port);
        ~TcpComM();

    signals:

    public slots:
        void newConnection();
        void clientDisconnected();
        void clienDataAvailable();

    private:
        QHostAddress TcpAddress;
        quint16 TcpPort;

        QTcpServer *server = Q_NULLPTR;
        QTcpSocket *client = Q_NULLPTR;

    signals:
        void TcpClientConnected(QTcpSocket *client);
        void TcpClientDisconnected(QTcpSocket *client);
        void TcpClientDataReception(QTcpSocket *client, QByteArray bytes);
};

#endif // TCPCOMM_H
