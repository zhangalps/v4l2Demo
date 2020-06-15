#include "capthread.h"
#include <QDebug>
#include <QException>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include "fileutils.h"

//const QString REAE_CAMERA = "后置相机";
//const QString FRONT_CAMERA = "前置相机";
#define FcameraPwrEnPath "/sys/misc-config/fcamera_pwr_en"
#define BcameraPwrEnPath "/sys/misc-config/bcamera_pwr_en"
CapThread::CapThread(QObject *parent) :
    QThread(parent)
{
    running = false;
}
CapThread::~CapThread()
{
    vd = NULL;
}

void CapThread::run(){

    qDebug()<<Q_FUNC_INFO << " zhg  CapThread run ---------------------- ";
    int rs = -1;
    try{
        rs = initCamera();
        qDebug()<<Q_FUNC_INFO << " zhg  initCamera rs ---------------------- "<< rs << "\n";
    }catch(QException ce){
        qDebug()<<Q_FUNC_INFO << " zhg catch Excep ce: ---------------------- "<< "\n";
    }

    //    timer = new QTimer(this);
    //    connect(timer,SIGNAL(timeout()),this,SLOT(onUpdate()));
    //    timer->start(50);
}
/* 初始化设备*/
int CapThread::initCamera(){
    qDebug()<< "zhg sssinitCamera  start init cameraPath:" << cameraPath << " width "<< FileUtils::width
            <<" height "<<FileUtils::height;
    isSwitch = false;
    if(!getVideoFileExist("video*")){
        setFileType(BcameraPwrEnPath,1);
        if(!isVideoSuc("video0")){
            emit cameraPosition(-1);
        }
        setFileType(FcameraPwrEnPath,1);
        if(!isVideoSuc("video1")){
            emit cameraPosition(-1);
        }


    }else{
        msleep(500);
    }
    vd  = new VideoDevice(cameraPath);
//    vd->width = width;
//    vd->height = height;
    qDebug()<< " zhg initCamera  :" << cameraPath;
    connect(vd, SIGNAL(display_error(QString)), this,SLOT(onCapError(QString)));


    int rs = vd->open_device();
    if(-1==rs)
    {
        qDebug()<< " zhg open_device error ";
        vd->close_device();
        emit cameraPosition(-1);
        return rs;
    }
    vd->getdevice_support();


    rs = vd->init_device();
    if(-1==rs)
    {
        qDebug()<< " zhg init_device error ";
        vd->close_device();
        return rs;
    }

    rs = vd->start_capturing();
    if(-1==rs)
    {
        qDebug()<< " zhg start_capturing error ";
        vd->close_device();
        qDebug()<< " zhg start_capturing error ";
        vd->stop_capturing();
        return rs;
    }
    while(running){
        // todo ..
        onUpdate();
//        msleep(30);
    }

    qDebug()<<Q_FUNC_INFO << " zhg start switching... \n";

    if(vd){
        disconnect(vd, SIGNAL(display_error(QString)), this,SLOT(onCapError(QString)));
        //        vd->unget_frame();
        qDebug()<<Q_FUNC_INFO << " zhg start switching...disconnect suc \n";
        vd->stop_capturing();
        qDebug()<<Q_FUNC_INFO << " zhg start switching...stop_capturing suc \n";
        vd->uninit_device();
        qDebug()<<Q_FUNC_INFO << " zhg start switching...uninit_device suc \n";
        vd-> close_device();
        vd = NULL;
    }
    qDebug()<<Q_FUNC_INFO << " zhg  initCamera end ---------------------- isSwitch:" << isSwitch;
    if(isSwitch){
        running = true;
        initCamera();
    }
    return 0;

}
/*切换摄像头*/
void CapThread::switchCamera(int position){

    qDebug()<< Q_FUNC_INFO << " switchCamera current position:" << position <<" \n";
    if(position == 0){
        cameraPath = "/dev/video1";
        FileUtils::width = 640;
        FileUtils::height = 480;
        emit cameraPosition(1);
    }else{
        cameraPath = "/dev/video0";
        FileUtils::width = 800;
        FileUtils::height = 600;
        emit cameraPosition(0);
    }
    qDebug()<< Q_FUNC_INFO << " switchCamera current position1111111111:" << cameraPath<<" \n" ;
    isSwitch = true;
    running = false;
}
/*停止获取图像*/
void CapThread::stopCap(){
    qDebug()<< Q_FUNC_INFO << " vd:"<< vd;
    running = false;
}

