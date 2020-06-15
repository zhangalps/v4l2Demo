#include "videodevice.h"
#include <unistd.h>
#include <QDebug>
#include "fileutils.h"

#define BCAMERA_PATH "/dev/video0"
#define FCAMERA_PATH "/dev/video1"

VideoDevice::VideoDevice(QString dev_name)
{
    //    QString temp_dev_name = "/dev/video0";
    this->dev_name = dev_name;
    this->fd = -1;
    this->buffers = NULL;
    this->n_buffers = 0;
    this->index = -1;
    //display_error(tr("dev_name"));


}

int VideoDevice::open_device()
{
    fd = open(dev_name.toStdString().c_str(), O_RDWR/*|O_NONBLOCK*/, 0);
    // fd = open(dev_name.toStdString().c_str(), O_RDWR|O_NONBLOCK, 0);
    // QString tmp;
    //display_error(tmp.setNum(fd));
    qDebug()<<Q_FUNC_INFO << " open fd :"<<fd << " :::::::::dev_name: "+ dev_name;

    if(-1 == fd)
    {
        emit display_error(tr("open: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}

int VideoDevice::close_device()
{
    if(close(fd) == -1)
    {
        emit display_error(tr("close: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}
int VideoDevice::init_device()
{
    v4l2_capability cap;
    v4l2_cropcap cropcap;
    v4l2_crop crop;
    v4l2_format fmt;
    v4l2_streamparm s_parm;


//    v4l2_control ctrl;

    qDebug()<<Q_FUNC_INFO << " VIDIOC_S_CTRL  zhg start adjust camera:";


    if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    {
        qDebug()<<Q_FUNC_INFO << " capabilities  :"<<cap.capabilities << " V4L2_CAP_VIDEO_CAPTURE:"<<V4L2_CAP_VIDEO_CAPTURE;

        emit display_error(tr("%1 is no video capture device").arg(dev_name));
        return -1;
    }

    if(!(cap.capabilities & V4L2_CAP_STREAMING))
    {
        qDebug()<<Q_FUNC_INFO << " V4L2_CAP_STREAMING capabilities  :"<<cap.capabilities << " V4L2_CAP_STREAMING:"<<V4L2_CAP_STREAMING;

        emit display_error(tr("%1 does not support streaming i/o").arg(dev_name));
        return -1;
    }

    CLEAR(cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    //if(0)
    if(0 == ioctl(fd, VIDIOC_CROPCAP, &cropcap))
    {
        CLEAR(crop);
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;
        //        qDebug()<<Q_FUNC_INFO << " VIDIOC_CROPCAP2222222222222 crop.type:" << V4L2_BUF_TYPE_VIDEO_CAPTURE<< " crop.c:"<< crop.c
        //               << " defrect :" << cropcap.defrect;

        //        if(-1 == ioctl(fd, VIDIOC_S_CROP, &crop))
        //        {
        //            qDebug()<<Q_FUNC_INFO << " VIDIOC_CROPCAP2222222222222 EINVAL:"<<EINVAL << " errno:"<<QString(strerror(errno));

        //            if(EINVAL == errno)
        //            {
        ////                emit display_error(tr("VIDIOC_S_CROP not supported"));
        //            }
        //            else
        //            {
        //                emit display_error(tr("VIDIOC_S_CROP: %1").arg(QString(strerror(errno))));
        //                return -1;
        //            }
        //        }
    }
    else
    {
        qDebug()<<Q_FUNC_INFO << " VIDIOC_CROPCAP 22222222222222:"<< " errno:"<<QString(strerror(errno));

        emit display_error(tr("VIDIOC_CROPCAP: %1").arg(QString(strerror(errno))));
        return -1;
    }



    //set ftps
    //__u32   numerator; fenzi
    // __u32   denominator; fenmu
    CLEAR(s_parm);
    s_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    s_parm.parm.capture.timeperframe.numerator = 1;
    s_parm.parm.capture.timeperframe.denominator = 20;
    if(-1 == ioctl(fd, VIDIOC_S_PARM, &s_parm)){
        qDebug()<<Q_FUNC_INFO << " zhg@@@@ VIDIOC_S_PARM ioctl fail======:";
        emit display_error(tr(" zhg@@@@VIDIOC_S_PARM").arg(QString(strerror(errno))));
    }else{
        qDebug()<<Q_FUNC_INFO << " zhg@@@@VIDIOC_S_PARM ioctl suc:";
    }

    /* crop.c.height = 640; //采集视频的高度
    crop.c.width = 480; //宽度
    crop.c.left = 0; //左偏移
    crop.c.top = 0;
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    */

    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = FileUtils::width;
    fmt.fmt.pix.height = FileUtils::height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if(-1 == ioctl(fd, VIDIOC_S_FMT, &fmt))
    {
        qDebug()<<Q_FUNC_INFO << " VIDIOC_S_FMT 22222222222222:"<< " errno:"<<QString(strerror(errno));

        emit display_error(tr("VIDIOC_S_FMT11").arg(QString(strerror(errno))));
        return -1;
    }

    if(-1 == init_mmap())
    {
        return -1;
    }

    return 0;
}

int VideoDevice::init_mmap()
{
    v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if(-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        if(EINVAL == errno)
        {
            emit display_error(tr("%1 does not support memory mapping").arg(dev_name));
            return -1;
        }
        else
        {
            emit display_error(tr("VIDIOC_REQBUFS %1").arg(QString(strerror(errno))));
            return -1;
        }
    }
    qDebug()<< Q_FUNC_INFO << " zhg req.count:"<< req.count;
    if(req.count < 2)
    {
        emit display_error(tr("Insufficient buffer memory on %1").arg(dev_name));
        return -1;
    }

    buffers = (buffer*)calloc(req.count, sizeof(*buffers));

    if(!buffers)
    {
        emit display_error(tr("out of memory"));
        return -1;
    }

    for(n_buffers = 0; n_buffers < req.count; ++n_buffers)
    {
        v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            emit display_error(tr("VIDIOC_QUERYBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
                mmap(NULL, // start anywhere
                     buf.length,
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED,
                     fd, buf.m.offset);

        if(MAP_FAILED == buffers[n_buffers].start)
        {
            emit display_error(tr("mmap %1").arg(QString(strerror(errno))));
            return -1;
        }
    }
    return 0;

}

int VideoDevice::start_capturing()
{
    unsigned int i;
    for(i = 0; i < n_buffers; ++i)
    {
        v4l2_buffer buf;
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory =V4L2_MEMORY_MMAP;
        buf.index = i;
        //        fprintf(stderr, "n_buffers: %d\n", i);

        if(-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            emit display_error(tr("VIDIOC_QBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }

    v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    qDebug() << Q_FUNC_INFO;

    if(-1 == ioctl(fd, VIDIOC_STREAMON, &type))
    {
        emit display_error(tr("VIDIOC_STREAMON: %1").arg(QString(strerror(errno))));
        return -1;
    }

    qDebug() << Q_FUNC_INFO << "stream on ...";
    return 0;
}

int VideoDevice::stop_capturing()
{
    v4l2_buf_type type;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type))
    {
        emit display_error(tr("VIDIOC_STREAMOFF: %1").arg(QString(strerror(errno))));
        return -1;
    }
    return 0;
}

int VideoDevice::uninit_device()
{
    unsigned int i;
    for(i = 0; i < n_buffers; ++i)
    {
        if(-1 == munmap(buffers[i].start, buffers[i].length))
        {
            emit display_error(tr("munmap: %1").arg(QString(strerror(errno))));
            return -1;
        }

    }
    free(buffers);
    return 0;
}

int VideoDevice::getdevice_support(){
    qDebug()<<Q_FUNC_INFO << " zhg@@@@current camera support:begin";
    v4l2_std_id std;
    int ret;
    do{
        ret = ioctl(fd,VIDIOC_QUERYSTD,&std);
        qDebug()<<Q_FUNC_INFO << " zhg@@@@do while ret:"<<ret;

    }while(ret == -1 && errno == EAGAIN);
    switch(std){
    case V4L2_STD_NTSC:
        qDebug()<<Q_FUNC_INFO << " zhg@@@@current camera support:V4L2_STD_NTSC";
        break;
    case V4L2_STD_PAL:
        qDebug()<<Q_FUNC_INFO << " zhg@@@@current camera support:V4L2_STD_PAL";
        break;
    }
    return ret;

}

int VideoDevice::get_frame(void **frame_buf, size_t* len)
{
    v4l2_buffer queue_buf;
    CLEAR(queue_buf);

    qDebug()<<Q_FUNC_INFO << " zhg queue_buf :"<< queue_buf.length <<"\n";
    queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    queue_buf.memory = V4L2_MEMORY_MMAP;

    if(-1 == ioctl(fd, VIDIOC_DQBUF, &queue_buf))
    {
        switch(errno)
        {
        case EAGAIN:
            //            perror("dqbuf");
            return -1;
        case EIO:
            return -1 ;
        default:
            emit display_error(tr("VIDIOC_DQBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
    }
    qDebug()<<Q_FUNC_INFO << " zhg ioctl end queue_buf :"<< queue_buf.length <<"\n";


    *frame_buf = buffers[queue_buf.index].start;
    *len = buffers[queue_buf.index].length;
    index = queue_buf.index;

    return 0;

}

int VideoDevice::unget_frame()
{
    if(index != -1)
    {
        v4l2_buffer queue_buf;
        CLEAR(queue_buf);

        queue_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        queue_buf.memory = V4L2_MEMORY_MMAP;
        queue_buf.index = index;

        if(-1 == ioctl(fd, VIDIOC_QBUF, &queue_buf))
        {
            emit display_error(tr("VIDIOC_QBUF: %1").arg(QString(strerror(errno))));
            return -1;
        }
        return 0;
    }
    return -1;
}


