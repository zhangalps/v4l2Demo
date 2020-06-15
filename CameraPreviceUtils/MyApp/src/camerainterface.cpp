#include "camerainterface.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDateTime>
#include "fileutils.h"

#include <unistd.h>
#include <fcntl.h>
#include <QThread>

static QByteArray videoData;
#define FcameraPwrEnPath "/sys/misc-config/fcamera_pwr_en"
#define BcameraPwrEnPath "/sys/misc-config/bcamera_pwr_en"
#define FcameraDEVPath "/dev/video1"
#define BcameraDEVPath "/dev/video0"
CameraInterface::CameraInterface(QObject *parent) : QObject(parent),
    isCapturing(false),
    isRecording(false),
    isPlaying(false),
    skipBufFlag(false),
    pendingRestart(false)
{
    mFpdlinkCameraSrc = new CameraSrc(CameraSrc::FPDLINK_CAMERA);
    mGstCameraCapture = new GstCameraCapture();
    mGstCameraRecorder = new GstCameraRecorder();
    _configUtils = ConfigUtils::instance();

    FileUtils::checkDirs();
    // fpdlink record and capture
    connect(this, SIGNAL(newCameraCaptureBuffer(CameraBuffer*)), mGstCameraCapture, SLOT(newCameraCaptureBuffer(CameraBuffer*)), Qt::DirectConnection);
    connect(this, SIGNAL(newCameraRecorderBuffer(CameraBuffer*)), mGstCameraRecorder, SLOT(newCameraRecorderBuffer(CameraBuffer*)), Qt::DirectConnection);
    connect(mGstCameraRecorder,SIGNAL(recordStopped()), this, SLOT(recordStopped()));


    qDebug() << __func__;
    mFpdlinkCameraSrc->start();

    qDebug() << Q_FUNC_INFO << "loadState IN";
    loadState();
    qDebug() << Q_FUNC_INFO << "loadState OUT";

    capThread = new CapThread();
//    capThread->width = width;
//    capThread->height = height;
    for(int i =0; i< 4; i++ ){
        capThread->rgbData[i] =  (unsigned char *)malloc(mallocSize * sizeof(char));
    }
    for(int i =0; i< 4; i++ ){
        capThread->fRgbData[i] =  (unsigned char *)malloc(fMallocSize * sizeof(char));
    }
//    capThread->rgbData = (unsigned char *)malloc(mallocSize * sizeof(char));
//    capThread->rgbData2 = (unsigned char *)malloc(mallocSize * sizeof(char));

    connect(
                capThread,
                SIGNAL(captureRgb(uchar*)),
                this,
                SLOT(onCapFrame(uchar*)));
    connect(
                capThread,
                SIGNAL(captureYuv(uchar*,unsigned int)),
                this,
                SLOT(onYuvReceived(uchar*,unsigned int)));

    connect(
                capThread,
                SIGNAL(cameraPosition(int)),
                this,
                SLOT(onCameraPosition(int)));


}

CameraInterface::~CameraInterface()
{

    delete mFpdlinkCameraSrc;
    delete mGstCameraCapture;

}

void CameraInterface::loadState()
{
    qDebug() << Q_FUNC_INFO << "IN";
    // Load current state
    bool cameraRecordState = _configUtils->cameraRecordState();
    bool palChannelState = _configUtils->palChannelState();

    if(cameraRecordState){
        _configUtils->setCameraRecordState(false);
        qDebug() << Q_FUNC_INFO << "recovery: start record";
        hw_camera_start_record();
    }

    if(palChannelState){
        _configUtils->setPalChannelState(false);
        qDebug() << Q_FUNC_INFO << "recovery: set pal channel 1";
        SetPalChannel(1);
    }
    qDebug() << Q_FUNC_INFO << "OUT";
}

void CameraInterface::saveState()
{
    qDebug() << Q_FUNC_INFO << "IN";
    // Save current state
    if(isRecording){
        _configUtils->setCameraRecordState(true);
        hw_camera_stop_record();
        pendingRestart = true;
    }else{
        _configUtils->setCameraRecordState(false);
    }

    if(GetPalChannel()){
        qDebug() << Q_FUNC_INFO << "Get pal channel 1";
        _configUtils->setPalChannelState(true);
        SetPalChannel(0);
    }else{
        qDebug() << Q_FUNC_INFO << "Get pal channel 0";
        _configUtils->setPalChannelState(false);
    }

    if(!pendingRestart){
        emit needRestart();
    }
    qDebug() << Q_FUNC_INFO << "OUT";
}

