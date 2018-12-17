#ifndef FX3USBDEVICE_H
#define FX3USBDEVICE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <DataAnalyse.h>
#include <FX3USBThread.h>

#define STATE_MOVING 1
#define STATE_FINISH_MOVING 0

typedef enum ER3_StepperCurrent
{
    ER3_Current0 = 0,
    ER3_Current33 = 1,
    ER3_Current67 = 2,
    ER3_Current100 = 3,
    ER3_Current100HalfStep = 4,
    ER3_Current100QuarterStep = 5
}ER3_StepperCurrent;

#define ENC_ZERO 32768
#define ENC_LARGE   32768+6200
#define MOTOR_STEP_HOME 0
#define MOTOR_STEP_IN   0
#define MOTOR_STEP_PRE  800
#define MOTOR_STEP_MID  2400
#define MOTOR_STEP_OUT  6200


#define STEPPER_MOVE_PERIOD         100
#define STEPPER_SCAN_PERIOD         30      // 120 >> 2
#define STEPPER_MOVE_DISTANCE       32764   // 8191 << 2
#define STEPPER_MOVE_EXTRA          12
#define STEPPER_MOVE_IRUN           ER3_Current100QuarterStep
#define STEPPER_MOVE_IHOLD          1
#define STEPPER_MOVE_STALLDETECT_ENABLE     1
#define STEPPER_MOVE_STALLDETECT_DISABLE    0
#define STEPPER_MOVE_STALLSTART     15
#define STEPPER_MOVE_STALLRUN       15

#define LASER1_NUMERATOR            8
#define LASER1_DENOM                16711688
#define LASER1_STARTTIME            0
#define LASER1_ENDTIME              8192
#define LASER1_PWM_MODE             1
#define LASER1_PWM_PERIOD           127
#define LASER1_PWM_WIDTH            32

#define LASER2_NUMERATOR            0
#define LASER2_DENOM                16711684
#define LASER2_STARTTIME            0
#define LASER2_ENDTIME              8192
#define LASER2_PWM_MODE             1
#define LASER2_PWM_PERIOD           127
#define LASER2_PWM_WIDTH            42

#define LASER_SELECT                12

#define SCAN_SAMPLES                66560       //256*260
#define SCAN_SELECT                 116

#define OVER_TIME_COUNT             3000        //单位：毫秒

enum TrayPos
{
    TrayPos_Moving = -1,
    TrayPos_Home = 0,
    TrayPos_In = 0,
    TrayPos_Pre,
    TrayPos_Mid,
    TrayPos_Out,
};

enum ScanProcess
{
    scanIdle = 0,
    scanStart,
    scanWait,
    scanWaitIn,
    scanWaitOut,
    scanWaitPre,
    scanning,
    scanDone,
    scanWaitCountFinish
};

/* Audit trail item - fixed 64 quint8 record. */
struct AuditTrail
{
    /* When this action happened. */
    quint32 m_timestamp;
    /* The user who did the action. */
    char m_user_name[12];
    /* The action performed. */
    char m_action[48];
};	// 64 quint8s

/* Motor configuration item. */
struct Motor
{
    /* Configuration settings for X axis. */
    SR3StepperConfig m_settings;
    /* Nudge positions for X axis. */
    int m_position_nudge[8];
};

/* Capacitance reporting configuration structure. */
struct CapConfig
{
    /* Base offset from which all capacitance readings are measured. */
    float m_baseOffset;
    /* Threshhold for cassette present. */
    float m_presentOffset;
    /* Threshold for wet cassette. */
    float m_wetOffset;
};

/* Magic EEPROM signature. */
#define RELIA3_EEPROM_MAGIC		0xbeef1347
/* Magic EEPROM signature including tray scale. */
#define RELIA3_EEPROM_MAGIC2	0xbeef1348
/* Magic EEPROM signature including laser power limits. */
#define RELIA3_EEPROM_MAGIC3	0xbeef1349

