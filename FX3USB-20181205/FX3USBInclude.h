#ifndef FX3USBLIB_H
#define FX3USBLIB_H

#include <QtGlobal>

#define OLD_VENDOR_ID					0xA806
#define OLD_PRODUCT_ID					0x130B

#define VENDOR_ID						0x22C0
#define PRODUCT_ID						0x3001

/* WPARAM Notification that the USB connection is up. */
#define USB_NOTIFY_CONNECTED			(0x01000000L)
/* WPARAM Notification that the USB connection is up. */
#define USB_NOTIFY_DISCONNECTED			(0x02000000L)
/* WPARAM Notification that a notify interrupt packet arrived. */
#define USB_NOTIFY_INTERRUPT_PACKET		(0x03000000L)
/* WPARAM Notification that a data packet has arrived. */
#define USB_NOTIFY_DATA_PACKET			(0x04000000L)

#define EEPROM_SIZE_BYTES				8192
#define EEPROM_USER_OFFSET				16

/* vendor commands */
#define R3_RESET						0x60
#define R3_INFO							0x61
#define R3_NOTIFICATIONS				0x62
#define R3_EEPROM						0x63
#define R3_LASER_CONFIG					0x64
#define R3_LASER_ON						0x65
#define R3_STEPPER_CONFIG				0x66
#define R3_STEPPER_MOVE					0x67
#define R3_SAMPLING_ON					0x68
#define R3_SEND_EVENT					0x69
#define R3_RENUM						0x6A
#define R3_SEND_DUMMY_DATA_PACKET		0x6B
#define R3_SCAN_CONFIG					0x6C
#define R3_TEMPERATURE_CONFIG			0x6D
#define R3_CAPACITANCE_CONFIG			0x6E
#define R3_JTAG_CONFIG					0x6F /* only in relia3conf */
#define R3_STEPPER_SET_HOME				0x70
#define R3_PEEK_POKE					0x71
#define R3_BOGUS						0x7F /* Special to test invalid command. */


#define STEPPER_TRAY 0


#pragma pack(push, 1)

//0~5为电机位置，之后为电机动作？
typedef enum ER3_Position
{
    ER3_TrayUnknown = 0,
    ER3_TrayIn = 1,
    ER3_TrayBetweenInMid = 2,
    ER3_TrayMid = 3,
    ER3_TrayBetweenMidOut = 4,
    ER3_TrayOut = 5,
    ER3_TrayMovingIn =	1 << 6,
    ER3_TrayMovingOut = 1 << 7,
    ER3_TrayPosMask = ~((1 << 6) | (1 << 7)),
    ER3_TrayEncoder = 0x3e,
    ER3_TrayIndex = 0x3f

/*	ER3_SlideUnknown = 0,
    ER3_SlideStarboard = 1,
    ER3_SlideHome = ER3_SlideStarboard,
    ER3_SlideEnc = 2,
    ER3_SlideEncoder = 0x3e,
    ER3_SlideIndex = 0x3f,
    ER3_SlideMovingPort = 1 << 6,
    ER3_SlideMovingStarboard = 1 << 7,
    ER3_SlidePosMask = ~((1 << 6) | (1 << 7))*/
}ER3_Position;

typedef enum ER3_Laser
{
    ER3_Laser1Cont = 1 << 0,
    ER3_Laser2Cont = 1 << 1,
    ER3_Laser1Scan = 1 << 2,
    ER3_Laser2Scan = 1 << 3
}ER3_Laser;

typedef enum ER3_Sampling
{
    ER3_SamplingOff = 0,
    ER3_SamplingCont = 1,
    ER3_SamplingScan = 2,
    ER3_SamplingBurst = 3
}ER3_Sampling;

