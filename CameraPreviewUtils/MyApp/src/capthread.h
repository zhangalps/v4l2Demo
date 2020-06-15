#ifndef CAPTHREAD_H
#define CAPTHREAD_H
#include <QThread>
#include "videodevice.h"
#include <QTimer>



class CapThread: public QThread
{
    Q_OBJECT
public:
    explicit CapThread(QObject *parent = 0);
    ~CapThread();
    //    CapThread(QObject *parent);
    virtual void run();
    int exec();
    bool running;
    bool isSwitch = false;
    void switchCamera(int);
    void stopCap();
    void startCap();
    bool setFileType(QString filePath, int type);
    uchar *rgbData[4];
    uchar *fRgbData[4];
//    uchar *rgbData2;

    int width = 800;
    int height = 600;
    int index = 0;
    QString cameraPath = "/dev/video0";




public slots:
    void onCapError(QString);
    void onClearFrame();

signals:
    void captureRgb(uchar *);
    void captureYuv(uchar *frame,unsigned int len);
    void cameraPosition(int);

private:
    VideoDevice *vd;
    QTimer *timer;
    uchar *yuvData;
    unsigned int len;
    void onUpdate();
    int initCamera();
    int convert_yuv_to_rgb_pixel(int y, int u, int v);
    int convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height);
    bool getVideoFileExist(QString filterStr);
    bool isVideoSuc(QString postion);
};

#endif // CAPTHREAD_H
