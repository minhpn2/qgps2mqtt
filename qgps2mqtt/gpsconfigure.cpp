#include "gpsconfigure.h"

gpsconfigure::gpsconfigure(gps_data_t *const gpsData,
                           int *const returnGpsDataStatus,
                           bool *const gpsRunningFlag)
    : getGpsData(gpsData),
      getReturnGpsDataStatus(returnGpsDataStatus),
      getGpsRunningFlag(gpsRunningFlag)
{
    // Contructor
}
gpsconfigure::~gpsconfigure()
{

}

void gpsconfigure::StartGpsModule()
{
    wiringPiSetup(); // Configure GPIO on raspberry pi
    pinMode(raspberryPiGpsStart, OUTPUT); // Dinh nghia chan 31 cua Raspberry: Output

    if (false == *getGpsRunningFlag)
    {
        // GPS chi khoi dong khi co xung len tai chan RaspberryPi_GPS_Start
        digitalWrite(raspberryPiGpsStart, LOW);
        delayMicroseconds(10); // Delay 10ms
        digitalWrite(raspberryPiGpsStart, HIGH);
    }
}

void gpsconfigure::ResetGpsModule(const unsigned int &TimeResetGps)
{
    wiringPiSetup();
    pinMode(raspberryPiGpsReset, OUTPUT);

    if(true == *getGpsRunningFlag)
    {
        // Set off-->on--> off chan Reset cua GPS trong mot thoi gian
        digitalWrite(raspberryPiGpsReset, LOW);
        delayMicroseconds(TimeResetGps);
        digitalWrite(raspberryPiGpsReset, HIGH);
        delayMicroseconds(TimeResetGps);
        digitalWrite(raspberryPiGpsReset, LOW);
    }
}

void gpsconfigure::StopGpsModule()
{
    wiringPiSetup(); // Configure GPIO on raspberry pi
    pinMode(raspberryPiGpsStart, OUTPUT); // Dinh nghia chan 31 cua Raspberry: Output
    // GPS chi khoi dong khi co xung len tai chan RaspberryPi_GPS_Start
    if(true == *getGpsRunningFlag)
    {
        digitalWrite(raspberryPiGpsStart, HIGH);
        delayMicroseconds(delayTimeStartMS);
        digitalWrite(raspberryPiGpsStart, LOW);
    }
}

void gpsconfigure::InitializeGpsSignal()
{
    for (int i = 0; i < NUMBER_LOOP_WAIT_CREATED_UC20_COMMAND; i++)
    {
        if(!QFileInfo(pathUc20Command).exists())
        {
#ifdef DEBUG_LOCATION_GPS
            qDebug() << "Waiting for InitializeGpsSignal\n";
#endif
            *getReturnGpsDataStatus = HAVE_PROBLEM;

            *getGpsRunningFlag = false;
        }

        else
        {
#ifdef DEBUG_LOCATION_GPS
            qDebug() << "GPS Module Started!!! \n" <<
                        "Initializing Gps Signal... \n";
#endif
            QProcess sendCommandAT;
            sendCommandAT.start("sh", QStringList() <<
                                "-c" <<
                                "echo \"AT+QGPS=1\""
                                " > "
                                "/dev/ttyUSB_UC20_COMMAND",
                                QIODevice::WriteOnly);
            sendCommandAT.waitForStarted();
            sendCommandAT.execute("sudo gpsd /dev/ttyUSB_UC20_DATA"
                                  " -F "
                                  "/var/run/gpsd.sock");
            sendCommandAT.waitForFinished();
            sendCommandAT.close();

            *getReturnGpsDataStatus = NO_DATA_GPS;
            // Gps Module da duoc khoi dong xong
            *getGpsRunningFlag = true;

            // Exit loop
            break;
        }

        delay(1000);
    }
}

bool gpsconfigure::ConnectGpsPort(bool OpenGpsPortData)
{
    try
    {
        if(-1 == gps_open("localhost", "2947", getGpsData))
        {
#ifdef DEBUG_LOCATION_GPS
            qDebug() << "ConnectGpsPort failed!!!";
#endif

            // Khong the ket noi voi Gps port data
            *getReturnGpsDataStatus = HAVE_PROBLEM;
            *getGpsRunningFlag = false;
        }

        else if((true == OpenGpsPortData) &&
                (-1 != gps_open("localhost", "2947", getGpsData)))
        {
            // Doc du lieu tu Gps port
            gps_stream(getGpsData, WATCH_ENABLE | WATCH_JSON, nullptr);

            // Da ket noi duoc voi Gps port
            *getGpsRunningFlag = true;
            *getReturnGpsDataStatus = NO_DATA_GPS;

        }

    }
    catch (...)
    {
        // Su dung ham try catch nay de tranh truong hop loi xay ra:
        // - Gps port mat ket noi khi gps_read dang doc.
        *getReturnGpsDataStatus = HAVE_PROBLEM;

        *getGpsRunningFlag = false;
        // Thong bao loi da xay ra
    }
    return *getGpsRunningFlag;
}