typedef enum ER3_NotificationEvent
{
    ER3Notify_Nop =				   0,
    ER3Notify_OverRun =			   1 << 0,
    ER3Notify_StepperPosition =	   1 << 1,
    ER3Notify_SamplingDone =	   1 << 2,
    ER3Notify_TrayPosSensors =	   1 << 3,
    ER3Notify_SamplingStartTime =  1 << 4,
    ER3Notify_Laser1StartTime =	   1 << 5,
    ER3Notify_Laser2StartTime =	   1 << 6,
    ER3Notify_FlagTime =		   1 << 7,
    ER3Notify_Timer =			   1 << 8,
    ER3Notify_Temperature =		   1 << 9,
    ER3Notify_Capacitance =		   1 << 10,
    ER3Notify_ContDone =		   1 << 11,
}ER3_NotificationEvent;

typedef struct SR3Info
{
    quint8 m_fwVersion[2];
    quint8 m_procRev;
    quint8 m_fpgaVersion;
    quint8 m_traySerialNumber[2][4];
    quint8 m_mainSerialNumber[4];
    quint8 m_laserVersion;
    quint8 m_laserSerialNumber[4];
}SR3Info;

typedef struct SR3LaserConfig
{
    quint8 m_laser1Numerator[3];
    quint8 m_laser1NumeratorMinusDenominator[3];
    quint8 m_laser1StartTime[2];
    quint8 m_laser1EndTime[2];
    quint8 m_laser1PWMMode;
    quint8 m_laser1PWMPeriod;
    quint8 m_laser1PWMWidth;
    quint8 m_laser2Numerator[3];
    quint8 m_laser2NumeratorMinusDenominator[3];
    quint8 m_laser2StartTime[2];
    quint8 m_laser2EndTime[2];
    quint8 m_laser2PWMMode;
    quint8 m_laser2PWMPeriod;
    quint8 m_laser2PWMWidth;
}SR3LaserConfig;

/* Capacitance sensor config. 暂无使用 */
typedef struct SR3CapsensorConfig
{
    quint8 m_offset;
    quint8 m_range;
}SR3CapsensorConfig;

/* Temperature config. */
typedef struct SR3TempConfig
{
    quint8 m_low[2];
    quint8 m_high[2];
}SR3TempConfig;

typedef struct SR3StepperConfig
{
    quint8 m_stepperPeriod[2];
    quint8 m_stepperDistance[2];
    quint8 m_extraSteps;
    quint8 m_runCurrent;
    quint8 m_holdCurrent;
    quint8 m_stallDetectEnable;
    quint8 m_stallDetectStartThreshold;
    quint8 m_stallDetectRunThreshold;
}SR3StepperConfig;

typedef struct SR3ScanConfig
{
    quint8 m_samples[4];
    quint8 m_channels;
}SR3ScanConfig;

typedef struct SR3Notification
{
    quint8 m_event[2];
    quint8 m_dropped;
    quint8 m_reserved;
    quint8 m_parm[4];
}SR3Notification;

typedef struct SR3VidAndPid
{
    quint8 m_signature;			// must be 0xc0
    quint8 m_vid[2];
    quint8 m_pid[2];
    quint8 m_zeropad[3];
    quint8 m_serial[4];
}SR3VidAndPid;

#pragma pack(pop)

/**
 * @brief Wrapper for low-level info struct
 */