/*开始获取图像*/
void CapThread::startCap(){
    qDebug()<< Q_FUNC_INFO << " isRunning():"<< isRunning();
    if(isRunning()){
        quit();
    }
    running = true;
    start();

}

void CapThread::onCapError(QString error){
    qDebug()<<Q_FUNC_INFO << " zhg  capError  ----------------------error: " << error;

}
/*清除getframe*/
void CapThread::onClearFrame(){
    qDebug()<<Q_FUNC_INFO << " zhg  unget_frame  ----------------------: " <<vd<<"\n";
    //    if(vd){
    //        vd->unget_frame();
    //    }

}
/*开始执行*/
int CapThread::exec(){
    qDebug()<<Q_FUNC_INFO << " zhg  CapThread exec ---------------------- ";
    return 0;
}
/*获取数据显示*/
void CapThread::onUpdate(){
    qDebug()<<Q_FUNC_INFO << " zhg update ";


    if(running){
        int res = vd->get_frame((void **)(&yuvData), (size_t*)(&len));
        emit captureYuv(yuvData,len);

        qDebug()<<Q_FUNC_INFO << " zhg update  res :" << res << " get_frame: "<< yuvData <<" len:"<< len << endl ;
        //    rgbData = (uchar *)malloc(640*480*3);
        int startTime = QTime::currentTime().msec();
//        if(index % 2 == 0){
            convert_yuv_to_rgb_buffer(yuvData,FileUtils::width == 800 ? rgbData[index%4]:fRgbData[index%4]
                    ,FileUtils::width,FileUtils::height);/*QWidget::width(),QWidget::height()*/
//        }else{
//            convert_yuv_to_rgb_buffer(yuvData,rgbData2,width,height);/*QWidget::width(),QWidget::height()*/
//        }
        int endTime = QTime::currentTime().msec();
        qDebug()<<Q_FUNC_INFO << " zhg update  convert_yuv_to_rgb_buffer pp:" << rgbData << " yuv->rgb time:"<< (endTime-startTime);


        if(running){
            emit captureRgb(FileUtils::width == 800 ? rgbData[index%4]:fRgbData[index%4]);
        }
        index++;

        if(vd){
            vd->unget_frame();
        }
    }
}

static inline unsigned char sat(int i) {
    return (unsigned char)( i >= 255 ? 255 : (i < 0 ? 0 : i));
}

#define YUYV2RGB_2(pyuv, prgb) { \
    float r = 1.402f * ((pyuv)[3]-128); \
    float g = -0.34414f * ((pyuv)[1]-128) - 0.71414f * ((pyuv)[3]-128); \
    float b = 1.772f * ((pyuv)[1]-128); \
    (prgb)[0] = sat(pyuv[0] + r); \
    (prgb)[1] = sat(pyuv[0] + g); \
    (prgb)[2] = sat(pyuv[0] + b); \
    (prgb)[3] = sat(pyuv[2] + r); \
    (prgb)[4] = sat(pyuv[2] + g); \
    (prgb)[5] = sat(pyuv[2] + b); \
    }
#define IYUYV2RGB_2(pyuv, prgb) { \
    int r = (22987 * ((pyuv)[3] - 128)) >> 14; \
    int g = (-5636 * ((pyuv)[1] - 128) - 11698 * ((pyuv)[3] - 128)) >> 14; \
    int b = (29049 * ((pyuv)[1] - 128)) >> 14; \
    (prgb)[0] = sat(*(pyuv) + r); \
    (prgb)[1] = sat(*(pyuv) + g); \
    (prgb)[2] = sat(*(pyuv) + b); \
    (prgb)[3] = sat((pyuv)[2] + r); \
    (prgb)[4] = sat((pyuv)[2] + g); \
    (prgb)[5] = sat((pyuv)[2] + b); \
    }
#define IYUYV2RGB_16(pyuv, prgb) IYUYV2RGB_8(pyuv, prgb); IYUYV2RGB_8(pyuv + 16, prgb + 24);
#define IYUYV2RGB_8(pyuv, prgb) IYUYV2RGB_4(pyuv, prgb); IYUYV2RGB_4(pyuv + 8, prgb + 12);
#define IYUYV2RGB_4(pyuv, prgb) IYUYV2RGB_2(pyuv, prgb); IYUYV2RGB_2(pyuv + 4, prgb + 6);

