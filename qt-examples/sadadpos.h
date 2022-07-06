#ifndef SADADPOS_H
#define SADADPOS_H

#include "pcposmanager.h"
#include "sadadpacket.h"

#include <QTcpSocket>
#include <QPointer>

class SadadPos : public AbstractPCPos<SadadPos>
{
    Q_OBJECT
public:
    SadadPos(QObject *parent = nullptr);
    virtual ~SadadPos();

    void start(qint32 price, std::function<void (Status)> callback);
    void stop();
    bool running() const {
        return mSocket;
    }
    QString displayName() const;

protected:
    static QByteArray toArray(const SadadPacket::PacketResult &list);
    static SadadPacket::PacketResult toVector(const QByteArray &bytes);

private:
    QPointer<QTcpSocket> mSocket;
    bool mOnPay = false;
};

#endif // SADADPOS_H
