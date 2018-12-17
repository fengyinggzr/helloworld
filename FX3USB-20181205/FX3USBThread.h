#ifndef FX3USBTHREAD_H
#define FX3USBTHREAD_H

#include <inc/CyAPI.h>
#include <QThread>
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <FX3USBInclude.h>

#define INTERRUPT_TRANSFER  false
#define DATA_TRANSFER       true

#define INTERRUPT_BUFF_SIZE 16
#define DATA_BUFF_SIZE      49152

class FX3USBThread : public QThread
{
    Q_OBJECT
public:
    explicit FX3USBThread(QObject *parent = nullptr);
    ~FX3USBThread();

public:
    /* Interrupt endpoint - 0x81. */
    CCyInterruptEndPoint *interruptEpt = nullptr;
    /* Data endpoint - 0x86. */
    CCyBulkEndPoint *dataEpt = nullptr;
//    CCyControlEndPoint *controlEpt = nullptr;
    CCyUSBDevice *usbDevice = nullptr;

public:
    bool sendCommand(/*CTL_XFER_TGT_TYPE target,
                     CTL_XFER_REQ_TYPE reqType,*/
                     CTL_XFER_DIR_TYPE direction,
                     quint8 reqCode,
                     quint16 value,
                     quint16 index,
                     quint8 *data, LONG &length);
///////////////////////////////////////////
///USB操作命令
public:
    bool bogus();
    bool resetDevice();
////////////////////////////////////////////////
    bool getR3Info(SR3Info &info);
    bool getR3ScanConfig(SR3ScanConfig &scanConfig);
    bool setR3ScanConfig(SR3ScanConfig scanConfig);
    bool getR3StepperConfig(SR3StepperConfig &stepperConfig, quint16 motor = STEPPER_TRAY);
    bool setR3StepperConfig(SR3StepperConfig stepperConfig, quint16 motor = STEPPER_TRAY);
    bool getR3TempConfig(quint8 which, SR3TempConfig &tempConfig);
    bool setR3TempConfig(quint8 which, SR3TempConfig tempConfig);
    bool getR3CapsensorConfig(quint8 which, SR3CapsensorConfig &capsensorConfig);
    bool setR3CapsensorConfig(quint8 which, SR3CapsensorConfig capsensorConfig);
    bool getR3LaserConfig(SR3LaserConfig &laserConfig);
    bool setR3LaserConfig(SR3LaserConfig laserConfig);
    bool readR3VIDandPID(SR3VidAndPid &vidAndPid);
//////////////////////////////////////////////////
    bool getInfo(Info &info);
    bool getScanConfig(ScanConfig &scanConfig);
    bool setScanConfig(ScanConfig scanConfig);
    bool getStepperConfig(StepperConfig &stepperConfig, quint16 motor = STEPPER_TRAY);
    bool setStepperConfig(StepperConfig stepperConfig, quint16 motor = STEPPER_TRAY);
    bool getTempConfig(quint8 which, TempConfig &tempConfig);
    bool setTempConfig(quint8 which, TempConfig tempConfig);
    bool getCapsensorConfig(quint8 which, CapsensorConfig &capsensorConfig);
    bool setCapsensorConfig(quint8 which, CapsensorConfig capsensorConfig);
    bool getLaserConfig(LaserConfig &laserConfig);
    bool setLaserConfig(LaserConfig laserConfig);
    bool readVIDAndPID(VidAndPid &vidAndPid);
///////////////////////////////////////////////////
    bool setNotifications(quint16 notifyBits);
    bool requestEventResult(quint16 event);

    bool readEEPROM(quint16 addr, quint8 len, quint8 *data);
    bool writeEEPROM(quint16 addr, quint8 len, quint8 *data);

    bool peek(quint16 addr, quint8 &value);
    bool poke(quint16 addr, quint8 value);

    bool setLaser(ER3_Laser laserSet);
    bool moveStepper(ER3_Position pos, quint16 motor = STEPPER_TRAY, int index = 0);
    bool setStepperHome(quint16 motor = STEPPER_TRAY);
    bool setSamplingState(ER3_Sampling samplingState);

///////////////////////////////////////////
public:
    //启动线程标志位
    bool started;
    bool isRunning;
    bool transferState;
    bool stopDataTransfer;
    QVector<QByteArray> interruptQueue;
    QVector<QByteArray> dataQueue;

public:
    int count;
    bool openDeviceDriver();
    bool closeDeviceDriver();
    void startThread();
    void stopThread();

protected:
    void run();

signals:
    /**
     * @brief sendNotification
     * @param type  通知类型
     * @param code  类型参数
     */
    void sendNotification(quint32 type, quint32 code);

};

#endif // FX3USBTHREAD_H
