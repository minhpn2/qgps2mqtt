#include "mqttsenderprivate.h"

MqttSenderPrivate::MqttSenderPrivate(const QString& host, int port)
{
    this->mqttHost = host;
    this->mqttPort = port;
}

MqttSenderPrivate::~MqttSenderPrivate()
{
    // Do nothing
}

void MqttSenderPrivate::setDefaultTopic(const QString& topic)
{
    this->mqttDefaultTopic = topic;
}

int MqttSenderPrivate::connectToBroker()
{
    qDebug() << "connect to host: " << mqttHost << ", port: " << mqttPort;

    mqttClient.setPort(this->mqttPort);
    mqttClient.setHostname(this->mqttHost);

    connect(&mqttClient,&QMqttClient::connected,this,&MqttSenderPrivate::brokerConnected);
    connect(&mqttClient,&QMqttClient::disconnected,this,&MqttSenderPrivate::brokerDisconnected);
    connect(&mqttClient,&QMqttClient::messageReceived,this,&MqttSenderPrivate::receiverMessage);
    connect(&mqttClient,&QMqttClient::pingResponseReceived,this,&MqttSenderPrivate::pingResponse);

    mqttClient.connectToHost();

    return 0;
}

// operation
int MqttSenderPrivate::publish(void*data, int size)
{
    if (size <=0)
        return -1;

    QByteArray qdata((char*) data, size);
    return mqttClient.publish(mqttDefaultTopic, qdata);
}

void MqttSenderPrivate::clientPublish(QString topic, QString msg)
{
    mqttClient.publish(topic,  msg.toUtf8());
}

void MqttSenderPrivate::clientSubscribe()
{

}

void MqttSenderPrivate::clientPing()
{
    mqttClient.requestPing();
}

void MqttSenderPrivate::brokerDisconnected()
{
    // try reconnect
    mqttClient.connectToHost();
}

void MqttSenderPrivate::brokerConnected()
{
    // try subscribe
    qDebug() << "connected to host: " << mqttHost << ",port: " << mqttPort;

}

void MqttSenderPrivate::pingResponse()
{
    // just debug
    qDebug() << "pong received";
}

void MqttSenderPrivate::receiverMessage(const QByteArray &message, const QMqttTopicName &topic)
{
    qDebug() << "message "<< message << "topic " << topic.name();

}

