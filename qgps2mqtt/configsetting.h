#ifndef CONFIGSETTING_H
#define CONFIGSETTING_H

#include <QObject>
#include <QSettings>
#include "gps.h"
/**
 * @brief The ConfigSetting class
 */
class ConfigSetting : public QObject
{
    Q_OBJECT
public:
    explicit ConfigSetting(QObject *parent = nullptr);
    static ConfigSetting& getInstance();

    const QString& getMqttHost()
    {
        return hostmqtt;
    }

    int getMqttPort()
    {
        return portmqtt;
    }

    const QString& getMqttTopic()
    {
        return topicpub;
    }

    gps_data_t* getPointerToGpsData()
    {
        return& gpsData;
    }

    int* getGpsDataStatus()
    {
        return& returnGpsDataStatus;
    }

    bool* getGpsRunningStatus()
    {
        return& gpsRunningFlag;
    }

signals:

public slots:

private:
    static ConfigSetting *m_instance;

    QString hostmqtt;
    int     portmqtt;
    QString topicpub;

    gps_data_t gpsData;
    int returnGpsDataStatus;
    bool gpsRunningFlag;
};

#endif // CONFIGSETTING_H
