#ifndef CAMERAINTERFACE_H
#define CAMERAINTERFACE_H

#include <QObject>
//#include <QVideoFrame>

#include <QMutex>
#include <QWaitCondition>
#include <QImage>

#include "camerasrc.h"
#include "gstcameracapture.h"
#include "gstcamerarecorder.h"
#include "configutils.h"
#include "capthread.h"


class CameraInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.syberos.camerautils.interface")
public:
    explicit CameraInterface(QObject *parent = 0);
    ~CameraInterface();
    bool isCapturing;
    bool isRecording;
    void cameraBufferForCameraCapture(CameraBuffer*);
    int width = 800;
    int height = 600;
    int position = 0;
    int mallocSize =  width*height*3;
    int fMallocSize =  640*480*3;

    bool isBackCamera = true;//add by hjy
    int methodId = 2;


signals:

    void newCameraBuffer(CameraBuffer*);
    void newCameraCaptureBuffer(CameraBuffer*);
    void newCameraRecorderBuffer(CameraBuffer*);

    void needRestart();
    void paizhao(QImage image);
    void callCapFrame(uchar*);
    void callCameraPosition(int);
    void callCaptureSuc();
    void callVideoSuc();


public slots:
    void openCamera();
    void closeCamera();
    void hw_camera_capture();
    int hw_camera_start_record();
    int hw_camera_stop_record();
    void switchCamera(int);
    uchar* getMallocData();


    int SetPalChannel(int value);
    int GetPalChannel();

    int SetPalStream(int value);
    int GetPalStream();

    int SetGlassStream(int value);
    int GetGlassStream();
    void powerDownVideo();

    int test();

private slots:


    void cameraBufferForCameraRecorder(CameraBuffer*);
    void cameraBufferForRemoteVideoPlayer(CameraBuffer*);
    void setCurrentPic1(QImage image);

    void cameraCaptured();
    void recordStopped();
    void onCapFrame(uchar*);
    void onYuvReceived(uchar*,unsigned int);
    void onCameraPosition(int);


private:

    void loadState();
    void saveState();
    QImage getCurrentPic1();

    CameraSrc *mFpdlinkCameraSrc;
    GstCameraCapture *mGstCameraCapture;
    GstCameraRecorder *mGstCameraRecorder;
    QImage backupImage;
    QMutex mMutex;
    QWaitCondition mWaitCond;
    bool pendingRestart;
    bool isPlaying;
    bool skipBufFlag;
    ConfigUtils *_configUtils;
    QString curImageName;
    CapThread *capThread;
};

#endif // CAMERAINTERFACE_H
