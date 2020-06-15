#include "cameracaptureprovider.h"


CameraCaptureProvider::CameraCaptureProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage CameraCaptureProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
//    QDebug()<<"requestImage image : "<<img;
    return this->img;
}

QPixmap CameraCaptureProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
//    qDebug()<<"requestPixmap image : "<<img;
    return QPixmap::fromImage(this->img);
}
