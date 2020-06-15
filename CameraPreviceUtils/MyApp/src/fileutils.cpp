#include "fileutils.h"

#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QException>

#include <sys/statvfs.h>
#define APP_DATADIR "/data/home/user"

#define MIN_AVAILABLE_DISK_SPACE (150 * 1024 * 1024) //100MB
#define CAMERA_FILE_PATH "/home/user/DCIM/Camera"
#define CAMERA_FILE "/home/user/DCIM/Camera/camera_position.ini"
int FileUtils::width = 800;
int FileUtils::height = 600;
FileUtils::FileUtils()
{

}
/*谨慎使用*/
void FileUtils::checkDirs()
{
    qDebug() << __func__ << " zhg ===========checkDirs";
    QDir dir(SAVE_DIR);
    if(!dir.exists()){
        //        bool isMk = dir.mkdir(SAVE_DIR);
        bool isMk = dir.mkpath(SAVE_DIR);

        qDebug() << __func__ << " zhg ===========isMksuc:"<<isMk;
    }
    dir.setPath(SAVE_TMP_DIR);
    qDebug() << __func__ << " zhg ===========dir.exists():"<<dir.exists();

    //    if(dir.exists()){
    //        QString str("rm -rf " + QString(SAVE_TMP_DIR));
    //        qDebug() << __func__ << "cmd:" << str;
    //        QByteArray byteArray = str.toLatin1();
    //        system(byteArray.data());
    //    }
    bool isTmp = dir.mkpath(SAVE_TMP_DIR);
    qDebug() << __func__ << " zhg ===========isTmp:"<<isTmp;

    dir.setPath(SAVE_IMAGE_DIR);
    if(!dir.exists()){
        dir.mkpath(SAVE_IMAGE_DIR);
        //        dir.mkdir(SAVE_IMAGE_DIR);
    }
    dir.setPath(SAVE_VIDEO_DIR);
    if(!dir.exists()){
        bool isVideo = dir.mkpath(SAVE_VIDEO_DIR);
        qDebug() << __func__ << " zhg ===========isVideo:"<<isVideo;
    }
}

