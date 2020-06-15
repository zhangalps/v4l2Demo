
#include "gstcamerarecorder.h"

#include <QDebug>

#include <QThread>

#include "fileutils.h"

#define USE_MPEG4 1 // for test
#define ENABLE_VIDEORATE 0 // Some bugs
#define NUM_BUFFERS (50 * 60 * 5) // 50fps * 5min

//gst-launch-0.10 --gst-debug=omx:4,2 -e v4l2src device=/dev/video1 ! ffmpegcolorspace ! "video/x-raw-yuv,format=(fourcc)I420,framerate=(fraction)30/1" ! videoparse format="i420" width=480 height=320 framerate=30/1 ! omx_h264enc ! h264parse ! qtmux ! filesink location=/home/user/test.mp4

GstCameraRecorder::GstCameraRecorder(QObject *parent) : QObject(parent)
{
    gst_init (NULL, NULL);
    memset(&mData, 0, sizeof(UserData));
    isCreated = false;

    mMutex = new QMutex();
    mCond = new QWaitCondition();
}

void GstCameraRecorder::create(int methodId)
{
    mData.pipeline = gst_pipeline_new("camera-capture");
    mData.appsrc = gst_element_factory_make("appsrc", "appsrc");
    mData.videorate = gst_element_factory_make("videorate", "videorate");
    mData.videoconvert = gst_element_factory_make("ffmpegcolorspace", "videoconvert");
    mData.capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    mData.videoparse = gst_element_factory_make("videoparse", "videoparse");
    mData.videoFlip = gst_element_factory_make("videoflip", "video_flip");

#if USE_MPEG4
    mData.encoder = gst_element_factory_make("ffenc_mpeg4", "encoder");
#else
    mData.encoder = gst_element_factory_make("omx_h264enc", "encoder");
#endif
    mData.h264parse = gst_element_factory_make("h264parse", "h264parse");
    mData.mux = gst_element_factory_make("qtmux", "mux");
    mData.filesink = gst_element_factory_make("filesink", "filesink");

    if(!mData.pipeline || !mData.appsrc || !mData.videorate || !mData.videoconvert || !mData.capsfilter
            || !mData.videoparse || !mData.encoder || !mData.h264parse || !mData.mux || !mData.filesink){

        g_printerr("Some elements could not be created\n");
        return;
    }

    GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
                                        "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'),
                                        "width", G_TYPE_INT, FileUtils::width,
                                        "height", G_TYPE_INT, FileUtils::height,
                                        "framerate", GST_TYPE_FRACTION, 20, 1,
                                        NULL);
#if ENABLE_VIDEORATE
    GstCaps *capsfilter_caps = gst_caps_new_simple("video/x-raw-yuv",
                                                   "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
                                                   "width", G_TYPE_INT, FileUtils::width,
                                                   "height", G_TYPE_INT, FileUtils::height,
                                                   "framerate", GST_TYPE_FRACTION, 25, 1,
                                                   NULL);
#else
    GstCaps *capsfilter_caps = gst_caps_new_simple("video/x-raw-yuv",
                                                   "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
                                                   "width", G_TYPE_INT, FileUtils::width,
                                                   "height", G_TYPE_INT, FileUtils::height,
                                                   "framerate", GST_TYPE_FRACTION, 20, 1,
                                                   NULL);
#endif

    if (mData.videoFlip) {
        qDebug()<< " zhg videoFlip camera methodId" << (FileUtils::width == 800?2:0);
        //methodId 2 is Back ,0 is Front
        g_object_set(mData.videoFlip, "method", FileUtils::width == 800?2:0, nullptr);
//        g_object_set(mData.pipeline->get(), "video-filter", mData.videoFlip, nullptr);
    }else{
        qDebug()<< " =============zhg videoFlip is null ===========";

    }
    g_object_set(G_OBJECT(mData.appsrc), "blocksize", FileUtils::width*FileUtils::height*2, NULL);
    g_object_set(G_OBJECT(mData.appsrc), "num-buffers", NUM_BUFFERS, NULL);
    g_object_set(G_OBJECT(mData.appsrc), "do-timestamp", 1, NULL);
    g_object_set(G_OBJECT(mData.appsrc), "caps", caps, NULL);

    g_object_set(G_OBJECT(mData.capsfilter), "caps", capsfilter_caps, NULL);
    g_object_set(G_OBJECT(mData.videoparse), "format", 1, NULL);
    g_object_set(G_OBJECT(mData.videoparse), "width", FileUtils::width, NULL);
    g_object_set(G_OBJECT(mData.videoparse), "height", FileUtils::height, NULL);
    g_object_set(G_OBJECT(mData.videoparse), "interlaced", 1, NULL);
