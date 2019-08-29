#ifndef LOCATIONDETECT_H
#define LOCATIONDETECT_H

#include "gpsconfigure.h"
#include <QGeoPositionInfoSource>
#include <QDebug>
#include <QObject>
#include <QTimer>
/**
 * @brief The LocationDetect class
 *
 * Nguồn lấy dữ liệu từ GPS (ở đây là geoclue2)
 */
class gpsconfigure;
class GpsDataSend : public QObject
{
    Q_OBJECT
public:
    GpsDataSend(gps_data_t *const gpsData,
                int *const ReturnGpsDataStatus,
                bool *const GpsRunningFlag);

    ~GpsDataSend();

signals:
    // Tin hieu yeu cau cap nhap data Gps moi
    void positionChanged(const QGeoPositionInfo &info);

private slots:

    // Chuong trinh kiem tra tin hieu Gps co duoc thay doi hay khong
    void positionUpdated(const QGeoPositionInfo &info);

    // Chuong trinh cap nhap data Gps thong qua Gps
    void timerRequestGetDataGps();

private:
     gpsconfigure* _gpsconfigure;

     QTimer* _timerRequestGetDataGps;

     QGeoPositionInfo _lastPos;
};


#endif // LOCATIONDETECT_H
