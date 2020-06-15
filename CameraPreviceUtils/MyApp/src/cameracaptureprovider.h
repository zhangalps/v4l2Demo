#ifndef CAMERACAPTUREPROVIDER_H
#define CAMERACAPTUREPROVIDER_H

#include <QQuickImageProvider>
#include <QObject>
#include <QImage>

class CameraCaptureProvider : public QQuickImageProvider
{
public:
    CameraCaptureProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);
    QImage img;
};

#endif // CAMERACAPTUREPROVIDER_H
