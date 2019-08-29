#include "mqttsender.h"

MqttSender::MqttSender(const QString& host, int port)
{
    _privateSender = new MqttSenderPrivate(host, port);

    if (_privateSender)
        _privateSender->connectToBroker();
}

MqttSender::~MqttSender()
{
    delete _privateSender;
}

void MqttSender::setDefaultTopic(const QString& topic)
{
    QString qTopic = topic;
    if (_privateSender) _privateSender->setDefaultTopic(qTopic);
}

int MqttSender::sendUtf8String(const char* utf8str)
{
    if (_privateSender)
        return _privateSender->publish((void*) utf8str, sizeof (utf8str));
    return -1;
}

int MqttSender::send(void*data, int size)
{
    if (_privateSender)
    {
        return _privateSender->publish(data, size);
    }

    return -1;
}

int MqttSender::asyncSend(void* data, int size)
{
    if (_privateSender)
    {
        return _privateSender->publish(data, size);
    }

    return -1;
}
