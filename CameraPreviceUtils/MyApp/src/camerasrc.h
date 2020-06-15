#ifndef CAMERASRC_H
#define CAMERASRC_H

#include <QObject>
#include <QMutex>
#include <QList>

//#include <hybris/syberos/camera/camera_compatibility_layer_capabilities.h>
#include "camera_compatibility_layer_capabilities_v2.h"
#include "camerabuffer.h"


class CameraSrc : public QObject
{
    Q_OBJECT
public:
    enum CameraType{
        FPDLINK_CAMERA,
        PAL_CAMERA
    };

    CameraSrc(CameraType type, QObject *parent = 0);
    ~CameraSrc();
//    void open();
//    void close();
    void start();
    void stop();
    void initBufferList();
    void deinitBufferList();
    void requireBuffer(CameraBuffer **buf, void *buffer);

    QList<CameraBuffer*> *list;

signals:
    void newVideoBuffer(char *, int size);
    void newCameraBuffer(CameraBuffer*);

public slots:
    void releaseBuffer(CameraBuffer *buf);

private:
    static void on_data_raw_image_cb(void *data, int size, void *userData);
    CameraControlListener listener;
    CameraType type;
    QMutex mutex;
//    bool opened;
    bool running;
    CameraControl *control;
    int ref;


};

#endif // CAMERASRC_H