class Info
{
public:
    /* Firmware version */
    quint16 m_fwVersion;
    /* Processor rev. */
    quint8 m_procRev;
    /* FPGA version. */
    quint8 m_fpgaVersion;
    /* Tray serial numbers. */
    quint32 m_traySerialNuumber[2];
    quint32 m_mainSerialNumber;
    quint8 m_laserVersion;
    quint32 m_laserSerialNumber;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbStepperStruct			the low-level struct
     */
    void setConfig(SR3Info &usbInfoStruct)
    {
        memcpy(usbInfoStruct.m_fwVersion, &m_fwVersion, sizeof(quint16));
        usbInfoStruct.m_procRev = m_procRev;
        usbInfoStruct.m_fpgaVersion = m_fpgaVersion;
        usbInfoStruct.m_laserVersion = m_laserVersion;
        memcpy(usbInfoStruct.m_traySerialNumber, m_traySerialNuumber, 2 * sizeof (quint32));
        memcpy(usbInfoStruct.m_mainSerialNumber, &m_mainSerialNumber, sizeof (quint32));
        memcpy(usbInfoStruct.m_laserSerialNumber, &m_laserSerialNumber, sizeof (quint32));
    }
    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbStepperStruct			the low-level struct to populate
     */
    void getConfig(SR3Info &usbInfoStruct)
    {
        memcpy(&m_fwVersion, &usbInfoStruct.m_fwVersion, sizeof (quint16));
        m_procRev = usbInfoStruct.m_procRev;
        m_fpgaVersion = usbInfoStruct.m_fpgaVersion;
        m_laserVersion = usbInfoStruct.m_laserVersion;
        memcpy(&m_traySerialNuumber, &usbInfoStruct.m_traySerialNumber, 2 * sizeof (quint32));
        memcpy(&m_mainSerialNumber, &usbInfoStruct.m_mainSerialNumber, sizeof (quint32));
        memcpy(&m_laserSerialNumber, &usbInfoStruct.m_laserSerialNumber, sizeof (quint32));
    }
};

/**
 * @brief Wrapper for low-level laser config
 */
class LaserConfig
{
public:
    /* Laser 1 numerator */
    quint32 m_laser1Numerator;
    /* Laser 1 denominator */
    qint32 m_laser1Denominator;
    /* Laser 1 start time */
    quint16 m_laser1StartTime;
    /* Laser 1 end time */
    quint16 m_laser1EndTime;
    /* Laser 1 PWM mode */
    quint8 m_laser1PWMMode;
    /* Laser 1 PWM period */
    quint8 m_laser1PWMPeriod;
    /* Laser 1 PWM width */
    quint8 m_laser1PWMWidth;
    /* Laser 2 numerator */
    quint32 m_laser2Numerator;
    /* Laser 2 denominator */
    qint32 m_laser2Denominator;
    /* Laser 2 start time */
    quint16 m_laser2StartTime;
    /* Laser 2 end time */
    quint16 m_laser2EndTime;
    /* Laser 2 PWM mode */
    quint8 m_laser2PWMMode;
    /* Laser 2 PWM period */
    quint8 m_laser2PWMPeriod;
    /* Laser 2 PWM width */
    quint8 m_laser2PWMWidth;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbStepperStruct			the low-level struct
     */
    void setConfig(SR3LaserConfig &usbLaserStruct)
    {
        memcpy(usbLaserStruct.m_laser1Numerator, &m_laser1Numerator, 3 * sizeof (quint8));
        qint32 temp = m_laser1Numerator - m_laser1Denominator;
        memcpy(usbLaserStruct.m_laser1NumeratorMinusDenominator, &temp, 3 * sizeof (quint8));
        memcpy(usbLaserStruct.m_laser1StartTime, &m_laser1StartTime, sizeof (quint16));
        memcpy(usbLaserStruct.m_laser1EndTime, &m_laser1EndTime, sizeof (quint16));
        usbLaserStruct.m_laser1PWMMode = m_laser1PWMMode;
        usbLaserStruct.m_laser1PWMPeriod = m_laser1PWMPeriod;
        usbLaserStruct.m_laser1PWMWidth = m_laser1PWMWidth;

        memcpy(usbLaserStruct.m_laser2Numerator, &m_laser2Numerator, 3 * sizeof (quint8));
        temp = m_laser2Numerator - m_laser2Denominator;
        memcpy(usbLaserStruct.m_laser2NumeratorMinusDenominator, &temp, 3 * sizeof (quint8));
        memcpy(usbLaserStruct.m_laser2StartTime, &m_laser2StartTime, sizeof (quint16));
        memcpy(usbLaserStruct.m_laser2EndTime, &m_laser2EndTime, sizeof (quint16));
        usbLaserStruct.m_laser2PWMMode = m_laser2PWMMode;
        usbLaserStruct.m_laser2PWMPeriod = m_laser2PWMPeriod;
        usbLaserStruct.m_laser2PWMWidth = m_laser2PWMWidth;
    }
    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbStepperStruct			the low-level struct to populate
     */
    void getConfig(SR3LaserConfig &usbLaserStruct)
    {
        memcpy(&m_laser1Numerator, usbLaserStruct.m_laser1Numerator, 3 * sizeof (quint8));
        qint32 temp;
        memcpy(&temp, usbLaserStruct.m_laser1NumeratorMinusDenominator, 3 * sizeof (quint8));
        if (temp & (1 << 23))
        {
            temp |= ~((1 << 24) - 1);
        }
        m_laser1Denominator = m_laser1Numerator - temp;
        memcpy(&m_laser1StartTime, usbLaserStruct.m_laser1StartTime, sizeof (quint16));
        memcpy(&m_laser1EndTime, usbLaserStruct.m_laser1EndTime, sizeof (quint16));
        m_laser1PWMMode = usbLaserStruct.m_laser1PWMMode;
        m_laser1PWMPeriod = usbLaserStruct.m_laser1PWMPeriod;
        m_laser1PWMWidth = usbLaserStruct.m_laser1PWMWidth;

        memcpy(&m_laser2Numerator, usbLaserStruct.m_laser2Numerator, 3 * sizeof (quint8));
        memcpy(&temp, usbLaserStruct.m_laser2NumeratorMinusDenominator, 3 * sizeof (quint8));
        if (temp & (1 << 23))
        {
            temp |= ~((1 << 24) - 1);
        }
        m_laser2Denominator = m_laser2Numerator - temp;
        memcpy(&m_laser2StartTime, usbLaserStruct.m_laser2StartTime, sizeof (quint16));
        memcpy(&m_laser2EndTime, usbLaserStruct.m_laser2EndTime, sizeof (quint16));
        m_laser2PWMMode = usbLaserStruct.m_laser2PWMMode;
        m_laser2PWMPeriod = usbLaserStruct.m_laser2PWMPeriod;
        m_laser2PWMWidth = usbLaserStruct.m_laser2PWMWidth;
    }
};

