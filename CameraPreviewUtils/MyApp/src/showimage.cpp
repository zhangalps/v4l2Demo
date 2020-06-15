#include "showimage.h"
#include <QDebug>
#include "fileutils.h"


ShowImage::ShowImage(QObject *parent) :
    QObject(parent)
{
    qDebug()<<"=========jake ShowImage========"<< "\n";
    provider = new CameraCaptureProvider();
    cameraInterface = new CameraInterface();
    //get camera position file
    int position = FileUtils::getCameraPostion();
    qmlPosition = position < 0 ? 0 : position;
    setCameraPosition(position);

    capImage = QImage(cameraInterface->getMallocData(),FileUtils::width,FileUtils::height,QImage::Format_RGB888);

    connect(
                cameraInterface,
                SIGNAL(callCapFrame(uchar*)),
                this,
                SLOT(onCapFrame(uchar*)));
    connect(
                cameraInterface,
                SIGNAL(callCameraPosition(int)),
                this,
                SLOT(onCameraPosition(int)));
    connect(
                cameraInterface,
                SIGNAL(callCaptureSuc()),
                this,
                SLOT(onCallCaptureSuc()));

}
/*接收RGB数据预览相机*/
void ShowImage::onCapFrame(uchar *frame){

    qDebug()<<" zhg =========onCapFrame======= frame="<< frame<< "\n";

    capImage.fromData((uchar *)frame,mallocSize);
    provider->img = capImage;

    emit callQmlRefeshImg();
}

void ShowImage::onCallCaptureSuc(){
    qDebug()<<Q_FUNC_INFO << " zhg onCallCaptureSuc ==";
    emit callQmlCaptureSuc();

}
/*摄像头前后位置监听*/
void ShowImage::onCameraPosition(int position){
    qDebug()<<Q_FUNC_INFO << " zhg postion =="<< position;
    QString info;
    switch(position){
    case 0:
        info = "后置相机";
        FileUtils::setCameraPostion(0);
        setCameraPosition(0);
        break;
    case 1:
        info = "前置相机";
        FileUtils::setCameraPostion(1);
        setCameraPosition(1);
        break;
    case -1:
        info = "打开失败";
        break;
    default:
        info = "相机异常";
        break;
    }
    if(info == "后置相机"){
        methodId = 2;
    }else if(info == "前置相机"){
        methodId = 0;
    }

    qDebug()<<Q_FUNC_INFO << " zhg postion info=="<< info;
    emit callQmlCameraPosition(info);

}

/*接收yuv数据保存图片*/
void ShowImage::onYuvReceived(uchar *frame,unsigned int len){
    qDebug()<< " zhg onYuvReceived frame :" << frame << "\n";
    if(cameraInterface->isCapturing){
        CameraBuffer *buf = new CameraBuffer();

        buf->buffer = frame;
        buf->size = len;
        qDebug()<<" zhg=========onYuvReceived========" << buf->buffer << buf->size<< "\n";
        //      buf->user_data = mGstCameraRecorder;
        emit cameraInterface->cameraBufferForCameraCapture(buf);
        cameraInterface->isCapturing = false;
    }

    if(cameraInterface->isRecording){
        CameraBuffer *buf = new CameraBuffer();

        buf->buffer = frame;
        buf->size = len;
        qDebug()<<" =========onYuvReceived========" << buf->buffer << buf->size<< "\n";
        //      buf->user_data = mGstCameraRecorder;
        emit cameraInterface->newCameraRecorderBuffer(buf);
    }

}

void ShowImage::setCameraPosition(int position){
    qDebug()<<"====ShowImage===zhg sss==setCameraPosition========::"<<position;
    if(position == 0){
        FileUtils::width = 800;
        FileUtils::height = 600;
//        cameraInterface->width = 800;
//        cameraInterface->height = 600;
    }else if(position == 1){
        FileUtils::width = 640;
        FileUtils::height = 480;
//        cameraInterface->width = 640;
//        cameraInterface->height = 480;
    }
    cameraInterface->position = position;
    capImage = QImage(cameraInterface->getMallocData(),FileUtils::width,FileUtils::height,QImage::Format_RGB888);

}

/*打开相机*/
void ShowImage::openCamera(){
    qDebug()<<"====ShowImage=====openCamera========::";

    cameraInterface->openCamera();

}

void ShowImage::stopCamera(){
    cameraInterface->closeCamera();
}
/*开始拍照*/
void ShowImage::startCameraCapture(){
    qDebug()<<"=========startCameraCapture========";
    cameraInterface->methodId = methodId;
    cameraInterface->hw_camera_capture();
}
/*开始录像*/
void ShowImage::startRecorder(){
    qDebug()<<"=========startRecorder========";
    cameraInterface->methodId = methodId;
    cameraInterface->hw_camera_start_record();
}
/*停止录像*/
void ShowImage::stopRecorder(){
    qDebug()<<"=========stopRecorder======== capThread->isRunning():";
    cameraInterface->hw_camera_stop_record();

}
/*切换摄像头*/
void ShowImage::switchCamera(QString current){
    qDebug()<<"=========switchCamera======== capThread->isRunning():"
           << " current:" <<current;
//    FileUtils::setCameraPostion(current == "后置相机"? 0 : 1);
//    emit callQmlCameraPosition(current == "后置相机"?"前置相机":"后置相机");
    cameraInterface->switchCamera(current == "后置相机"? 0 : 1);
}
/*back*/
void ShowImage::backKey(){
    qDebug()<<Q_FUNC_INFO<<" zhg backkey ";
    cameraInterface->closeCamera();
    cameraInterface->powerDownVideo();
}
/*getCameraPosition*/
int ShowImage::getCameraPosition(){
    qDebug()<<Q_FUNC_INFO<<" zhg getCameraPosition::: " << qmlPosition;
    return qmlPosition;
}

