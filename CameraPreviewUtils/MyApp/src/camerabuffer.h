#ifndef CAMERABUFFER_H
#define CAMERABUFFER_H

#include <QObject>
#include <QMutex>

class CameraBuffer : public QObject
{
    Q_OBJECT
public:
    explicit CameraBuffer(QObject *parent = 0);
    ~CameraBuffer();

    void ref();
    void unref();

    void *buffer;
    int size;
    bool isUsed;

    void *user_data;

    int dump();
signals:
    void release(CameraBuffer *);

public slots:

private:
    int refCount;
    QMutex *mutex;
};

#endif // CAMERABUFFER_H
