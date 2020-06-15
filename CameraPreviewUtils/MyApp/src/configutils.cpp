#include "configutils.h"

#define CONFIG_FILE "./config.json"
#define CONFIG_KEY_PAL  "pal"
#define CONFIG_KEY_FPDLINK  "fpdlink"
#define CONFIG_KEY_CAMERA_RECORD_STATE "camera-record-state"
#define CONFIG_KEY_PAL_CHANNEL_STATE "pal-channel-state"

#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

ConfigUtils* ConfigUtils::_configUtils = NULL;

ConfigUtils::ConfigUtils():mFpdlinkBitrate(DEFAULT_BITRATE), mPalBitrate(DEFAULT_BITRATE)
{

}

ConfigUtils *ConfigUtils::instance()
{
    if(_configUtils == NULL){
        _configUtils = new ConfigUtils();
        _configUtils->load();
    }
    return _configUtils;
}

void ConfigUtils::setFpdlinkBitrate(int bitrate)
{
    if(mFpdlinkBitrate != bitrate){
        mFpdlinkBitrate = bitrate;
        save();
    }
}

void ConfigUtils::setPalBitrate(int bitrate)
{
    if(mPalBitrate != bitrate){
        mPalBitrate = bitrate;
        save();
    }
}

int ConfigUtils::getFpdlinkBitrate()
{
    qDebug() << Q_FUNC_INFO << "Fpdlink bitrate:" << mFpdlinkBitrate;
    return mFpdlinkBitrate;
}

int ConfigUtils::getPalBitrate()
{
    qDebug() << Q_FUNC_INFO << "Pal bitrate:" << mPalBitrate;
    return mPalBitrate;
}

void ConfigUtils::setPalChannelState(bool state)
{
    if(mPalChannelState != state){
        mPalChannelState = state;
        save();
    }
}

bool ConfigUtils::palChannelState()
{
    return mPalChannelState;
}

void ConfigUtils::setCameraRecordState(bool state)
{
    if(mCameraRecordState != state){
        mCameraRecordState = state;
        save();
    }
}

bool ConfigUtils::cameraRecordState()
{
    return mCameraRecordState;
}

void ConfigUtils::load()
{
    QFile file(CONFIG_FILE);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << Q_FUNC_INFO << "Config file open failed!";
        mFpdlinkBitrate = DEFAULT_BITRATE;
        mPalBitrate = DEFAULT_BITRATE;
        return;
    }

    QByteArray array = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(array, &jsonError);
    if(jsonError.error != QJsonParseError::NoError){
        qDebug() << Q_FUNC_INFO << QString("JsonParseError: %1").arg(jsonError.errorString());
        mPalBitrate = DEFAULT_BITRATE;
        mFpdlinkBitrate = DEFAULT_BITRATE;
        return;
    }

    QJsonObject rootObject = doc.object();

    if(!rootObject.keys().contains(CONFIG_KEY_PAL))
    {
        qDebug() << "No target value:" << CONFIG_KEY_PAL;
        mPalBitrate = DEFAULT_BITRATE;
    }else{
        mPalBitrate = rootObject.value(CONFIG_KEY_PAL).toInt();
    }

    if(!rootObject.keys().contains(CONFIG_KEY_FPDLINK))
    {
        qDebug() << "No target value:" << CONFIG_KEY_FPDLINK;
        mFpdlinkBitrate = DEFAULT_BITRATE;
    }else{
        mFpdlinkBitrate = rootObject.value(CONFIG_KEY_FPDLINK).toInt();
    }

    if(!rootObject.keys().contains(CONFIG_KEY_CAMERA_RECORD_STATE))
    {
        qDebug() << "No target value:" << CONFIG_KEY_CAMERA_RECORD_STATE;
        mCameraRecordState = false;
    }else{
        mCameraRecordState = rootObject.value(CONFIG_KEY_CAMERA_RECORD_STATE).toBool();
    }

    if(!rootObject.keys().contains(CONFIG_KEY_PAL_CHANNEL_STATE))
    {
        qDebug() << "No target value:" << CONFIG_KEY_PAL_CHANNEL_STATE;
        mPalChannelState = false;
    }else{
        mPalChannelState = rootObject.value(CONFIG_KEY_PAL_CHANNEL_STATE).toBool();
    }
}

void ConfigUtils::save()
{
    QJsonObject rootObject;
    QJsonDocument jsonDocument;

    rootObject.insert(CONFIG_KEY_PAL, mPalBitrate);
    rootObject.insert(CONFIG_KEY_FPDLINK, mFpdlinkBitrate);
    rootObject.insert(CONFIG_KEY_CAMERA_RECORD_STATE, mCameraRecordState);
    rootObject.insert(CONFIG_KEY_PAL_CHANNEL_STATE, mPalChannelState);

    jsonDocument.setObject(rootObject);
    QByteArray byteArray = jsonDocument.toJson(QJsonDocument::Indented);

    QFile file(CONFIG_FILE);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << QString("fail to open the file: %1, %2, %3").arg(__FILE__).arg(__LINE__).arg(__FUNCTION__);
        return;
    }
    QTextStream out(&file);
    out << byteArray;

    file.close();

    qDebug() << byteArray;
}

QString ConfigUtils::num2bitrate(int value)
{
    std::string bitrate = DEFAULT_BITRATE_VALUE;
    switch (value) {
    case 1:
        bitrate = DEFAULT_BITRATE_VALUE_1;
        break;
    case 2:
        bitrate = DEFAULT_BITRATE_VALUE_2;
        break;
    case 3:
        bitrate = DEFAULT_BITRATE_VALUE_3;
        break;
    case 4:
        bitrate = DEFAULT_BITRATE_VALUE_4;
        break;
    case 5:
        bitrate = DEFAULT_BITRATE_VALUE_5;
        break;
    default:
        bitrate = DEFAULT_BITRATE_VALUE;
        break;
    }
    qDebug() << Q_FUNC_INFO << "bitrate:" << QString().fromStdString(bitrate);
    return QString().fromStdString(bitrate);
}