#if ENABLE_VIDEORATE
    g_object_set(G_OBJECT(mData.videoparse), "framerate", 25, 1, NULL);
#else
//    g_object_set(G_OBJECT(mData.videoparse), "framerate", 50, 1, NULL);
      g_object_set(G_OBJECT(mData.videoparse), "framerate", 20, 1, NULL);

#endif
    g_object_set(G_OBJECT(mData.encoder), "bitrate", 3000000, NULL);

    g_object_set(G_OBJECT(mData.filesink), "location", SAVE_TMP_VIDEO_FILE, NULL);

    gst_caps_unref(caps);
    gst_caps_unref(capsfilter_caps);


    /* we add a message handler */
    mData.bus = gst_pipeline_get_bus (GST_PIPELINE (mData.pipeline));
    gst_object_unref (mData.bus);
    gst_bus_add_signal_watch (mData.bus);

    gst_bin_add_many (GST_BIN (mData.pipeline), mData.appsrc,mData.videoFlip, mData.videorate, mData.videoconvert, mData.capsfilter,
                      mData.videoparse, mData.encoder, mData.h264parse, mData.mux, mData.filesink, NULL);
    g_signal_connect (mData.appsrc, "need-data", G_CALLBACK (cb_need_data), this);
    g_signal_connect (mData.bus, "message", G_CALLBACK (cb_message), this);
#if ENABLE_VIDEORATE
    if(!gst_element_link(mData.appsrc, mData.videorate)){
        g_warning("failed to link appsrc and videorate");
    }

    if(!gst_element_link(mData.videorate, mData.videoconvert)){
        g_warning("failed to link videorate and videoconvert");
    }
#else
    if(!gst_element_link(mData.appsrc, mData.videoFlip)){
        g_warning("failed to link appsrc and videoFlip");
    }
    if(!gst_element_link(mData.videoFlip, mData.videoconvert)){
        g_warning("failed to link appsrc and videoconvert");
    }
#endif

    if(!gst_element_link(mData.videoconvert, mData.capsfilter)){
        g_warning("failed to link videoconvert and capsfilter");
    }

    if(!gst_element_link(mData.capsfilter, mData.videoparse)){
        g_warning("failed to link capsfilter and videoparse");
    }

    if(!gst_element_link(mData.videoparse, mData.encoder)){
        g_warning("failed to link videoparse and encoder");
    }

#if USE_MPEG4
    if(!gst_element_link(mData.encoder, mData.mux)){
        g_warning("failed to link encoder and mux");
    }
#else
    if(!gst_element_link(mData.encoder, mData.h264parse)){
        g_warning("failed to link encoder and h264parse");
    }

    if(!gst_element_link(mData.h264parse, mData.mux)){
        g_warning("failed to link h264parse and mux");
    }
#endif
    if(!gst_element_link(mData.mux, mData.filesink)){
        g_warning("failed to link mux and filesink");
    }
    isCreated = true;
}

// Temp solution for resolve omx framework issue
void GstCameraRecorder::destroy()
{
    qDebug() << __func__;

    // set pipeline state to null
    gst_element_set_state(mData.pipeline, GST_STATE_NULL);
    gst_element_set_state(mData.videoFlip, GST_STATE_NULL);


    gst_object_unref(mData.pipeline);
    gst_object_unref(mData.videoFlip);


    mMutex->lock();
    if(mData.cameraBuffer){
        qDebug() << Q_FUNC_INFO << "chenxin:------------------release last buffer";
        mData.cameraBuffer->unref();
        mData.cameraBuffer = NULL;
        mData.bufBusy = false;
    }
    memset(&mData, 0, sizeof(UserData));
    mMutex->unlock();

    isCreated = false;
}

GstCameraRecorder::~GstCameraRecorder()
{

}

void GstCameraRecorder::record()
{
    qDebug() << __func__;

    while(!FileUtils::checkDiskAvailable()){
        if(!FileUtils::removeOldVideo() && !FileUtils::removeOldImage()){
            qDebug() << "Chenxin(record): Not enough space, but no video or image file found!!!!!!!!";
            return;
        }
    }

    mData.userStopped = false;
#if 1
    if(isCreated){
        destroy();
    }
    create(methodId);
#endif
    qDebug() << __func__ << "set pipeline to playing";
    gst_element_set_state (mData.pipeline, GST_STATE_PLAYING);
    gst_element_set_state (mData.videoFlip, GST_STATE_PLAYING);


    mMutex->lock();
    mData.isEOS = false;
    mMutex->unlock();
}

