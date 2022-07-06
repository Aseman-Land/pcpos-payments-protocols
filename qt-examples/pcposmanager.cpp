#include "pcposmanager.h"

QMap<QString, PCPosManager::Factory> PCPosManager::mFactories;

PCPosManager::PCPosManager(QObject *parent)
    : QObject(parent)
{
}

PCPosManager::~PCPosManager()
{
}

void PCPosManager::registerFactory(const QString &name, Factory f)
{
    mFactories[name] = f;
}

QStringList PCPosManager::keys()
{
    return mFactories.keys();
}

AbstractPCPosBase *PCPosManager::create(const QString &key)
{
    return mFactories.value(key)();
}

const QHostAddress &AbstractPCPosBase::hostAddress() const
{
    return mHostAddress;
}

void AbstractPCPosBase::setHostAddress(const QHostAddress &newHostAddress)
{
    mHostAddress = newHostAddress;
}

qint32 AbstractPCPosBase::port() const
{
    return mPort;
}

void AbstractPCPosBase::setPort(qint32 newPort)
{
    mPort = newPort;
}
