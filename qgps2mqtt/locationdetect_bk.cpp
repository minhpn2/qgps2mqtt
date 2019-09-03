#include "locationdetect.h"
/**
 * @brief LocationDetect::LocationDetect
 * @param parent
 */
class Uc20gModuleGps : public QObject
{
    Q_OBJECT
public:
    explicit Uc20gModuleGps(gps_data_t &gpsData,
                   int &returnGpsDataStatus,
                   bool &gpsRunningFlag)
        : getGpsRunningFlag(gpsRunningFlag)
    {
        this->getGpsData = &gpsData;
        this->getReturnGpsDataStatus = &returnGpsDataStatus;
    }

    ~Uc20gModuleGps()
    {

    }

    void StartGpsModule()
    {
        wiringPiSetup(); // Configure GPIO on raspberry pi
        pinMode(raspberryPiGpsStart, OUTPUT); // Dinh nghia chan 31 cua Raspberry: Output

        if (false == getGpsRunningFlag)
        {
            // GPS chi khoi dong khi co xung len tai chan RaspberryPi_GPS_Start
            digitalWrite(raspberryPiGpsStart, LOW);
            delayMicroseconds(10); // Delay 10ms
            digitalWrite(raspberryPiGpsStart, HIGH);
        }
    }

    void ResetGpsModule(const unsigned int &TimeResetGps)
    {
        wiringPiSetup();
        pinMode(raspberryPiGpsReset, OUTPUT);

        if(true == getGpsRunningFlag)
        {
            // Set off-->on--> off chan Reset cua GPS trong mot thoi gian
            digitalWrite(raspberryPiGpsReset, LOW);
            delayMicroseconds(TimeResetGps);
            digitalWrite(raspberryPiGpsReset, HIGH);
            delayMicroseconds(TimeResetGps);
            digitalWrite(raspberryPiGpsReset, LOW);
        }
    }

    void TurnOffGpsModule()
    {
        wiringPiSetup(); // Configure GPIO on raspberry pi
        pinMode(raspberryPiGpsStart, OUTPUT); // Dinh nghia chan 31 cua Raspberry: Output
        // GPS chi khoi dong khi co xung len tai chan RaspberryPi_GPS_Start
        if(true == getGpsRunningFlag)
        {
            digitalWrite(raspberryPiGpsStart, HIGH);
            delayMicroseconds(delayTimeStartMS);
            digitalWrite(raspberryPiGpsStart, LOW);
        }
    }

    void InitializeGpsSignal()
    {
        for (int i = 0; i < NUMBER_LOOP_WAIT_CREATED_UC20_COMMAND; i++)
        {
            if(!QFileInfo(pathUc20Command).exists())
            {
#ifdef DEBUG_LOCATION_GPS
                qDebug() << "Dang cho Gps tren UC20 Module khoi dong... \n";
#endif
                *getReturnGpsDataStatus = HAVE_PROBLEM;

                 getGpsRunningFlag = false;
            }

            else
            {
#ifdef DEBUG_LOCATION_GPS
                qDebug() << "GPS Module Started!!! \n" <<
                            "Initializing Gps Signal... \n";
#endif
                try
                {
                    QProcess sendCommandAT;
                    sendCommandAT.start("sh", QStringList() <<
                                        "-c" <<
                                        "echo \"AT+QGPS=1\" "
                                        " > "
                                        "/dev/ttyUSB_UC20_COMMAND",
                                        QIODevice::WriteOnly);
                    sendCommandAT.waitForStarted();
                    sendCommandAT.execute("gpsd /dev/ttyUSB_UC20_DATA"
                                          " -F "
                                          "/var/run/gpsd.sock");
                    sendCommandAT.close();

                    *getReturnGpsDataStatus = NO_DATA_GPS;
                    // Gps Module da duoc khoi dong xong
                    getGpsRunningFlag = true;

                    // Exit loop
                    break;
                }
                catch(...)
                {
#ifdef DEBUG_LOCATION_GPS
                    qDebug() << "Module Gps da gap su co."
                                "Khong the khoi dong Gps!!! \n";
#endif
                    // Gui thong bao loi den chuong trinh chinh
                    *getReturnGpsDataStatus = HAVE_PROBLEM;
                    getGpsRunningFlag = false;

                    break;
                }
            }
        }
    }

