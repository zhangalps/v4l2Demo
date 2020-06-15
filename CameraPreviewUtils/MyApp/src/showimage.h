#ifndef SHOWIMAGE_H
#define SHOWIMAGE_H

#include <QObject>
#include <QImage>
#include "cameracaptureprovider.h"
#include <QTime>
#include "gstcamerarecorder.h"
#include "camerabuffer.h"
#include "camerasrc.h"
#include <gst/gst.h>
#include "configutils.h"
#include "camerainterface.h"
#include "videodevice.h"
#include "capthread.h"

class ShowImage : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.syberos.camerautils.interface")
public:

    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void stopCamera();
    Q_INVOKABLE void startCameraCapture();
    Q_INVOKABLE void startRecorder();
    Q_INVOKABLE void stopRecorder();
    Q_INVOKABLE void switchCamera(QString);
    Q_INVOKABLE void backKey();
    Q_INVOKABLE int getCameraPosition();




    explicit ShowImage(QObject *parent = NULL);
    CameraCaptureProvider *provider ;

public slots:

    void onCapFrame(uchar *frame);
    void onCameraPosition(int);
    void onYuvReceived(uchar *frame,unsigned int len);
    void onCallCaptureSuc();


signals:
    void callQmlRefeshImg();
    void newCameraRecorderBuffer(CameraBuffer*);
    void callQmlCameraStatus(int status);// 0 open,1 close,2 ing,3 已经打开，不用操作
    void callQmlCameraPosition(QString position);
    void callQmlCaptureSuc();

public:
//    uchar *rgbData;
    int width = 800;
    int height = 600;
    int mallocSize =  width*height*3;
    int methodId = 2;
    int qmlPosition = 0;


private:
    GstCameraRecorder *mGstCameraRecorder;
    bool isRecording;
    CameraSrc *mFpdlinkCameraSrc;
    ConfigUtils *_configUtils;
    CameraInterface *cameraInterface;
    VideoDevice *vd;
    bool isCameraOpen;
    bool isCameraOpening;
    CapThread *capThread;
    QImage capImage;
    QImage capImage2;
    int rindex = 0;
    int windex = 0;
    void setCameraPosition(int position);

};

#endif // SHOWIMAGE_H
