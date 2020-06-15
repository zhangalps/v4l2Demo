#include "camerasrc.h"
#include <QDebug>
#include <QDateTime>

#include "stdio.h"

#define N_BUFFERS 4 // From native

#define WIDTH 480
#define HEIGHT 320
static const int PAL_CAMERA = 0;
static const int FPDLINK_CAMERA = 1;
static int cameraswitch = PAL_CAMERA;

void CameraSrc::on_data_raw_image_cb(void *data, int size, void *userData)
{

    CameraSrc *cameraSrc = (CameraSrc*)userData;

    //    QDateTime current_date_time = QDateTime::currentDateTime();
    //    QString current_date = current_date_time.toString("yyyy-MM-dd");
    //    QString current_time = current_date_time.toString("hh:mm:ss.zzz");

    //    qDebug() << __func__ << "size:" << size << ",time:" << current_time << data;

    //    for (int i = 0; i < cameraSrc->list->size(); ++i) {
    //        CameraBuffer *tmpBuffer = cameraSrc->list->at(i);
    //        tmpBuffer->dump();
    //    }

    qDebug() << __func__ << "size:" << size << data;
    CameraBuffer *buf;
    cameraSrc->requireBuffer(&buf, data);

    emit cameraSrc->newCameraBuffer(buf);
    buf->unref();
}


CameraSrc::CameraSrc(CameraType type, QObject *parent) : QObject(parent),
    control(NULL), ref(0), running(false)
{
    this->type = type;
    listener.on_data_raw_image_cb = on_data_raw_image_cb;
    listener.context = this;

    //    initBufferList();
}

CameraSrc::~CameraSrc()
{
    //    deinitBufferList();
}

void CameraSrc::start()
{
    mutex.lock();
    if(running){
        mutex.unlock();
        return;
    }
    initBufferList();
    //    android_camera_init();
    printf("chenxin test android_camera_init\n");
    //    if(type == FPDLINK_CAMERA){
    //        control = android_camera_open(BACK_FACING_CAMERA_TYPE, &listener);
    //    }else if(type == PAL_CAMERA){
    //        control = android_camera_open(FRONT_FACING_CAMERA_TYPE, &listener);
    //    }
    if(control == NULL){
        printf("%s, %s, %d: open camera type:%d failed\n", __FILE__, __FUNCTION__, __LINE__, type);
        mutex.unlock();
        return;
    }
    printf("chenxin test android_camera_open\n");
    //    android_camera_start_preview(control);
    printf("chenxin test android_start_preview\n");
    running = true;
    mutex.unlock();
}

void CameraSrc::stop()
{
    mutex.lock();
    if(!running){
        mutex.unlock();
        return;
    }
    if(control){
        //        android_camera_stop_preview(control);
        //        android_camera_close(control);
    }
    deinitBufferList();
    running = false;
    mutex.unlock();
}

//void CameraSrc::open()
//{
//    mutex.lock();
//    if(!opened){
//        start();
//        opened = true;
//    }
//    ref++;
//    mutex.unlock();
//}

//void CameraSrc::close()
//{
//    mutex.lock();
//    ref--;
//    if(opened && ref == 0){
//        stop();
//        opened = false;
//    }
//    mutex.unlock();
//}

void CameraSrc::initBufferList()
{
    list = new QList<CameraBuffer*>();
    for(int i = 0; i < N_BUFFERS; i++){
        CameraBuffer *buf = new CameraBuffer();
        buf->buffer = NULL;
        if(type == FPDLINK_CAMERA){
            buf->size = 640 * 480 * 2;
        }else if(type == PAL_CAMERA){
            buf->size = 480 * 320 * 2;
        }
        connect(buf, SIGNAL(release(CameraBuffer*)), this, SLOT(releaseBuffer(CameraBuffer*)), Qt::DirectConnection);
        list->append(buf);
    }
}

void CameraSrc::deinitBufferList()
{
    if(!list){
        return;
    }
    for(int i = 0; i < list->size(); i++){
        CameraBuffer *buf = list->at(i);
        disconnect(buf, SIGNAL(release(CameraBuffer*)), this, SLOT(releaseBuffer(CameraBuffer*)));
        delete buf;
    }
    delete list;
    list = NULL;
}

void CameraSrc::requireBuffer(CameraBuffer **buf, void *buffer)
{
    for (int i = 0; i < list->size(); ++i) {
        CameraBuffer *tmpBuffer = list->at(i);
        if (!tmpBuffer->buffer){
            //            buf->isUsed = true;
            tmpBuffer->buffer = buffer;
            *buf = tmpBuffer;
            (*buf)->ref();
            return;
        }else if(tmpBuffer->buffer == buffer){
            *buf = tmpBuffer;
            (*buf)->ref();
            return;
        }
    }
}

void CameraSrc::releaseBuffer(CameraBuffer *buf)
{
    //    qDebug() << __func__;
    if(buf){
        qDebug() << __func__ << "release buffer" << buf->buffer;
        //        android_camera_release_recording_frame(this->control, buf->buffer);
    }
}
