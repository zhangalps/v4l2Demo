#include "gstcameracapture.h"

#include "fileutils.h"
#include <QDebug>
#include <QThread>

GstCameraCapture::GstCameraCapture(QObject *parent) : QObject(parent)
{
    gst_init (NULL, NULL);
    memset(&mData, 0, sizeof(mData));
}

void GstCameraCapture::create(int methodId)
{
    mData.pipeline = gst_pipeline_new("camera-capture");
    mData.appsrc = gst_element_factory_make("appsrc", "appsrc");
    mData.encoder = gst_element_factory_make("jpegenc", "encoder");
    mData.filesink = gst_element_factory_make("filesink", "filesink");
    mData.videoFlip = gst_element_factory_make("videoflip", "video_flip");
//    mData.videoconvert = gst_element_factory_make ("videoconvert", "convert1");

    mData.taginject = gst_element_factory_make("taginject", NULL);
    mData.mutex = new QMutex();
    mData.cond = new QWaitCondition();

    if(!mData.pipeline || !mData.appsrc /*|| !mData.videoconvert*/ || !mData.encoder || !mData.filesink){
        g_printerr("Some elements could not be created");
    }

    GstCaps *caps = gst_caps_new_simple("video/x-raw-yuv",
            "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'),
            "width", G_TYPE_INT, FileUtils::width,
            "height", G_TYPE_INT, FileUtils::height,
            "framerate", GST_TYPE_FRACTION, 20, 1,
            NULL);
    //"format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'),

    if (mData.videoFlip) {
        qDebug()<< " zhg videoFlip camera methodId666666:" << (FileUtils::width == 800?2:0) << " width "<<FileUtils::width <<" height "<< FileUtils::height;
        //methodId 2 is Back ,0 is Front
        g_object_set(mData.videoFlip, "method", FileUtils::width == 800?2:0, nullptr);
//        g_object_set(mData.pipeline->get(), "video-filter", mData.videoFlip, nullptr);
    }else{
        qDebug()<< " =============zhg videoFlip is null ===========";

    }
    g_object_set(mData.taginject, "tags", "EXIF LensModel=\"13MP\"", NULL);
    g_object_set(G_OBJECT(mData.appsrc), "blocksize", FileUtils::width * FileUtils::height *2, NULL);
    g_object_set(G_OBJECT(mData.appsrc), "num-buffers", 1, NULL);
    g_object_set(G_OBJECT(mData.appsrc), "caps", caps, NULL);
    g_object_set(G_OBJECT(mData.filesink), "location", SAVE_TMP_IMAGE_FILE, NULL);

    /* we add a message handler */
    mData.bus = gst_pipeline_get_bus (GST_PIPELINE (mData.pipeline));
    gst_object_unref (mData.bus);
    gst_bus_add_signal_watch (mData.bus);

    gst_bin_add_many (GST_BIN (mData.pipeline), mData.appsrc, mData.videoFlip/*,mData.videoconvert*/,mData.taginject, mData.encoder, mData.filesink, NULL);
    g_signal_connect (mData.appsrc, "need-data", G_CALLBACK (cb_need_data), &mData);
    g_signal_connect (mData.bus, "message", G_CALLBACK (cb_message), this);

    if(!gst_element_link(mData.appsrc, mData.videoFlip)){
        g_warning("failed to link appsrc and videoFlip");
    }
//    if(!gst_element_link(mData.videoFlip, mData.videoconvert)){
//        g_warning("failed to link videoFlip and videoconvert");
//    }
    if(!gst_element_link(mData.videoFlip, mData.taginject)){
        g_warning("failed to link videoFlip and taginject");
    }
    if(!gst_element_link(mData.taginject, mData.encoder)){
        g_warning("failed to link taginject and encoder");
    }

    if(!gst_element_link(mData.encoder, mData.filesink)){
        g_warning("failed to link encoder and filesink");
    }
}

GstCameraCapture::~GstCameraCapture()
{
}

void GstCameraCapture::capture(QString imageName)
{
    qDebug() << __func__<< " zhg start videoflip =====";
    curImageName = imageName;
    gst_element_set_state (mData.pipeline, GST_STATE_PLAYING);
    gst_element_set_state (mData.videoFlip, GST_STATE_PLAYING);

}

