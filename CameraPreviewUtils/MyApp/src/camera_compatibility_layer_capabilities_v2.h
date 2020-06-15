/*
 * This file is part of libhybris
 *
 * Copyright (C) 2019 Beijing Yuan Xin Technology Co.,Ltd. All rights reserved.
 *
 * Authors:
 *        Chen Xin<chenxin@syberos.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Beijing Yuan Xin Technology Co.,Ltd. All rights are reserved.
 */

#ifndef SYBEROS_CAMERA_COMPATIBILITY_LAYER_CONFIGURATION_V2_H_
#define SYBEROS_CAMERA_COMPATIBILITY_LAYER_CONFIGURATION_V2_H_

#define ALL_CALLBACK_WITH_CONTEXT
//#define FORCE_FOCUS_CALLBACK_WITH_CONTEXT

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum
    {
        BACK_FACING_CAMERA_TYPE,
        FRONT_FACING_CAMERA_TYPE
    }CameraType;

    typedef void (*on_data_raw_image)(void* data, int data_size, void* context);
    typedef void (*on_data_compressed_image)(void* data, int data_size, void* context);

    typedef struct CameraControlListener{
        on_data_raw_image on_data_raw_image_cb;
        on_data_compressed_image on_data_compressed_image_cb;
        void *context;
    }CameraControlListener;

    typedef struct CameraControl{
        void *camera;
        CameraControlListener *listener;
    }CameraControl;

    void android_camera_init();
    CameraControl* android_camera_open(CameraType camera_type, CameraControlListener *listener);
    int android_camera_close(CameraControl *control);
    void android_camera_start_preview(CameraControl *control);
    void android_camera_stop_preview(CameraControl *control);

    int android_camera_release_recording_frame(CameraControl *control, void* data);
#if 0
    int android_camera_init();
    int android_camera_open(int cameraId);
    int android_camera_get_info(int cameraId,void *sInfo);
    int android_camera_get_orientation(int cameraId);
    int android_camera_close();
    int android_camera_take_picture(capture_cb cb);
    int android_camera_cancel_picture();
    int android_camera_post_take_picture();
    //    int android_camera_set_preview_window(struct ANativeWindow* nativeWindow);
    int android_camera_start_preview(update_cb cb, timestamp_cb ts_cb);
    int android_camera_stop_preview();
    int android_camera_set_parameters(char* params);
    char* android_camera_get_parameters();
    int android_camera_get_number_of_devices();
    int android_camera_start_recording(recording_cb cb);
    int android_camera_stop_recording();
    int android_camera_release_recording_frame(char* data);
    int android_camera_auto_focus(status_cb st_cb);
    int android_camera_cancel_auto_focus();
    int android_camera_store_metadata_in_buffers(int flag);

    int android_camera_auto_focus_with_context(status_cb st_cb, void *user);
    int android_camera_start_preview_with_context(update_cb cb, timestamp_cb ts_cb, void *user);
    int android_camera_take_picture_with_context(capture_cb cb, void *user);
    int android_camera_start_recording_with_context(recording_cb cb, void *user);
#endif
#ifdef __cplusplus
}
#endif

#endif // SYBEROS_CAMERA_COMPATIBILITY_LAYER_CONFIGURATION_V2_H_
