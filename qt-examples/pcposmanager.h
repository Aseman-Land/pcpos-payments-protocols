#ifndef PCPOSMANAGER_H
#define PCPOSMANAGER_H

#include <QObject>
#include <QList>
#include <QRegularExpression>
#include <QHostAddress>

class AbstractPCPosBase;
class PCPosManager : public QObject
{
    Q_OBJECT
public:
    typedef std::function<AbstractPCPosBase *()> Factory;

    PCPosManager(QObject *parent = nullptr);
    virtual ~PCPosManager();

    static QStringList keys();
    static AbstractPCPosBase *create(const QString &key);

    static void registerFactory(const QString &name, Factory f);

Q_SIGNALS:

private:
    static QMap<QString, Factory> mFactories;
};

class AbstractPCPosBase : public QObject
{
    Q_OBJECT
public:
    enum Status {
        Nothing = 0,
        Connecting = 1,
        Initializing = 2,
        OnThePay = 3,
        PaidSuccessfully = 4,
        Failed = 5,
        Finished = 6,
    };

    AbstractPCPosBase(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~AbstractPCPosBase() {}

    virtual void start(qint32 price, std::function<void (Status)> callback) = 0;
    virtual void stop() = 0;
    virtual bool running() const = 0;
    virtual QString displayName() const = 0;

    const QHostAddress &hostAddress() const;
    void setHostAddress(const QHostAddress &newHostAddress);

    qint32 port() const;
    void setPort(qint32 newPort);

private:
    QHostAddress mHostAddress;
    qint32 mPort = 0;
};

template<typename T, bool AutoCreation = true>
class AbstractPCPos: public AbstractPCPosBase
{
public:
    AbstractPCPos(QObject *parent = nullptr) : AbstractPCPosBase(parent) {}
    virtual ~AbstractPCPos() {}

    static QString name() {
        const auto type_name = typeid (T).name();
        return QString(type_name).remove(QRegularExpression("^\\d+")).remove("class ");
    }

protected:
    class methodRegistrator
    {
      public:
        methodRegistrator() {
            PCPosManager::registerFactory(name(), []() -> AbstractPCPosBase* { return new T; });
        }
    };

    static methodRegistrator registrator_;
    virtual void *touch()
    {
        return &registrator_;
    }
};

template <typename T, bool AutoCreation>
typename AbstractPCPos<T, AutoCreation>::methodRegistrator
    AbstractPCPos<T, AutoCreation>::registrator_;

#endif // PCPOSMANAGER_H