/**
 * @brief Wrapper for low-level stepper config
 */
class StepperConfig
{
public:
    /* Stepper period */
    quint16 m_stepperPeriod;
    /* Stepper distance */
    quint16 m_stepperDistance;
    /* Extra steps */
    quint8 m_extraSteps;
    /* Running current. */
    quint8 m_runCurrent;
    /* Holding current. */
    quint8 m_holdCurrent;
    /* Stall detect enable. */
    quint8 m_stallDetectEnable;
    /* Stall detect start. */
    quint8 m_stallDetectStartThreshold;
    /* Stall detect run. */
    quint8 m_stallDetectRunThreshold;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbStepperStruct			the low-level struct
     */
    void setConfig(SR3StepperConfig &usbStepperStruct)
    {
        memcpy(&usbStepperStruct, this, sizeof(usbStepperStruct));
    }
    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbStepperStruct			the low-level struct to populate
     */
    void getConfig(SR3StepperConfig &usbStepperStruct)
    {
        memcpy(this, &usbStepperStruct, sizeof(usbStepperStruct));
    }
};

/**
 * @brief Wrapper for low-level scan config
 */
class ScanConfig
{
public:
    /* Number of scan samples. */
    quint32 m_samples;
    /* Channels to scan. */
    quint8 m_channels;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbStepperStruct			the low-level struct
     */
    void setConfig(SR3ScanConfig &usbScanStruct)
    {
        memcpy(&usbScanStruct, this, sizeof(usbScanStruct));
    }
    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbStepperStruct			the low-level struct to populate
     */
    void getConfig(SR3ScanConfig &usbScanStruct)
    {
        memcpy(this, &usbScanStruct, sizeof(usbScanStruct));
    }
};

