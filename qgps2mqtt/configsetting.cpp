 #include "configsetting.h"
#include <QMutex>
#include <QDebug>
#include "gps.h"

// FIXME: sai cái lol gì đó, chỗ khởi tạo giá trị

ConfigSetting* ConfigSetting::m_instance = nullptr;
/**
 * @brief ConfigSetting::getInstance
 * @return
 *
 * Lấy cái lol biến toàn cục chứa config của hệ thống
 */
ConfigSetting& ConfigSetting::getInstance()
{
    static QMutex mutex;

    if (m_instance == nullptr)
    {
        mutex.lock();

        if (m_instance == nullptr)
        {
            m_instance = new ConfigSetting();
        }

        mutex.unlock();
    }

    return *(m_instance);

}

/**
 * @brief ConfigSetting::ConfigSetting
 * @param parent
 */
ConfigSetting::ConfigSetting(QObject *parent) : QObject(parent)
{
    QSettings sets("vng", "gps2mqtt");
    qDebug() << sets.fileName();

    hostmqtt     = sets.value(QString("option/hostmqtt"), QString("localhost")).toString();
    portmqtt     = sets.value(QString("option/portmqtt"), 1883).toInt();
    topicpub     = sets.value(QString("option/topicpub"), QString("bus/position")).toString();
    sets.sync();
}


