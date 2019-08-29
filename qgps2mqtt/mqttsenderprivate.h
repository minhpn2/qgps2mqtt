#ifndef MQTTSENDERPRIVATE_H
#define MQTTSENDERPRIVATE_H

#include <qmqttclient.h>
#include <QGeoPositionInfoSource>
#include <QObject>
#include <QString>

class MqttSenderPrivate : public QObject
{
    Q_OBJECT
public:
    explicit MqttSenderPrivate(const QString& host, int port);

    ~MqttSenderPrivate();

    void setDefaultTopic(const QString& topic);

    int connectToBroker();

    int publish(void*data, int size);

    void clientPublish(QString topic, QString msg);

    void clientSubscribe();

    void clientPing();

private slots:
    void brokerDisconnected();

    void brokerConnected();

    void pingResponse();

    void receiverMessage(const QByteArray &message, const QMqttTopicName &topic);

private:
    QMqttClient mqttClient;
    QString     mqttHost;
    QString     mqttDefaultTopic;
    int         mqttPort;

    friend class MqttSender;
};
#endif // MQTTSENDERPRIVATE_H
