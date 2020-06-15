#include "camerabuffer.h"

#include <QDebug>

CameraBuffer::CameraBuffer(QObject *parent) : QObject(parent),
    refCount(0), buffer(NULL)
{
    mutex = new QMutex();
}

CameraBuffer::~CameraBuffer()
{
    delete mutex;
}

void CameraBuffer::ref()
{
//    qDebug() << __func__ << this->buffer;

    mutex->lock();
    refCount++;
    mutex->unlock();
}

void CameraBuffer::unref()
{
//    qDebug() << __func__ << this->buffer;
    mutex->lock();
    refCount--;
    if(refCount == 0){
//        qDebug() << __func__ << "release";
        emit release(this);
    }
    mutex->unlock();
}

int CameraBuffer::dump()
{
    mutex->lock();
    qDebug() << __func__
             << ",this:" << this
             << ",buffer:" << buffer
             << ",refCount:" << refCount;
    mutex->unlock();
}

