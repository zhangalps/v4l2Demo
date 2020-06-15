#ifndef FILEUTILS_H
#define FILEUTILS_H

#define SAVE_DIR "/home/user/DCIM/Camera"
#define SAVE_TMP_DIR "/home/user/DCIM/Camera"
//#define SAVE_IMAGE_DIR "/data/home/user/.208/images"/home/user/DCIM/Camera
//#define SAVE_VIDEO_DIR "/data/home/user/.208/videos"
#define SAVE_IMAGE_DIR "/home/user/DCIM/Camera"
#define SAVE_VIDEO_DIR "/home/user/DCIM/Camera"
#define SAVE_TMP_IMAGE_FILE "/home/user/DCIM/Camera/image.jpg"
#define SAVE_TMP_VIDEO_FILE "/home/user/DCIM/Camera/video.mp4"

#include <QString>

class FileUtils
{
public:
    FileUtils();
    static void checkDirs();
    static bool moveVideo();
    static bool moveImage();
    static bool moveImage(QString filename);
    static qreal totalDiskSpace(QString path);
    static qreal availableDiskSpace(QString path);
    static bool checkDiskAvailable();

    static bool removeOldVideo();
    static bool removeOldImage();
    static int getCameraPostion();
    static bool setCameraPostion(int);
    static int width;
    static int height;


};

#endif // FILEUTILS_H
