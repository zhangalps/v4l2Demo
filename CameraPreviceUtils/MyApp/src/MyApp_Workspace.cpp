#include "MyApp_Workspace.h"
#include <QDebug>
#include "showimage.h"
#include <QtQuick>

#include <QDBusConnection>
#include <QDBusError>

MyApp_Workspace::MyApp_Workspace()
    : CWorkspace()
{
    m_view = SYBEROS::SyberosGuiCache::qQuickView();
    QObject::connect(m_view->engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    qDebug()<<"jake MyApp_Workspace ";


    QDBusConnection connection = QDBusConnection::sessionBus();

    if(!connection.registerService("com.syberos.camerautils"))
    {
        qDebug() << "error:" << connection.lastError().message();
    }

    ShowImage *CodeImage = new ShowImage(NULL);
    CodeImage->width = width;
    CodeImage->height = height;
    qDebug()<<"jake setContextProperty : CodeImage = "<< CodeImage;

    if(!connection.registerObject("/camerautils/interface", CodeImage, QDBusConnection::ExportAllSlots)){
         qDebug() << __func__ << "=============error!!!";
    }
    m_view->rootContext()->setContextProperty("capwidth",width);
    m_view->rootContext()->setContextProperty("capheight",height);

    m_view->rootContext()->setContextProperty("CodeImage",CodeImage);
    m_view->engine()->addImageProvider(QLatin1String("myImage"), CodeImage->provider);

    m_view->setSource(QUrl("qrc:/qml/main.qml"));
    m_view->showFullScreen();
}

void MyApp_Workspace::onLaunchComplete(Option option, const QStringList& params)
{
    Q_UNUSED(params)

    switch (option) {
    case CWorkspace::HOME:
        qDebug()<< "jake Start by Home";
        break;
    case CWorkspace::URL:
         qDebug()<< "jake Start by URL";
        break;
    case CWorkspace::EVENT:
         qDebug()<< "Start by EVENT";
        break;
    case CWorkspace::DOCUMENT:
         qDebug()<< "Start by DOCUMENT";
        break;
    default:
        break;
    }
}

