#include "sadadpos.h"

SadadPos::SadadPos(QObject *parent) : AbstractPCPos<SadadPos>(parent)
{
    setPort(8888);
}

SadadPos::~SadadPos()
{

}

void SadadPos::start(qint32 price, std::function<void (Status)> callback)
{
    if (mSocket)
        return;

    mOnPay = false;
    auto socket = new QTcpSocket(this);

    mSocket = socket;
    connect(socket, &QTcpSocket::connected, [callback, socket](){
        callback(Initializing);

        SadadPacket pk(SadadPacket::PacketType::INM);
        socket->write(toArray(pk.data()));
        socket->flush();
    });
    connect(socket, &QTcpSocket::readyRead, [this, socket, callback, price](){
        const auto bytes = socket->readAll();

        SadadPacket pk;
        if (!SadadPacket::fromData( pk, toVector(bytes) ))
            return;

        switch(static_cast<int>(pk.type()))
        {
        case (int)SadadPacket::PacketType::RESPONSE:
        {
            if (pk.responce_code() == 0 && pk.price() == price)
                callback(PaidSuccessfully);
            else
                callback(Failed);

            socket->disconnectFromHost();
        }
            break;

        case (int)SadadPacket::PacketType::ACK:
        {
            if (mOnPay)
                return;

            SadadPacket pk;
            pk.setPrice(price);
            pk.setOrder_id(321654);
            socket->write(toArray(pk.data()));

            mOnPay = true;
            callback(OnThePay);
        }
            break;

        default:
        case (int)SadadPacket::PacketType::NACK:
            callback(Failed);
            socket->disconnectFromHost();
            break;
        }
    });
    connect(socket, &QTcpSocket::disconnected, [this, callback, socket](){
        if (mSocket == socket)
            mSocket = Q_NULLPTR;

        socket->deleteLater();
        callback(Finished);
    });

    callback(Connecting);
    socket->connectToHost(hostAddress(), port());
}

void SadadPos::stop()
{
    if (!mSocket)
        return;

    mSocket->disconnectFromHost();
    mSocket = Q_NULLPTR;
}

QString SadadPos::displayName() const
{
    return tr("Sadad");
}

QByteArray SadadPos::toArray(const SadadPacket::PacketResult &list)
{
    QByteArray res;
    for (const auto l: list)
        res += (uchar)l;
    return res;
}

SadadPacket::PacketResult SadadPos::toVector(const QByteArray &bytes)
{
    SadadPacket::PacketResult res;
    for (const auto b: bytes)
        res.push_back(b);
    return res;
}
