#ifndef GSTCAMERACAPTURE_H
#define GSTCAMERACAPTURE_H

#include <QObject>
#include <gst/gst.h>
#include "camerabuffer.h"

#include <QMutex>
#include <QWaitCondition>

class GstCameraCapture : public QObject
{
    Q_OBJECT
    typedef struct UserData{
        GstElement *pipeline;
        GstElement *appsrc;
//        GstElement *videoconvert;
        GstElement *encoder;
        GstElement *filesink;
        GstElement *videoFlip;
        GstElement *taginject;

//        void *m_buffer;
        GstBus *bus;
        QMutex *mutex;
        QWaitCondition *cond;
        CameraBuffer *cameraBuffer;
        bool bufBusy;
    }UserData;

public:
    explicit GstCameraCapture(QObject *parent = 0);
    ~GstCameraCapture();
    UserData mData;

    void create(int methodId);
    void capture(QString);

    static void gst_buffer_free(gpointer user_data);
    static void cb_need_data(GstElement *element, guint unused_size, gpointer user_data);
    static void cb_message (GstBus *bus, GstMessage *msg, gpointer user_data);

    QString curImageName;
    int width = 800;
    int height = 600;

signals:
    void cameraCaptured();

public slots:
//    void newCameraCaptureBuffer(char *data, int size);
    void newCameraCaptureBuffer(CameraBuffer*);

private:

};

#endif // GSTCAMERACAPTURE_H