/**
 * @brief Wrapper for low-level temperature config
 */
class TempConfig
{
public:
    /* Low temperature. */
    float m_low;
    /* High temperature. */
    float m_high;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbTempStruct			the low-level struct
     */
    void setConfig(SR3TempConfig &usbTempStruct)
    {
        quint16 temp = (quint16)(m_low * 256.0f);
        memcpy(usbTempStruct.m_low, &temp, sizeof(quint16));
        temp = (quint16)(m_high * 256.0f);
        memcpy(usbTempStruct.m_high, &temp, sizeof(quint16));
    }
    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbTempStruct			the low-level struct to populate
     */
    void getConfig(SR3TempConfig &usbTempStruct)
    {
        quint16 temp;
        memcpy(&temp, usbTempStruct.m_low, sizeof(quint16));
        m_low = temp / 256.0f;
        memcpy(&temp, usbTempStruct.m_high, sizeof(quint16));
        m_high = temp / 256.0f;
    }
};

/**
 * @brief Wrapper for low-level cap sensor config
 */
class CapsensorConfig
{
public:
    /* Which range limit 0-3. */
    int m_range;
    /* Limit value 0.0 - 255.0. */
    float m_offset;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbCapStruct				the low-level struct
     */
    void setConfig(SR3CapsensorConfig &usbCapStruct)
    {
        usbCapStruct.m_range = m_range;
        int val = (int)(m_offset);
        quint8 bVal = (quint8)val;
        if (val < 0)
        {
            bVal = 0;
        }
        if (val > 255)
        {
            bVal = 255;
        }
        usbCapStruct.m_offset = bVal;
    }

    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbCapStruct				the low-level struct to populate
     */
    void getConfig(SR3CapsensorConfig &usbCapStruct)
    {
        m_range = usbCapStruct.m_range;
        quint8 bVal = usbCapStruct.m_offset;
        m_offset = (float)bVal;
    }
};

/**
 * @brief Wrapper for low-level VID/PID config
 */
class VidAndPid
{
public:
    /* Device boot VID. */
    quint16 m_vid;
    /* Device boot PID. */
    quint16 m_pid;
    /* Device serial number. */
    quint32 m_serial;
    /**
     * @brief Populate the structure with values from low level
     * @param [in] usbStepperStruct			the low-level struct
     */
    void setConfig(SR3VidAndPid &usbVidPidStruct)
    {
        usbVidPidStruct.m_signature = 0xC0;
        memcpy(usbVidPidStruct.m_vid, &m_vid, sizeof(quint16));
        memcpy(usbVidPidStruct.m_pid, &m_pid, sizeof(quint16));
        //usbVidPidStruct.m_serial[4]为大端字节序
        usbVidPidStruct.m_serial[3] = (quint8)(m_serial & 0xFF);
        usbVidPidStruct.m_serial[2] = (quint8)((m_serial >> 8) & 0xFF);
        usbVidPidStruct.m_serial[1] = (quint8)((m_serial >> 16) & 0xFF);
        usbVidPidStruct.m_serial[0] = (quint8)((m_serial >> 24) & 0xFF);
    }

    /**
     * @brief Populate the low-level struct with values
     * @param [in] usbStepperStruct			the low-level struct to populate
     */
    void getConfig(SR3VidAndPid &usbVidPidStruct)
    {
        memcpy(&m_vid, usbVidPidStruct.m_vid, sizeof(quint16));
        memcpy(&m_pid, usbVidPidStruct.m_pid, sizeof (quint16));
        //usbVidPidStruct.m_serial[4]为大端字节序
        m_serial = (usbVidPidStruct.m_serial[0] << 24) |
                   (usbVidPidStruct.m_serial[1] << 16) |
                   (usbVidPidStruct.m_serial[2] << 8) |
                    usbVidPidStruct.m_serial[3];
    }
};



#endif // FX3USBLIB_H