void CameraInterface::cameraBufferForCameraCapture(CameraBuffer* buf)
{
    qDebug() << __FUNCTION__ << "000000000000000000000000000000000000000";

    if(isCapturing){
        qDebug() << __func__;
        emit newCameraCaptureBuffer(buf);

        curImageName = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-zzz")+".jpg";
        mGstCameraCapture->capture(curImageName);
        //        disconnect(mFpdlinkCameraSrc, SIGNAL(newCameraBuffer(CameraBuffer*)), this, SLOT(cameraBufferForCameraCapture(CameraBuffer*)));

        //        mMutex.lock();
        //        mWaitCond.wakeAll();
        //        mMutex.unlock();
    }
}

void CameraInterface::cameraBufferForCameraRecorder(CameraBuffer *buf)
{
    if(isRecording){
        emit newCameraRecorderBuffer(buf);
    }
}

void CameraInterface::cameraBufferForRemoteVideoPlayer(CameraBuffer *buf)
{
    emit newCameraBuffer(buf);
}

void CameraInterface::cameraCaptured()
{
    qDebug() << __func__;
    disconnect(mGstCameraCapture, SIGNAL(cameraCaptured()), this, SLOT(cameraCaptured()));
    isCapturing = false;
    emit callCaptureSuc();

    qDebug() << __func__ << "Captured";
}

void CameraInterface::recordStopped()
{
    qDebug() << __func__;

    if(pendingRestart){
        qDebug() << Q_FUNC_INFO << "emit need restart";
        pendingRestart = false;
        emit needRestart();

    }
}


/*接收RGB数据预览相机*/
void CameraInterface::onCapFrame(uchar *frame){

//    qDebug()<<" zhg =========onCapFrame======= frame="<< frame<< "\n";
    emit callCapFrame(frame);
}

/*接收yuv数据保存图片*/
void CameraInterface::onYuvReceived(uchar *frame,unsigned int len){
    qDebug()<< " zhg onYuvReceived frame :" << frame << "\n";
    if(isCapturing){
        CameraBuffer *buf = new CameraBuffer();

        buf->buffer = frame;
        buf->size = len;
        qDebug()<<" zhg=========onYuvReceived========" << buf->buffer << buf->size<< "\n";
        //      buf->user_data = mGstCameraRecorder;
        emit cameraBufferForCameraCapture(buf);
        isCapturing = false;
    }

    if(isRecording){
        CameraBuffer *buf = new CameraBuffer();

        buf->buffer = frame;
        buf->size = len;
        qDebug()<<" =========onYuvReceived========" << buf->buffer << buf->size<< "\n";
        //      buf->user_data = mGstCameraRecorder;
        emit newCameraRecorderBuffer(buf);
    }

}

void CameraInterface::onCameraPosition(int postion){
        qDebug() << __func__ <<" postion: " << postion;
        //add by hjy start
        if(postion == 0)//back
        {
            isBackCamera = true;
        }else if(postion == 1)//front
        {
            isBackCamera = false;
        }
        //add by hjy end
        emit callCameraPosition(postion);

}
uchar* CameraInterface::getMallocData(){
    qDebug() << __func__ <<" getMallocData: ";

    return FileUtils::width == 800 ?capThread->rgbData[0]:capThread->fRgbData[0];
}


void CameraInterface::setCurrentPic1(QImage image){
    //    qDebug() << __func__ <<" setCurrentPic1 " << image;
    backupImage = image;

}

QImage CameraInterface::getCurrentPic1(){
    //    qDebug() << __func__ <<" setCurrentPic " << backupImage;
    return backupImage;
}
void CameraInterface::openCamera()
{
    qDebug() << __func__ <<" openCamera ";
    if(capThread->isRunning()){
        return;
    }
    qDebug() << __func__ <<" openCamera position:"<<position;

    if(position == 0){
        capThread -> cameraPath = BcameraDEVPath;
        FileUtils::width=800;
        FileUtils::height=600;

    }else if(position == 1){
        capThread -> cameraPath = FcameraDEVPath;
        FileUtils::width=640;
        FileUtils::height=480;

    }
//    capThread->width = width;
//    capThread->height = height;

    qDebug() << __func__ <<" openCamera width:"<< FileUtils::width << "height "<<FileUtils::height;
    capThread->startCap();

}
void CameraInterface::closeCamera()
{
    qDebug() << __func__ <<" closCamera ";
    capThread->stopCap();
    capThread->quit();
}
void CameraInterface::switchCamera(int position){
    qDebug() << __func__ <<" switchCamera position:"<<position;
    if(capThread->isRunning()){
        //== 0 ? "REAR" : "FRONT"
        capThread->switchCamera(position);
        qDebug()<<"=========switchCamera======== switchCamera end:"<<capThread->isRunning();
    }
}