void gpsconfigure::StartGpsSignal()
{
    // Chuong trinh khoi dong Gps, chuong trinh nay se duoc goi dau tien
    StartGpsModule();

    if(false == *getGpsRunningFlag)
    {
        ConnectFunction();
    }

    // Configure pin of GPS module
    InitializeGpsSignal();

    // Ket noi voi port du lieu GPS voi GPSD port
    if(true == ConnectGpsPort(*getGpsRunningFlag))
    {
        qDebug() << "StartGpsSignal successed!!!";
    }
    else
    {
        // Khong the ket noi Gps duoc
        qDebug() << "StartGpsSignal failed!!!";
    }
}

void gpsconfigure::ConnectFunction()
{
#ifdef DEBUG_LOCATION_GPS
    qDebug() << "Connect Function!!!";
#endif
    connect(this, SIGNAL(callInitializeGpsSignalWhenIssueHappened()),
            this, SLOT(InitializeGpsSignalWhenIssueHappened()));

}

// Chuong trinh nay dung de goi lay du lieu tu ben ngoai nhu MQTT, hay gi do, tuy ...
void gpsconfigure::getDataGpsChanged()
{
    try
    {
        // Kiem tra trang thai hoat dong cua Gps Module thong qua gpsRunningFlag xem co du lieu moi cap nhap hay khoong thong qua ham gps_waiting(), thoi gian cho la 5s
        if((true == *getGpsRunningFlag)
           && (true == gps_waiting(getGpsData, 5000)))
        {
            if(-1 == gps_read(getGpsData))
            {
#ifdef DEBUG_LOCATION_GPS
                qDebug() << "getDataGpsChanged failed!!!";
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
                    qDebug("latitude: %f, longitude: %f, speed: %f, timestamp: %lf\n", getGpsData->fix.latitude, getGpsData->fix.longitude, getGpsData->fix.speed, getGpsData->fix.time);
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
#ifdef DEBUG_LOCATION_GPS
                    qDebug() << "No updated data!!!";
#endif
                }
            }
        }

        else if((true == *getGpsRunningFlag)
                && (false == gps_waiting(getGpsData, 5000))
                && !QFileInfo(pathUc20Command).exists())
        {
            /* Truong hop nay xay ra khi Gps khong hoat dong (gpsRunningFlag = false) hoac khong co du lieu Gps duoc day len trong 5s
             * Goi chuong tirnh khoi dong lai gps
             */

#ifdef DEBUG_LOCATION_GPS
            qDebug() << "GPS module was reseted!!!";
#endif
            QProcess sendCommandAT;
            sendCommandAT.start("sh", QStringList() <<
                                "-c" <<
                                "sudo killall gpsd",
                                QIODevice::WriteOnly);
            sendCommandAT.waitForStarted();
            sendCommandAT.execute("sudo rm /var/run/gpsd.sock");
            sendCommandAT.execute("sudo systemctl stop gpsd.service");
            sendCommandAT.waitForFinished();
            sendCommandAT.close();

            // Thong bao Module GPS da bi reset
            emit callInitializeGpsSignalWhenIssueHappened();
        }

        else if((true == *getGpsRunningFlag)
                && (false == gps_waiting(getGpsData, 5000))
                && QFileInfo(pathUc20Command).exists())
        {
            /*
             * Truong hop nay xay ra khi bi mat tinh hieu GPS
             */
#ifdef DEBUG_LOCATION_GPS
            qDebug() << "GPS signal lost!!!";
#endif
            *getReturnGpsDataStatus = NO_SIGNAL_GPS;
        }
    }

    catch (...)
    {
#ifdef DEBUG_LOCATION_GPS
    qDebug() << "Co loi xay ra voi port du lieu Gps!!!";
#endif
    *getReturnGpsDataStatus = HAVE_PROBLEM;
    *getGpsRunningFlag = true;
    // Gui yeu cau khoi dong lai Gps Module
    emit callInitializeGpsSignalWhenIssueHappened();
    }
}

//private slots:

void gpsconfigure::InitializeGpsSignalWhenIssueHappened()
{
    try
    {
        // Truong hop xay ra van de khong the khoi dong GPS duoc, chuong trinh nay se ket noi lai tin hieu GPS voi GPSD.
        if(false == *getGpsRunningFlag)
//            && (HAVE_PROBLEM == *getReturnGpsDataStatus))
        {
            // Chuong trinh duoc yeu cau khoi dong lai tu dau
            StartGpsSignal();
        }

        // Truong hop khoi dong lai sau khi bi reset
        else if(true == *getGpsRunningFlag)
//               && (HAVE_PROBLEM == *getReturnGpsDataStatus))
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
        *getGpsRunningFlag = false;
        *getReturnGpsDataStatus = HAVE_PROBLEM;
    }
}
