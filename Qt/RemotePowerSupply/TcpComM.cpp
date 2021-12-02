#include "TcpComM.h"

TcpComM::TcpComM(QHostAddress address, quint16 tcp_port): TcpAddress(address), TcpPort(tcp_port)
{
    this->server = new QTcpServer(this);
    this->server->setMaxPendingConnections(1);

    // whenever a user connects, it will emit signal
    connect(this->server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    if(!this->server->listen(this->TcpAddress, this->TcpPort))
    {
        qDebug() << "TCP server could not start on "<<address.toString()<<":"<<tcp_port;
    }
    else
    {
        qDebug() << "TCP server started on "<<address.toString()<<":"<<tcp_port;
    }
}

TcpComM::~TcpComM()
{
    if( this->client != Q_NULLPTR )
    {
        if (this->client->isOpen())
            this->client->close();
        this->client = Q_NULLPTR;
    }

    if( this->server != Q_NULLPTR)
    {
        delete this->server;
        this->server = Q_NULLPTR;
        qDebug() << "TCP server stopped";
    }
}

void TcpComM::newConnection()
{
    // Client cannot stay connected more than one second

    if(!this->client || !this->client->isOpen())
    {
        // need to grab the socket
        this->client = this->server->nextPendingConnection();

        connect(this->client, SIGNAL(disconnected()),this, SLOT(clientDisconnected()));
        connect(this->client, SIGNAL(readyRead()),this, SLOT(clienDataAvailable()));

        emit this->TcpClientConnected(this->client);
    }
    else
    {
        this->server->nextPendingConnection()->close();
    }
}

void TcpComM::clientDisconnected()
{
    emit this->TcpClientDisconnected(this->client);
    this->client = Q_NULLPTR;
}

void TcpComM::clienDataAvailable()
{
    emit this->TcpClientDataReception(this->client, this->client->readAll());
}
