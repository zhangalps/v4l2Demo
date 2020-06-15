#ifndef CONFIGUTILS_H
#define CONFIGUTILS_H

#define DEFAULT_BITRATE 1

#define DEFAULT_BITRATE_VALUE_1 "500000"
#define DEFAULT_BITRATE_VALUE_2 "800000"
#define DEFAULT_BITRATE_VALUE_3 "1000000"
#define DEFAULT_BITRATE_VALUE_4 "1200000"
#define DEFAULT_BITRATE_VALUE_5 "1500000"
#define DEFAULT_BITRATE_VALUE   DEFAULT_BITRATE_VALUE_1

#include <QString>

class ConfigUtils
{
public:
    static ConfigUtils* instance();
    void setPalBitrate(int);
    void setFpdlinkBitrate(int);
    int getPalBitrate();
    int getFpdlinkBitrate();
    void setPalChannelState(bool);
    bool palChannelState();
    void setCameraRecordState(bool);
    bool cameraRecordState();
    static QString num2bitrate(int);

private:
    ConfigUtils();

    void load();
    void save();

    static ConfigUtils *_configUtils;

    int mPalBitrate;
    int mFpdlinkBitrate;
    bool mPalChannelState;
    bool mCameraRecordState;
};

#endif // CONFIGUTILS_H