void GstCameraRecorder::internalRecord()
{
    qDebug() << __func__;

    while(!FileUtils::checkDiskAvailable()){
        if(!FileUtils::removeOldVideo() && !FileUtils::removeOldImage()){
            qDebug() << "Chenxin(record): Not enough space, but no video or image file found!!!!!!!!";
            return;
        }
    }

#if 1
    if(isCreated){
        destroy();
    }
    qDebug() << __func__ << "Continue record";
    create(methodId);
#endif
    qDebug() << __func__ << "set pipeline to playing";
    gst_element_set_state (mData.pipeline, GST_STATE_PLAYING);
    gst_element_set_state (mData.videoFlip, GST_STATE_PLAYING);



    mMutex->lock();
    mData.isEOS = false;
    mMutex->unlock();
}

void GstCameraRecorder::stop()
{
    qDebug() << __func__;
    qDebug() << __func__ << "threadid:" << QThread::currentThreadId();

    GstFlowReturn ret = GST_FLOW_OK;
    mMutex->lock();
    if(!mData.isEOS){
        g_signal_emit_by_name(mData.appsrc, "end-of-stream", &ret);
        mCond->wakeOne();// wake up the last need_data
    }
    mMutex->unlock();

    mData.userStopped = true;
}

void GstCameraRecorder::newCameraRecorderBuffer(CameraBuffer *buf)
{
    mMutex->lock();

    if(mData.isEOS){
        mMutex->unlock();
        qDebug() << Q_FUNC_INFO << "======= EOS, return";
        return;
    }

    if(mData.cameraBuffer != NULL){
        if(!mData.bufBusy){
            mData.cameraBuffer->unref();
            mData.cameraBuffer = buf;
            mData.cameraBuffer->ref();
            mCond->wakeOne();
        }
    }else{
        mData.cameraBuffer = buf;
        mData.cameraBuffer->ref();
        mCond->wakeOne();
    }

    qDebug() << Q_FUNC_INFO << "mData.cameraBuffer:" << mData.cameraBuffer;

    mMutex->unlock();
}

void GstCameraRecorder::gst_buffer_free(gpointer user_data)
{
    g_print("%s, %d\n", __FUNCTION__, __LINE__);
    qDebug() << __func__ << "threadid:" << QThread::currentThreadId();

    qDebug() << Q_FUNC_INFO << "user_data:" << user_data;

    GstCameraRecorder *gstCameraRecorder = (GstCameraRecorder*)user_data;
    UserData *data = &gstCameraRecorder->mData;
    gstCameraRecorder->mMutex->lock();

    if(data->cameraBuffer){
        qDebug() << __func__ << "camera:" << data->cameraBuffer;
        qDebug() << __func__ << "buffer:" << data->cameraBuffer->buffer;
        data->cameraBuffer->unref();
        data->cameraBuffer = NULL;
    }else{
        qDebug() << Q_FUNC_INFO << "chenxin: cameraBuffer is null";
    }
    data->bufBusy = false;
    gstCameraRecorder->mMutex->unlock();
}
#include <QDateTime>
void GstCameraRecorder::cb_need_data(GstElement *element, guint unused_size, gpointer user_data)
{
    qDebug() << __func__ << QDateTime::currentDateTime().toMSecsSinceEpoch();
    g_print("%s,%d, size:%d\n", __FUNCTION__, __LINE__, unused_size);
    qDebug() << __func__ << "threadid:" << QThread::currentThreadId();

    GstBuffer *buf = NULL;
    GstFlowReturn ret;
    GstCameraRecorder *gstCameraRecorder = (GstCameraRecorder*)user_data;
    UserData *data = &gstCameraRecorder->mData;

    qDebug() << __func__ << "   cb_need_data    1111 lock__________";
    gstCameraRecorder->mMutex->lock();
    if(data->userStopped){
        g_print("Found eos return\n");
        gstCameraRecorder->mMutex->unlock();
        return;
    }

//    g_print("%s,%d, size:%d\n", __FUNCTION__, __LINE__, unused_size);
    if(data->cameraBuffer == NULL){
        gstCameraRecorder->mCond->wait(gstCameraRecorder->mMutex);

        if(data->cameraBuffer == NULL){
            g_print("%s,%d, size:%d\n", __FUNCTION__, __LINE__, unused_size);
            gstCameraRecorder->mMutex->unlock();
            return;
        }
    }

    qDebug() << __func__ << "2222_        unlock__________";

    buf = gst_buffer_new();
    GST_BUFFER_FREE_FUNC(buf) = gst_buffer_free;

    qDebug() << Q_FUNC_INFO << "data.cameraBuffer->buffer::" << data->cameraBuffer;
    gst_buffer_set_data(buf, (guint8*)data->cameraBuffer->buffer, unused_size);
    GST_BUFFER_MALLOCDATA(buf) = (guint8*)user_data;

    data->bufBusy = true;
    gstCameraRecorder->mMutex->unlock();

    g_signal_emit_by_name(data->appsrc, "push-buffer", buf, &ret);
    gst_buffer_unref(buf);

    data->bufferCounter++;
    qDebug() << Q_FUNC_INFO << "-------------------------------------------buf:" << data->bufferCounter;

    if(ret != GST_FLOW_OK){
        g_print("gst flow error.");
    }

    qDebug() << __func__ << "999";
}

