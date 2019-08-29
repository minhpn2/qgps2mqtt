#ifndef GPSCONFIGURE_H
#define GPSCONFIGURE_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QDebug>
#include <QProcess>
#include <QDir>

#include "gps.h"
#include "wiringPi.h"
#include "math.h"

#define raspberryPiGpsStart 31   // Pin 31 of Raspberry Pi
#define raspberryPiGpsReset 30   // Pin 30 of Raspbeery Pi
#define delayTimeStartMS      10   // 10 ms
#define delayTimeResetMS      10   // 10 ms
#define NUMBER_LOOP_WAIT_CREATED_UC20_COMMAND INT_MAX //2147483647
#define pathUc20Command "/dev/ttyUSB_UC20_COMMAND"
#define pathUc20Data    "/dev/ttyUSB_UC20_DATA"
#define SET_TIME_GETGPS 50 // 0.05s cap nhap mot lan
class gpsconfigure : public QObject
{
    Q_OBJECT
public:
    explicit gpsconfigure(gps_data_t *const gpsData,
                          int *const returnGpsDataStatus,
                          bool *const gpsRunningFlag);

    ~gpsconfigure();

    // Chuong trinh khoi dong GPS module
    void StartGpsModule();

    // Chuong trinh khoi dong lai GPS module
    void ResetGpsModule(const unsigned int &TimeResetGps);

    // Chuong trinh tat GPS module
    void StopGpsModule();

    // Chuong trinh khoi tao tin hieu GPS
    void InitializeGpsSignal();

    // Ket noi port data cua gps voi gpsd
    bool ConnectGpsPort(bool OpenGpsPortData = false);

    // Chuong trinh main chay gps
    void StartGpsSignal();

    // Ket noi cac signal va slot can thiet
    void ConnectFunction();

signals:
    // Tin hieu dung de goi chuong trinh khoi dong lai khi co bat cu issue nao xay ra voi Gps
    void callInitializeGpsSignalWhenIssueHappened();

    // Tin hieu dung de goi chuong trinh positionChanged()
    void getDataGpsUpdated(const QGeoPositionInfo &info);

public slots:
    // Chuong trinh nay dung de goi lay du lieu tu ben ngoai nhu MQTT, hay gi do, tuy ...
    void getDataGpsChanged();

private slots:
    void InitializeGpsSignalWhenIssueHappened();

private:
    gps_data_t *const getGpsData;

    int *const getReturnGpsDataStatus;

    bool *const getGpsRunningFlag;

private:
    enum returnGpsDataStatus
    {
        HAVE_DATA_GPS = 0,
        NO_DATA_GPS = 1,
        NO_SIGNAL_GPS = 2,
        GPS_MODULE_RESETED = 3,
        HAVE_PROBLEM = 4,
    };

    friend class GpsDataSend;
};

#endif // GPSCONFIGURE_H