void GstCameraCapture::newCameraCaptureBuffer(CameraBuffer *buf)
{
    qDebug() << __func__ << "data 3333333333333333";
    mData.mutex->lock();

    if(mData.cameraBuffer != NULL){
        if(!mData.bufBusy){
            mData.cameraBuffer->unref();
            mData.cameraBuffer = buf;

            mData.cameraBuffer->ref();
            mData.cond->wakeOne();
        }
    }else{
        mData.cameraBuffer = buf;
        mData.cameraBuffer->ref();
        mData.cond->wakeOne();
    }

    mData.mutex->unlock();
    qDebug() << __func__ << "OUT";
}

void GstCameraCapture::gst_buffer_free(gpointer user_data)
{
    g_print("%s, %d\n", __FUNCTION__, __LINE__);
    qDebug() << __func__ << "threadid:" << QThread::currentThreadId();

    UserData *data = (UserData*)user_data;
    data->mutex->lock();
    data->cameraBuffer->unref();
    data->cameraBuffer = NULL;
    data->bufBusy = false;
    data->mutex->unlock();
}

void GstCameraCapture::cb_need_data(GstElement *element, guint unused_size, gpointer user_data)
{
    g_print("%s,%d, size:%d\n", __FUNCTION__, __LINE__, unused_size);

    GstBuffer *buf = NULL;
    GstFlowReturn ret;
    UserData *data = (UserData*)user_data;

    data->mutex->lock();

//    g_print("%s,%d, size:%d\n", __FUNCTION__, __LINE__, unused_size);
    if(data->cameraBuffer == NULL){
        data->cond->wait(data->mutex);
        if(data->cameraBuffer == NULL){
            g_print("%s,%d, size:%d\n", __FUNCTION__, __LINE__, unused_size);
            data->mutex->unlock();
            return;
        }
    }

    buf = gst_buffer_new();
    GST_BUFFER_FREE_FUNC(buf) = gst_buffer_free;
    gst_buffer_set_data(buf, (guint8*)data->cameraBuffer->buffer, unused_size);
    GST_BUFFER_MALLOCDATA(buf) = (guint8*)data;

    data->bufBusy = true;
    data->mutex->unlock();

    g_signal_emit_by_name(data->appsrc, "push_buffer", buf, &ret);
    gst_buffer_unref(buf);

    if(ret != GST_FLOW_OK){
        g_print("gst flow error.");
    }
}

void GstCameraCapture::cb_message (GstBus *bus, GstMessage *msg, gpointer user_data)
{
    qDebug() << "cb_message";
    GstCameraCapture *gstCameraCapture = (GstCameraCapture*)user_data;
    switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *debug;

        gst_message_parse_error (msg, &err, &debug);
        g_print ("Error: %s\n", err->message);
        g_error_free (err);
        g_free (debug);

        gst_element_set_state (gstCameraCapture->mData.pipeline, GST_STATE_READY);
        gst_element_set_state (gstCameraCapture->mData.videoFlip, GST_STATE_READY);
        // TODO no space
        emit gstCameraCapture->cameraCaptured();
        break;
    }
    case GST_MESSAGE_EOS:
        qDebug()<<"EOS !!!!!!";
        /* end-of-stream */
        gst_element_set_state (gstCameraCapture->mData.pipeline, GST_STATE_NULL);
        gst_element_set_state (gstCameraCapture->mData.videoFlip, GST_STATE_NULL);
        FileUtils::moveImage(gstCameraCapture->curImageName);
        emit gstCameraCapture->cameraCaptured();
        break;
    case GST_MESSAGE_BUFFERING: {
        gint percent = 0;

        gst_message_parse_buffering (msg, &percent);
        g_print ("Buffering (%3d%%)\r", percent);
        break;
    }
    case GST_MESSAGE_CLOCK_LOST:
        /* Get a new clock */
        gst_element_set_state (gstCameraCapture->mData.pipeline, GST_STATE_PAUSED);
        gst_element_set_state (gstCameraCapture->mData.pipeline, GST_STATE_PLAYING);
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