void GstCameraRecorder::cb_message (GstBus *bus, GstMessage *msg, gpointer user_data)
{
    qDebug() << "gst camera recorder cb_message msg:" << GST_MESSAGE_TYPE (msg);
    GstCameraRecorder *gstCameraRecorder = (GstCameraRecorder*)user_data;
    GstStateChangeReturn ret1;
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {

        GError *err;
        gchar *debug;

        gst_message_parse_error (msg, &err, &debug);
        qDebug() << "cb_message GST_MESSAGE_ERROR11111111****************:" << err->message;
//        g_print ("Error: %s\n", err->message);
        g_error_free (err);
        g_free (debug);

        gst_element_set_state (gstCameraRecorder->mData.pipeline, GST_STATE_READY);
        gst_element_set_state (gstCameraRecorder->mData.videoFlip, GST_STATE_READY);
        break;
    }
    case GST_MESSAGE_EOS:
        qDebug()<<"EOS !!!!!!";
        /* end-of-stream */

        gstCameraRecorder->mMutex->lock();
        gstCameraRecorder->mData.isEOS = true;
        gstCameraRecorder->mMutex->unlock();

        gstCameraRecorder->mData.bufferCounter = 0;
        // set pipeline state to NULL
        ret1 = gst_element_set_state (gstCameraRecorder->mData.pipeline, GST_STATE_NULL);
         gst_element_set_state (gstCameraRecorder->mData.videoFlip, GST_STATE_NULL);

        switch (ret1) {
        case GST_STATE_CHANGE_FAILURE:
            g_print ("---GST_STATE_CHANGE_FAILURE\n");
            break;
        case GST_STATE_CHANGE_SUCCESS:
            g_print ("---GST_STATE_CHANGE_SUCCESS\n");
            break;
        case GST_STATE_CHANGE_NO_PREROLL:
            /* for live sources, we need to set the pipeline to PLAYING before we can
               * receive a buffer. We don't do that yet */
            g_print ("---GST_STATE_CHANGE_NO_PREROLL\n");
            break;
        case GST_STATE_CHANGE_ASYNC:
            g_print ("---GST_STATE_CHANGE_ASYNC\n");
            break;
        default:
            g_print("----------!!!!");
            break;
        }
        FileUtils::moveVideo();

        if(!gstCameraRecorder->mData.userStopped){
            qDebug() << __func__ << "record next video";
            gstCameraRecorder->internalRecord();
        }else{
            qDebug() << __func__ << "User stopped";
            emit gstCameraRecorder->recordStopped();
        }

        break;
    case GST_MESSAGE_BUFFERING: {
        gint percent = 0;

        gst_message_parse_buffering (msg, &percent);
        g_print ("Buffering (%3d%%)\r", percent);
        break;
    }
    case GST_MESSAGE_CLOCK_LOST:
        /* Get a new clock */
        gst_element_set_state (gstCameraRecorder->mData.pipeline, GST_STATE_PAUSED);
        gst_element_set_state (gstCameraRecorder->mData.pipeline, GST_STATE_PLAYING);
        break;
    case GST_MESSAGE_STATE_CHANGED:
        GstState stateOld;
        GstState stateNew;

        gst_message_parse_state_changed(msg, &stateOld, &stateNew, NULL);
        qDebug() << "STATE CHANGE: "
                 << "NAME: " << QString(GST_OBJECT_NAME (msg->src))
                 << " Old State: "  << QString(gst_element_state_get_name (stateOld))
                 << " New State: "  << QString(gst_element_state_get_name (stateNew));
        break;
    default:
        /* Unhandled message */
        break;
    }
}