void CameraInterface::hw_camera_capture()
{
    qDebug() << __func__ <<" 2222222222222222 isCapturing " << isCapturing;
    // important, sleep 300ms to fix image tearing
    QThread::msleep(300);

    if(!isCapturing){
        isCapturing = true;
        QImage capturePic = getCurrentPic1();
        qDebug() << __func__ <<" 33333 paizhao send : " << capturePic;
        //        emit paizhao(capturePic);
        //modify by hjy start
//        if(isBackCamera)//we use pic width to distinguish(qu fen) front or back camera
//        {
//            mGstCameraCapture->width = width + 1;
//            mGstCameraCapture->width = width;
//            mGstCameraCapture->height = height;
//        }else
//        {
//            mGstCameraCapture->width = width;
//            mGstCameraCapture->height = height;
//        }
        //modify by hjy emd
        mGstCameraCapture->create(methodId);
        connect(mGstCameraCapture, SIGNAL(cameraCaptured()), this, SLOT(cameraCaptured()), Qt::DirectConnection);
        //cameraBufferForCameraCapture();
        //        connect(mFpdlinkCameraSrc, SIGNAL(newCameraBuffer(CameraBuffer*)), this, SLOT(cameraBufferForCameraCapture(CameraBuffer*)), Qt::DirectConnection);
    }else{
        qDebug() << __func__ << "ignore, capturing !!!";
        //        return QString();
    }
}

int CameraInterface::hw_camera_start_record()
{
    qDebug() <<" zhg  camera start "<< __func__ <<" isRecording " << isRecording;
    // important, sleep 300ms to fix video tearing
    QThread::msleep(300);

    if(!isRecording){
        isRecording = true;
//        mGstCameraRecorder->width = width;
//        mGstCameraRecorder->height = height;
        mGstCameraRecorder->methodId = methodId;
        mGstCameraRecorder->record();
        connect(mFpdlinkCameraSrc, SIGNAL(newCameraBuffer(CameraBuffer*)), this, SLOT(cameraBufferForCameraRecorder(CameraBuffer*)), Qt::DirectConnection);
    }else {
        qDebug() << __func__ << "ignore, recording !!!";
    }

    return 0;
}

int CameraInterface::hw_camera_stop_record()
{
    qDebug() << __func__ << " isRecording:" << isRecording;
    if(isRecording){
        isRecording = false;
        mGstCameraRecorder->stop();
        disconnect(mFpdlinkCameraSrc, SIGNAL(newCameraBuffer(CameraBuffer*)), this, SLOT(cameraBufferForCameraRecorder(CameraBuffer*)));
    }else{
        qDebug() << __func__ << "ignore, not in recording state !!!";
    }
    return 0;
}

int CameraInterface::SetPalChannel(int value)
{
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << value;
    int current = GetPalChannel();
    if(current == value){
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << current << "==" << value;
        return 0;
    }

    if(value){
        //        setGpio(12, value);
        QThread::sleep(1);
        //        mPalCameraSrc->start();
        //        mGstMJpegDec->start();
        //        mPalGstRtspServer->startServer();
        //        mRemoteVideoPlayer->start();
    }else{
        //        mRemoteVideoPlayer->stop();
        //        mPalGstRtspServer->stopServer();
        //        mGstMJpegDec->stop();
        //        mPalCameraSrc->stop();
        QThread::sleep(1);
        //        setGpio(12, value);
    }

    qDebug() << __FILE__ << __FUNCTION__ << __LINE__;
    return 0;
}

int CameraInterface::GetPalChannel()
{
    return 0;
}

int CameraInterface::SetPalStream(int value)
{
    int last = ConfigUtils::instance()->getPalBitrate();
    qDebug() << Q_FUNC_INFO << "last:" << last << ", new:" << value;
    if(last != value){
        ConfigUtils::instance()->setPalBitrate(value);
        saveState();
    }

    return 0;
}

int CameraInterface::GetPalStream()
{
    return ConfigUtils::instance()->getPalBitrate();
}

int CameraInterface::SetGlassStream(int value)
{
    int last = ConfigUtils::instance()->getFpdlinkBitrate();
    qDebug() << Q_FUNC_INFO << "last:" << last << ", new:" << value;
    if(last != value){
        ConfigUtils::instance()->setFpdlinkBitrate(value);
        saveState();
    }

    return 0;
}

int CameraInterface::GetGlassStream()
{
    return ConfigUtils::instance()->getFpdlinkBitrate();
}

void CameraInterface::powerDownVideo(){
    capThread->setFileType(FcameraPwrEnPath,0);
    capThread->setFileType(BcameraPwrEnPath,0);
}

#include "fileutils.h"
int CameraInterface::test()
{
    FileUtils::checkDiskAvailable();
    FileUtils::removeOldVideo();
    FileUtils::removeOldImage();
    return 0;
}