    bool ConnectGpsPort(bool OpenGpsPortData = false)
    {
        try
        {
            if(-1 == gps_open("localhost", "2947", getGpsData))
            {
#ifdef DEBUG_LOCATION_GPS
                qDebug() << "Khong the ket noi duoc voi Gps port data!!!";
#endif

                // Khong the ket noi voi Gps port data
                *getReturnGpsDataStatus = HAVE_PROBLEM;
                getGpsRunningFlag = false;
            }

            else if(true == OpenGpsPortData &&
                    (-1 != gps_open("localhost", "2947", getGpsData)))
            {
                // Doc du lieu tu Gps port
                gps_stream(getGpsData, WATCH_ENABLE | WATCH_JSON, nullptr);

                // Da ket noi duoc voi Gps port
                getGpsRunningFlag = true;
                *getReturnGpsDataStatus = NO_DATA_GPS;

            }

        }
        catch (...)
        {
            // Su dung ham try catch nay de tranh truong hop loi xay ra:
            // - Gps port mat ket noi khi gps_read dang doc.
            *getReturnGpsDataStatus = HAVE_PROBLEM;

            getGpsRunningFlag = false;
            // Thong bao loi da xay ra
        }
        return getGpsRunningFlag;
    }

    void StartGpsSignal()
    {
        // Chuong trinh khoi dong Gps, chuong trinh nay se duoc goi dau tien
        if(false == getGpsRunningFlag)
        {
            StartGpsModule();
            // Configure pin of GPS module
            InitializeGpsSignal();
            // Ket noi voi port du lieu GPS voi GPSD port
            if(true == ConnectGpsPort(getGpsRunningFlag))
            {
                qDebug() << "Connected!!!";
            }
            else
            {
                // Khong the ket noi Gps duoc
                qDebug() << "Cann't connect with GPS!!!";
            }
        }
    }

    void ConnectFunction()
    {
#ifdef DEBUG_LOCATION_GPS
        qDebug() << "Connect Function!!!";
#endif
        connect(this, SIGNAL(callInitializeGpsSignalWhenIssueHappened()),
                this, SLOT(InitializeGpsSignalWhenIssueHappened()));

    }
signals:
    // Tin hieu dung de goi chuong trinh khoi dong lai khi co bat cu issue nao xay ra voi Gps
    void callInitializeGpsSignalWhenIssueHappened();

    // Tin hieu dung de goi chuong trinh positionChanged()
    void getDataGpsUpdated(const QGeoPositionInfo &info);

public slots:
    // Chuong trinh nay dung de goi lay du lieu tu ben ngoai nhu MQTT, hay gi do, tuy ...
    void getDataGpsChanged()
    {
        try
        {
            // Kiem tra trang thai hoat dong cua Gps Module thong qua gpsRunningFlag xem co du lieu moi cap nhap hay khoong thong qua ham gps_waiting(), thoi gian cho la 5s
            if((true == getGpsRunningFlag)
               && (gps_waiting(getGpsData, 5000)))
            {
                if(-1 == gps_read(getGpsData))
                {
#ifdef DEBUG_LOCATION_GPS
                    qDebug() << "Khong the mo port Gps Data duoc!!!";
#endif
                    // Xay ra loi voi port Gps Data
                    *getReturnGpsDataStatus = HAVE_PROBLEM;

                    // Thong bao chuong trinh gap loi
                    emit callInitializeGpsSignalWhenIssueHappened();
                }

                else
                {
                    /*
                     * Kiem tra xem du lieu Gps co bi sai hay khong
                     * - Kiem tra hai du lieu latitude va longitude cos phai la so hay khong
                     * */
                    if ((STATUS_FIX == getGpsData->status)
                         && (!isnan(getGpsData->fix.latitude)
                            |!isnan(getGpsData->fix.longitude)))
                    {
#ifdef DEBUG_LOCATION_GPS
                        qDebug("latitude: %f, longitude: %f, speed: %f, timestamp: %lf\n", gpsData->fix.latitude, gpsData->fix.longitude, gpsData->fix.speed, gpsData->fix.time);
#endif
                        // Thong bao co du lieu moi
                        *getReturnGpsDataStatus = HAVE_DATA_GPS;

                        // Gui thong bao cap nhap du lieu
                        QGeoPositionInfo info;
                        info.setCoordinate(QGeoCoordinate(getGpsData->fix.latitude,
                                                          getGpsData->fix.longitude,
                                                          getGpsData->fix.altitude));
                        emit getDataGpsUpdated(info);
                    }

                    else
                    {
                        // Du lieu tra ve bi loi
                        *getReturnGpsDataStatus = NO_DATA_GPS;
                    }
                }
            }

            else
            {
                /* Truong hop nay xay ra khi Gps khong hoat dong (gpsRunningFlag = false) hoac khong co du lieu Gps duoc day len
                 * Khong thuc hien chuong trinh nao o day, vi co the xay ra truong hop nhay vao goi chuong trinh khoi dong nhieu lan do vuot thoi gian cho cua gps_waiting la 5s.
                 * Khong them bat ki chuong trinh nao vao phan nay
                 */
                return;
            }
        }
        catch (...)
        {
#ifdef DEBUG_LOCATION_GPS
        qDebug() << "Co loi xay ra voi port du lieu Gps!!!";
#endif
        *getReturnGpsDataStatus = HAVE_PROBLEM;
        getGpsRunningFlag = true;
        // Gui yeu cau khoi dong lai Gps Module
        emit callInitializeGpsSignalWhenIssueHappened();
        }
    }

private slots:

