#ifndef GSTCAMERARECORDER_H
#define GSTCAMERARECORDER_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <gst/gst.h>

#include "camerabuffer.h"

class GstCameraRecorder : public QObject
{
    Q_OBJECT
    typedef struct UserData{
        GstElement *pipeline;
        GstElement *appsrc;
        GstElement *videorate;
        GstElement *videoconvert;
        GstElement *capsfilter;
        GstElement *ffdeinterlace;
        GstElement *videoparse;
        GstElement *encoder;
        GstElement *h264parse;
        GstElement *mux;
        GstElement *filesink;
        GstElement *videoFlip;
        GstBus *bus;
        CameraBuffer *cameraBuffer;
        bool userStopped;
        bool bufBusy;
        int bufferCounter;
        bool isEOS;
    }UserData;

public:
    explicit GstCameraRecorder(QObject *parent = 0);
    ~GstCameraRecorder();


    void record();
    void stop();


    static void gst_buffer_free(gpointer data);
    static void cb_need_data(GstElement *element, guint unused_size, gpointer user_data);
    static void cb_message (GstBus *bus, GstMessage *msg, gpointer user_data);

    QMutex *mMutex;
    QWaitCondition *mCond;
    int width = 800;
    int height = 600;
    int methodId = 2;

signals:
    void recordStopped();

public slots:
    //    void newCameraRecorderBuffer(char *data, int size);
    void newCameraRecorderBuffer(CameraBuffer*);

private:
    void create(int methodId);
    void destroy();

    void internalRecord();

    UserData mData;
    bool isCreated;
};

#endif // GSTCAMERARECORDER_H