/*yuv格式转换为rgb格式*/
int CapThread::convert_yuv_to_rgb_buffer(unsigned char *yuv, unsigned char *rgb, unsigned int width, unsigned int height)
{
    qDebug()<<Q_FUNC_INFO << " zhg convert_yuv_to_rgb_buffer " << endl;
#if 1
    uint8_t *pyuv = yuv;
    uint8_t *prgb = rgb;
    uint8_t *prgb_end = prgb + width * height * 3;

    while (prgb < prgb_end) {
        IYUYV2RGB_16(pyuv, prgb);

        prgb += 3 * 8 * 2;
        pyuv += 2 * 8 * 2;
    }
#else
    unsigned int in, out = 0;
    unsigned int pixel_16;
    unsigned char pixel_24[3];
    unsigned int pixel32;
    int y0, u, y1, v;
    for(in = 0; in < width * height * 2; in += 4) {
        pixel_16 =
                yuv[in + 3] << 24 |
                               yuv[in + 2] << 16 |
                                              yuv[in + 1] <<  8 |
                                                              yuv[in + 0];
        y0 = (pixel_16 & 0x000000ff);
        u  = (pixel_16 & 0x0000ff00) >>  8;
        y1 = (pixel_16 & 0x00ff0000) >> 16;
        v  = (pixel_16 & 0xff000000) >> 24;
        pixel32 = convert_yuv_to_rgb_pixel(y0, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
        pixel32 = convert_yuv_to_rgb_pixel(y1, u, v);
        pixel_24[0] = (pixel32 & 0x000000ff);
        pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
        pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
        rgb[out++] = pixel_24[0];
        rgb[out++] = pixel_24[1];
        rgb[out++] = pixel_24[2];
    }
#endif
    return 0;
}

int CapThread::convert_yuv_to_rgb_pixel(int y, int u, int v)
{
    unsigned int pixel32 = 0;
    unsigned char *pixel = (unsigned char *)&pixel32;
    int r, g, b;
    r = y + (1.370705 * (v-128));
    g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
    b = y + (1.732446 * (u-128));
    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;
    if(r < 0) r = 0;
    if(g < 0) g = 0;
    if(b < 0) b = 0;
    pixel[0] = r * 220 / 256;
    pixel[1] = g * 220 / 256;
    pixel[2] = b * 220 / 256;
    return pixel32;
}
/*yuv格式转换为rgb格式*/

bool CapThread::isVideoSuc(QString postion){

    int count = 0;
    int endSDTime =0;
    int startSDTime = QTime::currentTime().msec();
    qDebug()<<Q_FUNC_INFO << " start: postion " + postion + " ";
    while(count < 30){
        if(getVideoFileExist(postion)){
            endSDTime = QTime::currentTime().msec();
            qDebug()<<Q_FUNC_INFO << " end:true postion " + postion + " time:"+ (endSDTime - startSDTime);
            return true;
        }
        count++;
        msleep(50);
    }
     endSDTime = QTime::currentTime().msec();
    qDebug()<<Q_FUNC_INFO << " end:false postion " + postion + " time:"+ (endSDTime - startSDTime);

    return false;
}
bool CapThread::getVideoFileExist(QString filterStr)
{
    //"video*"
    qDebug()<< Q_FUNC_INFO <<" :::::start";
    QDir dir(tr("/dev"));
    QStringList nameFilter;
    nameFilter << filterStr;
    QStringList localVideoFiles = dir.entryList(nameFilter , QDir::System);

    qDebug()<< Q_FUNC_INFO << " ::::end localVideoFiles : " << localVideoFiles;

    return localVideoFiles.size() > 0;

}

bool CapThread::setFileType(QString filePath, int type)
{
    QFile file(filePath);
    bool isOK = file.open(QIODevice::ReadWrite | QIODevice::Truncate);
    if(isOK)
    {
        try{
            QTextStream stream(&file);
            stream.reset();
            stream << type;
            stream.flush();
            file.flush();
            file.close();
            qDebug() << "zhg write shangdian success \n";
        }catch(QException e){
            qDebug() << "zhg write shangdian failure \n" ;
        }

    }
    qDebug() << "hjy setFileType filePath == " << filePath << " type == " << type << " isOK == " << isOK;
    return isOK;
}