/* Contents of a Relia3 unit EEPROM. */
struct UnitEEPROM
{
    /* Signature value. */
    quint32 m_magic;
    /* Unit serial number. */
    quint32 m_serial_number;
    /* Time to delay before scans. */
    quint16 m_scan_delay_time;
    /* Start of bar scan range. */
    quint16 m_left_start;
    /* Width of bar scan region. */
    quint16 m_analyze_width;
    /* Analysis offset for tray 1. */
    quint16 m_tray1_offset;
    /* Analysis offset for tray 2. */
    quint16 m_tray2_offset;
    /* Speed at which we scan strips. */
    quint8 m_strip_scan_speed_val;
    /* Speed at which we scan barcode. */
    quint8 m_barcode_scan_speed_val;
    /* Unit version info. */
    SR3Info m_version;
    /* Original manufacturing info. */
    AuditTrail m_manufactured;
    /* The most recent 3 things done to this unit. */
    AuditTrail m_audit_trail[3];
    /* Sub assembly serial numbers, main board in [0]. (Is 10 items enough?) */
    quint32 m_manifest_sn[10];
    /* Settings for X axis. */
    Motor m_motor_X;
    /* Settings for Y1 axis. */
    Motor m_motor_Y1;
    /* Settings for Y2 axis. */
    Motor m_motor_Y2;
    /* Laser settings. */
    SR3LaserConfig m_laser_settings;
    /* Scan configuration. */
    SR3ScanConfig m_laser_scan_settings;
    /* Configuration for reporting tray 1 capacitance events. */
    CapConfig m_cap_reporting_tray1;
    /* Cofiguration for reporting tray 1 capacitance events. */
    CapConfig m_cap_reporting_tray2;
    /* Analysis time constant. */
    float m_time_constant;
    /* Laser 1 power scaling. */
    //float m_laser1_scale;
    float m_analyze_width_double;
    /* Laser 2 power scaling. */
    float m_laser2_scale;
    /* Checksum of EEPROM. */
    quint8 m_checksum;

    quint8 m_laser1_power;

    quint8 m_laser2_power;

    quint8 m_serial_port;
    /* Tray 2 red power scaling. */
    float m_tray_red_scale;
    /* Tray 2 IR power scaling. */
    float m_tray_ir_scale;
    /* Number of reads to perform */
    quint16 m_num_reads;
    /* Laser warm up delay. */
    quint16 m_laser_delay;
    /* Start threhold for jam detect. */
    quint8 m_start_threshold;
    /* Run threshold for jam detect. */
    quint8 m_run_threshold;

    quint8 m_dummy1;

    quint8 m_dummy2;
    /* Red power must exceed this. */
    //float m_red_power_limit;
    float m_analyze_width_three;
    /* IR power must exceed this. */
    float m_ir_power_limit;
};

///////////////////////////////////////////////////
/// \brief The FX3USBDevice class
///
class FX3USBDevice : public QObject
{
    Q_OBJECT
public:
    explicit FX3USBDevice(QObject *parent = nullptr);
    ~FX3USBDevice();

public:
    FX3USBThread *fx3USBThread;

public:
    bool openDeviceDriver();
    bool closeDeviceDriver();

public:
    void init();
    bool connected;

public:
    //CQuickScanConfig
    StepperConfig stepperMoveConfig;
    LaserConfig laserConfig;
    quint8 laserSelect;
    ScanConfig scanConfig;

    bool motorStartMoving;
    bool motorMoving;
    int motorHoming;
    int motorPosition[10];  //逻辑位置ER3_Position（说明电机位置和运动状态）到物理位置（电机步数）的映射关系
    int motorTrayPosState;  //电机当前状态 TrayPos
    int motorTrayPosTarget; //TrayPos
    bool motorMove(int posIndex, quint16 motor = STEPPER_TRAY, int setConfig = true);
    bool motorMoveAbs(int stepsIndex, quint16 motor = STEPPER_TRAY);
    bool motorMoveHome(quint16 motor = STEPPER_TRAY);

public:
    UnitEEPROM eeprom;
    bool readEEPROM();

public:
    QDateTime oldDateTime;
    int scanErrorCount;
    int scanProcessState;
    QTimer *dealCommandTimer;

public:
    bool isBusy;
    int bufferSize;
    QByteArray sampleBuffer;
    bool isSampleCountRight;

    char *bufer;
    DataAnalyse *dataAnalyse;

    bool dealScanSample();

signals:
    void sendError(int errorNumber);

public slots:
    void receiveNotification(quint32 type, quint32 code);
    void dealCommand();
};

#endif // FX3USBDEVICE_H
