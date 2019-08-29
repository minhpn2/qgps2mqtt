#ifndef MAINMODEL_H
#define MAINMODEL_H

#include "configsetting.h"
#include "locationdetect.h"
#include "mqttsender.h"
#include <QObject>
#include <QTimer>
#include <QThread>

/**
 * @brief The mainModel class
 *
 * Logic chung của ứng dụng
 */
class mainModel : public QObject
{
    Q_OBJECT
public:
    explicit mainModel(QObject *parent = nullptr);
    void start();

signals:

public slots:
    void updatePosition(const QGeoPositionInfo &info);

private:
    GpsDataSend* m_gld;

    MqttSender*  m_sender;
};

#endif // MAINMODEL_H
