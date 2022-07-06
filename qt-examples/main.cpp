#include <QCoreApplication>
#include <QDebug>
#include <QHostAddress>

#include "pcposmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    auto pos = PCPosManager::create("SadadPos");
    pos->setHostAddress(QHostAddress("192.168.0.100"));
    pos->setPort(8080);

    pos->start(100000, [pos](AbstractPCPosBase::Status s){
        qDebug() << "Status changed" << s;
        switch (static_cast<int>(s))
        {
        case AbstractPCPosBase::Status::Finished:
            qDebug() << "Finished";
            pos->deleteLater();
            qApp->quit();
            break;
        }
    });
    
    return app.exec();
}