    void InitializeGpsSignalWhenIssueHappened()
    {
        try
        {
            // Truong hop xay ra van de khong the khoi dong GPS duoc, chuong trinh nay se ket noi lai tin hieu GPS voi GPSD.
            if((false == getGpsRunningFlag)
                && (HAVE_PROBLEM == *getReturnGpsDataStatus))
            {
                // Chuong trinh duoc yeu cau khoi dong lai tu dau
                StartGpsSignal();
            }

            // Truong hop khoi dong lai sau khi bi reset
            else if((true == getGpsRunningFlag)
                   && (HAVE_PROBLEM == *getReturnGpsDataStatus))
            {
                // Chuong trinh duoc yeu cau khoi dong lai tu dau
                StartGpsSignal();
            }

            else
            {
                // Bo xung neu nhu co truong hop ngoai le xay ra bat ngo
#ifdef DEBUG_LOCATION_GPS
                qDebug() << "Can't connect with Gps after restart GPS module!!!";
#endif
            }
        }
        catch (...)
        {
            // Da xay loi voi Gps Module hoac tren he thong,
            // KHONG THE KHOI DONG LAI DUOC GPS MODULE
            getGpsRunningFlag = false;
            *getReturnGpsDataStatus = HAVE_PROBLEM;
        }
    }
private:
    gps_data_t *getGpsData;

    int *getReturnGpsDataStatus;

    bool &getGpsRunningFlag;

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

GpsDataSend::GpsDataSend(gps_data_t &gpsData,
                         int &ReturnGpsDataStatus,
                         bool &GpsRunningFlag)
{
#ifdef DEBUG_LOCATION_GPS
    qDebug() << "Khoi dong tin hieu GPS cua module UC20G";
#endif

    _Uc20gModuleGps = new Uc20gModuleGps(gpsData,
                                         ReturnGpsDataStatus,
                                         GpsRunningFlag);

    if(_Uc20gModuleGps)
    {
        _Uc20gModuleGps->ConnectFunction();
    }

    // Ket noi tin hieu getDataGpsUpdated() va function positionUpdated()
    connect(_Uc20gModuleGps, SIGNAL(getDataGpsUpdated(QGeoPositionInfo)),
            this, SLOT(positionUpdated(QGeoPositionInfo)));

    // Khoi tao ket noi giua cac signals va slot can thiet
    QGeoPositionInfoSource *source = QGeoPositionInfoSource::createDefaultSource(this);
    if(source)
    {
        connect(source, SIGNAL(positionUpdated(QGeoPositionInfo)), this, SLOT(positionUpdated(QGeoPositionInfo)));
        source->startUpdates();
    }

    // Khoi dong Gps va ket noi port data
    _Uc20gModuleGps->StartGpsSignal();
}

GpsDataSend::~GpsDataSend()
{
    // Xoa vung cap bo nho cua class nay khi khong can thiet nua, no chi hoat dong khi duoc goi
    delete _Uc20gModuleGps;
}

void GpsDataSend::positionUpdated(const QGeoPositionInfo &info)
{
    qDebug() << "Position updated:" << info;

    if (_lastPos != info)
    {
        _lastPos = info;
        emit positionChanged(_lastPos);
    }
}