bool FileUtils::moveVideo()
{
#if 0
    QDir dir;
    return dir.rename(SAVE_TMP_VIDEO_FILE, QString(SAVE_VIDEO_DIR) + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-zzz")+".mp4");
#else
    QString str("mv " + QString(SAVE_TMP_VIDEO_FILE) + " " + QString(SAVE_VIDEO_DIR) + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-zzz") + ".mp4");
    qDebug() << __func__ << "cmd:" << str;
    QByteArray byteArray = str.toLatin1();
    system(byteArray.data());
#endif
}

bool FileUtils::moveImage()
{
#if 0
    QDir dir;
    return dir.rename(SAVE_TMP_IMAGE_FILE, QString(SAVE_IMAGE_DIR) + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-zzz")+".jpg");
#else
    QString str("mv " + QString(SAVE_TMP_IMAGE_FILE) + " " + QString(SAVE_IMAGE_DIR) + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss-zzz")+".jpg");
    qDebug() << __func__ << "cmd:" << str;
    QByteArray byteArray = str.toLatin1();
    system(byteArray.data());
#endif
}

bool FileUtils::moveImage(QString filename)
{
#if 0
    QDir dir;
    return dir.rename(SAVE_TMP_IMAGE_FILE, QString(SAVE_IMAGE_DIR) + "/" + filename);
#else
    QString str("mv " + QString(SAVE_TMP_IMAGE_FILE) + " " + QString(SAVE_IMAGE_DIR) + "/" + filename);
    qDebug() << __func__ << "cmd:" << str;
    QByteArray byteArray = str.toLatin1();
    system(byteArray.data());
#endif
}

qreal FileUtils::totalDiskSpace(QString path)
{
    struct statvfs buf;

    if (!statvfs(path.toLocal8Bit().data(), &buf)) {
        return buf.f_bsize * buf.f_blocks;
    }
    else {
        return -1;
    }
}

qreal FileUtils::availableDiskSpace(QString path)
{
    struct statvfs buf;

    if (!statvfs(path.toLocal8Bit().data(), &buf)) {
        //       return buf.f_bsize * buf.f_bfree;
        return buf.f_bsize * buf.f_bavail;
    }
    else {
        return -1;
    }
}

bool FileUtils::checkDiskAvailable()
{
    //    qreal totalDiskSpace = FileUtils::totalDiskSpace(APP_DATADIR);
    //    qreal availableDiskSpace = FileUtils::availableDiskSpace(APP_DATADIR);
    //    qDebug()<<Q_FUNC_INFO<<"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    //    qDebug()<<Q_FUNC_INFO<<"totalDiskSpace="<<totalDiskSpace/1024/1024;
    //    qDebug()<<Q_FUNC_INFO<<"availableDiskSpace="<<availableDiskSpace/1024/1024;
    //    qDebug()<<Q_FUNC_INFO<<"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    //    if(availableDiskSpace > MIN_AVAILABLE_DISK_SPACE){
    return true;
    //    }else{
    //        return false;
    //    }
}

bool FileUtils::removeOldVideo()
{
    QDir dir(SAVE_VIDEO_DIR);

    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setSorting(QDir::Time | QDir::Reversed);

    if(dir.count() == 0){
        qDebug() << Q_FUNC_INFO << "Video dir is empty";
        return false;
    }

    QStringList list = dir.entryList();
    qDebug() << Q_FUNC_INFO << list << list.size() << list.length();

    if(list.length() > 0){
        QString filename = list.at(0);
        qDebug() << "filename:" << filename;
        QFile file(QString("%1/%2").arg(SAVE_VIDEO_DIR, filename));
        bool ret = file.remove();
        qDebug() << Q_FUNC_INFO << "remove file:" << ret;
    }
}

bool FileUtils::removeOldImage()
{
    QDir dir(SAVE_IMAGE_DIR);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setSorting(QDir::Time | QDir::Reversed);

    if(dir.count() == 0){
        qDebug() << Q_FUNC_INFO << "Image dir is empty";
        return false;
    }

    QStringList list = dir.entryList();
    qDebug() << Q_FUNC_INFO << list << list.size() << list.length();
    if(list.length() > 0){
        QString filename = list.at(0);
        qDebug() << "filename:" << filename;
        QFile file(QString("%1/%2").arg(SAVE_IMAGE_DIR, filename));
        bool ret = file.remove();
        qDebug() << Q_FUNC_INFO << "remove file:" << ret;
    }
}
int FileUtils::getCameraPostion(){
    QDir dir(CAMERA_FILE_PATH);
    int position = -1;
    qDebug() << Q_FUNC_INFO << "---zhg sss---------------fileR read-------------" << position;
    if(!dir.exists()){
        bool isMk = dir.mkpath(SAVE_DIR);
        qDebug() << __func__ << " zhg =zhg sss==========getCameraPostion:"<<isMk;
        if(!isMk){
            return position;
        }
    }
    QFile file(CAMERA_FILE);
    bool isOK = file.open(QIODevice::ReadWrite);
    if(isOK)
    {
        //           file.seek(0);
        position = file.readAll().toInt();
        qDebug() << Q_FUNC_INFO << "---zhg sss---------------fileR read-------------" << position;
        file.close();

    }
    return position;

}
bool FileUtils::setCameraPostion(int position){
    qDebug() << Q_FUNC_INFO << "--zhg sss------ position-"<<position;

    QDir dir(CAMERA_FILE_PATH);
    bool status;
    if(!dir.exists()){
        bool isMk = dir.mkpath(SAVE_DIR);
        qDebug() << __func__ << " zhg ===zhg sss========getCameraPostion:"<<isMk;
        if(!isMk){
            return status;
        }
    }
    QFile file(CAMERA_FILE);
    bool isOK = file.open(QIODevice::ReadWrite);
    if(isOK)
    {
        try{
            QTextStream stream(&file);
            stream.reset();
            stream << position;
            stream.flush();
            file.flush();
            file.close();
            qDebug() << "zhg sss write shangdian success \n";
        }catch(QException e){
            qDebug() << "zhg sss write shangdian failure \n" ;
        }
        status = true;
        qDebug() << Q_FUNC_INFO << "--zhg sss----------------file read-------------";
    }
    return status;
}
