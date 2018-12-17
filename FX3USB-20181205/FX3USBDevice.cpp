#include "FX3USBDevice.h"

FX3USBDevice::FX3USBDevice(QObject *parent) : QObject(parent)
{
    init();
}

FX3USBDevice::~FX3USBDevice()
{
    closeDeviceDriver();
//    disconnect(fx3USBThread, &FX3USBThread::sendNotification, this, &FX3USBDevice::receiveNotification);
    delete fx3USBThread;
}

bool FX3USBDevice::openDeviceDriver()
{
    return fx3USBThread->openDeviceDriver();
}

bool FX3USBDevice::closeDeviceDriver()
{
    return fx3USBThread->closeDeviceDriver();
}

void FX3USBDevice::init()
{
    fx3USBThread = new FX3USBThread(this);
    connect(fx3USBThread, &FX3USBThread::sendNotification, this, &FX3USBDevice::receiveNotification);

    scanErrorCount = 0;

    dealCommandTimer = new QTimer(this);
    QObject::connect(dealCommandTimer, &QTimer::timeout, this, &FX3USBDevice::dealCommand);
    dealCommandTimer->start(15);

    connected = false;
    motorMoving = false;
    motorTrayPosState = 0xFF;
    motorTrayPosTarget = 0xFF;
    memset(motorPosition, 0, 3 * sizeof(int));

    isBusy = false;
    bufferSize = SCAN_SAMPLES * 4 * 3;
    scanProcessState = ScanProcess::scanIdle;


    motorPosition[TrayPos::TrayPos_Home]    = ENC_ZERO + MOTOR_STEP_HOME;
    motorPosition[TrayPos::TrayPos_Pre]     = ENC_ZERO + MOTOR_STEP_PRE;
    motorPosition[TrayPos::TrayPos_Mid]     = ENC_ZERO + MOTOR_STEP_MID;
    motorPosition[TrayPos::TrayPos_Out]     = ENC_ZERO + MOTOR_STEP_OUT;

    stepperMoveConfig.m_stepperPeriod               = STEPPER_MOVE_PERIOD;
    stepperMoveConfig.m_stepperDistance             = STEPPER_MOVE_DISTANCE;
    stepperMoveConfig.m_extraSteps                  = STEPPER_MOVE_EXTRA;
    stepperMoveConfig.m_runCurrent                  = STEPPER_MOVE_IRUN;
    stepperMoveConfig.m_holdCurrent                 = STEPPER_MOVE_IHOLD;
    stepperMoveConfig.m_stallDetectEnable           = STEPPER_MOVE_STALLDETECT_ENABLE;
    stepperMoveConfig.m_stallDetectStartThreshold   = STEPPER_MOVE_STALLSTART;
    stepperMoveConfig.m_stallDetectRunThreshold     = STEPPER_MOVE_STALLRUN;

    laserConfig.m_laser1Numerator   = LASER1_NUMERATOR;
    laserConfig.m_laser1Denominator = LASER1_DENOM;
    laserConfig.m_laser1StartTime   = LASER1_STARTTIME;
    laserConfig.m_laser1EndTime     = LASER1_ENDTIME;
    laserConfig.m_laser1PWMMode     = LASER1_PWM_MODE;
    laserConfig.m_laser1PWMPeriod   = LASER1_PWM_PERIOD;
    laserConfig.m_laser1PWMWidth    = LASER1_PWM_WIDTH;

    laserConfig.m_laser2Numerator   = LASER2_NUMERATOR;
    laserConfig.m_laser2Denominator = LASER2_DENOM;
    laserConfig.m_laser2StartTime   = LASER2_STARTTIME;
    laserConfig.m_laser2EndTime     = LASER2_ENDTIME;
    laserConfig.m_laser2PWMMode     = LASER2_PWM_MODE;
    laserConfig.m_laser2PWMPeriod   = LASER2_PWM_PERIOD;
    laserConfig.m_laser2PWMWidth    = LASER2_PWM_WIDTH;

    laserSelect = LASER_SELECT;

    scanConfig.m_samples    = SCAN_SAMPLES;
    scanConfig.m_channels   = SCAN_SELECT;      //该参数暂时不用？
}

bool FX3USBDevice::motorMove(int posIndex, quint16 motor, int setConfig)
{
    bool ret;
    if(motorMoving)
        return false;
    if(motorTrayPosState == posIndex)
        return false;

    if(posIndex == TrayPos::TrayPos_Home || motorPosition[posIndex] == MOTOR_STEP_HOME)
    {
        motorHoming = 1;
        stepperMoveConfig.m_stepperPeriod = (STEPPER_MOVE_PERIOD * 3 / 2) >> 2;
        stepperMoveConfig.m_stallDetectEnable = STEPPER_MOVE_STALLDETECT_ENABLE;
        fx3USBThread->setStepperConfig(stepperMoveConfig);
        ret = fx3USBThread->moveStepper(ER3_TrayIn, motor, 0);
    }
    else
    {
        motorHoming = 0;
        if(setConfig)
        {
            stepperMoveConfig.m_stepperPeriod = STEPPER_MOVE_PERIOD >> 2;
            stepperMoveConfig.m_stallDetectEnable = STEPPER_MOVE_STALLDETECT_DISABLE;
            fx3USBThread->setStepperConfig(stepperMoveConfig);
        }
        //posIndex为TrayPos_Home 对应 motorPosition[0]
        //posIndex为TrayPos_Pre 对应 motorPosition[1]
        //posIndex为TrayPos_Mid 对应 motorPosition[2]
        //posIndex为TrayPos_Out 对应 motorPosition[3]
        if(posIndex == TrayPos::TrayPos_Pre)
        {
            int pos = motorPosition[posIndex];
            if(eeprom.m_version.m_fpgaVersion > 14)
            {
                pos -= 150;
            }
            ret = fx3USBThread->moveStepper(ER3_TrayEncoder, motor, pos);
        }
        else
            ret = fx3USBThread->moveStepper(ER3_TrayEncoder, motor, motorPosition[posIndex]);
//        motorMoveAbs(motorPosition[posIndex / 2], motor);
    }
    if(!ret)
        return false;

    motorTrayPosTarget = posIndex;
    motorMoving = true;
    motorStartMoving = false;

    return true;
}

bool FX3USBDevice::motorMoveAbs(int stepsIndex, quint16 motor)
{
    if(motorMoving)
        return false;

    fx3USBThread->moveStepper(ER3_TrayEncoder, motor, stepsIndex);
    motorTrayPosTarget = TrayPos::TrayPos_Moving;
    motorMoving = true;
    motorStartMoving = false;
    return true;
}

bool FX3USBDevice::motorMoveHome(quint16 motor)
{
    if(motorMoving)
        return false;
    if(motorTrayPosState == TrayPos::TrayPos_Home)
        return true;

    motorTrayPosTarget = TrayPos::TrayPos_Home;
    motorHoming = 1;
    stepperMoveConfig.m_stepperPeriod = (STEPPER_MOVE_PERIOD * 3 / 2) >> 2;
    stepperMoveConfig.m_stallDetectEnable = STEPPER_MOVE_STALLDETECT_ENABLE;
    fx3USBThread->setStepperConfig(stepperMoveConfig);
    if(!fx3USBThread->moveStepper(ER3_TrayIn, motor, 0))
        return false;

    motorMoving = true;
    motorStartMoving = false;
    return true;
}

bool FX3USBDevice::readEEPROM()
{
    quint16 i = 0;
    quint8 *data = reinterpret_cast<quint8 *>(&eeprom);
    quint16 length = sizeof(UnitEEPROM);

    for(i = 0; i < length; i += 32)
    {
        quint16 len = min(32, length - i);
        if(!fx3USBThread->readEEPROM(0x400 + i, static_cast<quint8>(len), data))
        {
            qDebug()<<"read eeprom wrong";
        }
        else
        {
            QByteArray array;
            array.append(reinterpret_cast<char *>(data), 32);
            qDebug()<<QString("eeprom %1:").arg(i)<<array.toHex();

            data += 32;
        }
    }

    return true;
}



bool FX3USBDevice::dealScanSample()
{
//    ::Sleep(5);


    int i;

//    dataAnalyse = new DataAnalyse();



//    for (i = 0; i < 12288 / 8; i++)
//    {
//        qDebug()<<"samplebuff "<<i<<": "<<sampleBuffer.mid(i * 8, 8).toHex();
//    }

//    bufer = sampleBuffer.data();
    dataAnalyse = new DataAnalyse(sampleBuffer);
//    dataAnalyse.rawData = sampleBuffer.data();
    dataAnalyse->extractRawData();

    return true;
}



void FX3USBDevice::receiveNotification(quint32 type, quint32 code)
{
    QByteArray array;
    SR3Notification notify;
    quint16 notifyType;

    quint32 parm;
    quint8 motorState;
    quint8 logicPos;
    qint16 motorPos;
    quint8 motor;
    quint8 status;
    int movingState;
//    bool ret;

//    int m_move_speed = 100;
//    int distance = 8191;

    Q_UNUSED(code);
    switch (type & 0xFF000000)
    {
    case USB_NOTIFY_CONNECTED:
        qDebug()<<"USB_NOTIFY_CONNECTED";
        connected = true;
        fx3USBThread->bogus();
        if(!fx3USBThread->resetDevice())
        {
            connected = false;
            break;
        }

///////////////////////////////////////////////////////
///可以不要？
        VidAndPid vidAndPid;
        if(!fx3USBThread->readVIDAndPID(vidAndPid))
        {

        }

        Info info;
        if(!fx3USBThread->getInfo(info))
        {

        }

        TempConfig tempConfig;
        tempConfig.m_low = 0.0;
        tempConfig.m_high = 0.0;
        if(!fx3USBThread->setTempConfig(0, tempConfig))
        {

        }

        CapsensorConfig capsensorConfig;
        capsensorConfig.m_range = 0;
        capsensorConfig.m_offset = 0.0;
        if(!fx3USBThread->setCapsensorConfig(0, capsensorConfig))
        {

        }
//////////////////////////////////////////////////////////////


        fx3USBThread->setStepperConfig(stepperMoveConfig);
        fx3USBThread->setNotifications(0x01FF); //每一位对应事件ER3_NotificationEvent的每一位，表明进行通知的事件
        fx3USBThread->requestEventResult(ER3Notify_StepperPosition | ER3Notify_TrayPosSensors/* |
                                         ER3Notify_Temperature | ER3Notify_Capacitance*/); //请求相关事件通知结果（单次）

        if(!readEEPROM())
        {
            qDebug()<<"read eeprom fail";
        }


        break;
    case USB_NOTIFY_DISCONNECTED:
        connected = false;
        break;
    case USB_NOTIFY_INTERRUPT_PACKET:
        if(connected && fx3USBThread->usbDevice != nullptr)
        {
            array = fx3USBThread->interruptQueue.first();
            fx3USBThread->interruptQueue.removeFirst();
            memcpy(&notify, array.data(), sizeof (SR3Notification));
            memcpy(&notifyType, notify.m_event, 2 * sizeof(quint8));
            switch (notifyType)
            {
            case ER3Notify_Nop:
                break;
            case ER3Notify_TrayPosSensors:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - Tray sensors";
                break;
            case ER3Notify_SamplingStartTime:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_SamplingStartTime";
                // Notify is SamplingStartTime, and we ignore it.
                break;
            case ER3Notify_Laser1StartTime:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_Laser1StartTime";
                // Notify is Laser1StartTime, and we ignore it.
                break;
            case ER3Notify_Laser2StartTime:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_Laser2StartTime";
                // Notify is Laser2StartTime, and we ignore it.
                break;
            case ER3Notify_FlagTime:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_FlagTime";
                // Notify is FlagTime, and we ignore it.
                break;
            case ER3Notify_Timer:
                // Notify is Timer, and we ignore it.
                break;
            case ER3Notify_OverRun:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_OverRun";
                // Notify is OverRun, and we ignore it.
                fx3USBThread->setSamplingState(ER3_SamplingOff);
                break;
            case ER3Notify_Temperature:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_Temperature";
                break;
            case ER3Notify_Capacitance:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_Capacitance";
                break;
            case ER3Notify_SamplingDone:
                qDebug()<<"USB_NOTIFY_INTERRUPT_PACKET - ER3Notify_SamplingDone";
                break;
            case ER3Notify_StepperPosition:
                memcpy(&parm, notify.m_parm, sizeof (quint32));
                //motorState值含义
                //0x3E: ER3_TrayEncoder
                //0x7E: ER3_TrayEncoder | ER3_TrayMovingIn
                //0xBE: ER3_TrayEncoder | ER3_TrayMovingOut
                motorState = static_cast<quint8>(parm & 0xFF);
                logicPos = motorState & ER3_TrayPosMask;            //ER3_Position标志,说明实际运动情况
                motorPos = static_cast<short>(static_cast<int>((parm >> 16) & 0xFFFF) - 0x8000);   //实际物理电机距离原点位置，单位步数？
                motor = static_cast<quint8>((parm >> 8) & 0x03);
                status = static_cast<quint8>((parm >> 8) & 0xFC);
                movingState = (motorState & (ER3_TrayMovingIn | ER3_TrayMovingOut)) ? 1 : 0;

                qDebug()<<QString("tray %1 motorState: 0x%2, motorPos: %3, logicPos: 0x%4, movingState: 0x%5, status : 0x%6, parm = 0x%7")
                          .arg(motor).arg(motorState, 0, 16).arg(motorPos).arg(logicPos, 0, 16)
                          .arg(movingState, 0, 16).arg(status, 0, 16).arg(parm, 0, 16);

                if(movingState == STATE_MOVING)
                {
                    motorStartMoving = true;
                }
                //////////////////////////////
                ///为了当电机回到原点是，重置原点位置
                if(motorHoming == 1 && movingState == STATE_MOVING)
                {
                    motorHoming = 2;
                }
                else if(motorHoming == 2 && movingState == STATE_FINISH_MOVING)
                {
                    if(logicPos == ER3_TrayIn)
                        motorHoming = -2;
                    else
                        motorHoming = -1;

                    if(motorHoming < 0)
                    {
                        motorHoming = 0;
                        fx3USBThread->setStepperHome();
                        motorPos = 0;
                    }
                }
                /////////////////////////////

                if(movingState == STATE_MOVING)
                {
                    motorTrayPosState = TrayPos::TrayPos_Moving;
                    motorMoving = true;
                }
                else if(movingState == STATE_FINISH_MOVING)
                {
                    //移动结束,移动开始后设置的标志位motorMoving motorStartMoving为ture，status和movingState表示移动结束
                    if(motorMoving && motorStartMoving == true && (status & 0x10) == 0x0)
                    {
    //                    motorMoveControl(motorPos);
                        motorStartMoving = false;
                        if(motorPos < ER3_Position::ER3_TrayOut && motorTrayPosTarget != TrayPos::TrayPos_Home)
                        {
                            motorMoving = false;
                            motorMove(motorTrayPosTarget, STEPPER_TRAY);
                            return;
                        }

                        stepperMoveConfig.m_stepperPeriod = STEPPER_MOVE_PERIOD >> 2;
                        stepperMoveConfig.m_stallDetectEnable = STEPPER_MOVE_STALLDETECT_DISABLE;
                        fx3USBThread->setStepperConfig(stepperMoveConfig);
                        motorMoving = false;
                        motorTrayPosState = motorTrayPosTarget;
                    }
    //                if (motorMoving && motorPos - 2 <= motorTrayPosTarget && motorPos + 2 >= motorTrayPosTarget && (status & 0x01) == 0)
    //                {
    //                    // We got an end move notification without a start notification.
    //
    ////                    goto force_move_complete_t;
    //                }
                }

                break;
            case ER3Notify_ContDone:
                isBusy = false;

                qDebug()<<"notify count done";

                if(!fx3USBThread->setSamplingState(ER3_SamplingOff))
                {
                    qDebug()<<"sampling off wrong";
                    break;
                }

                fx3USBThread->transferState = INTERRUPT_TRANSFER;
                fx3USBThread->stopDataTransfer = true;

                stepperMoveConfig.m_stepperPeriod               = STEPPER_MOVE_PERIOD >> 2;
                stepperMoveConfig.m_stepperDistance             = STEPPER_MOVE_DISTANCE;
                stepperMoveConfig.m_extraSteps                  = STEPPER_MOVE_EXTRA;
                stepperMoveConfig.m_runCurrent                  = STEPPER_MOVE_IRUN;
                stepperMoveConfig.m_holdCurrent                 = STEPPER_MOVE_IHOLD;
                stepperMoveConfig.m_stallDetectEnable           = STEPPER_MOVE_STALLDETECT_DISABLE;
                stepperMoveConfig.m_stallDetectStartThreshold   = STEPPER_MOVE_STALLSTART;
                stepperMoveConfig.m_stallDetectRunThreshold     = STEPPER_MOVE_STALLRUN;
                fx3USBThread->setStepperConfig(stepperMoveConfig);

                if(!fx3USBThread->setLaser(static_cast<ER3_Laser>(0)))
                {
                    qDebug()<<"set laser 0 wrong";
                    break;
                }
                else
                {
                    qDebug()<<"set laser right";
                }

                isSampleCountRight = dealScanSample();
                sampleBuffer.clear();
                break;
            }
        }

        break;
    case USB_NOTIFY_DATA_PACKET:
        if(connected && fx3USBThread->usbDevice != nullptr)
        {
            while(fx3USBThread->dataQueue.size() != 0)
            {
                array = fx3USBThread->dataQueue.first();
                fx3USBThread->dataQueue.removeFirst();
                qDebug()<<"dataqueue.size:"<<fx3USBThread->dataQueue.size();
                if(sampleBuffer.size() + array.size() <= bufferSize)
                {
                    sampleBuffer.append(array);
                    qDebug()<<"samplebuffer size:"<<sampleBuffer.size();
                }
                else
                {
                    int len = bufferSize - sampleBuffer.size();
                    if(len > 0)
                        sampleBuffer.append(array.mid(0, len));
                    qDebug()<<"samplebuffer size:"<<sampleBuffer.size();
                }
            }
        }

        break;
    }
}

void FX3USBDevice::dealCommand()
{
    switch (scanProcessState)
    {
    case ScanProcess::scanIdle:
        break;
    case ScanProcess::scanStart:
        fx3USBThread->transferState = INTERRUPT_TRANSFER;
        fx3USBThread->stopDataTransfer = true;
        if(motorTrayPosState != TrayPos::TrayPos_Home)
        {
            if(!motorMoveHome())
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanStart);
                }
                isBusy = false;
                return;
            }
            oldDateTime = QDateTime::currentDateTime();
            scanProcessState = ScanProcess::scanWaitIn;
        }
        scanProcessState = ScanProcess::scanWaitIn;
        break;
    case ScanProcess::scanWaitIn:
        if(motorMoving)
            break;
        if(motorTrayPosState != TrayPos::TrayPos_Home)
        {
            if(oldDateTime.msecsTo(QDateTime::currentDateTime()) > OVER_TIME_COUNT)
            {
                scanProcessState = ScanProcess::scanStart;
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanStart);
                }
                isBusy = false;
                return;
            }
        }
        else
        {
            motorMove(TrayPos::TrayPos_Pre);
            scanErrorCount = 0;
            oldDateTime = QDateTime::currentDateTime();
            scanProcessState = ScanProcess::scanWaitPre;
        }
        break;
    case ScanProcess::scanWaitPre:
        if(motorMoving)
            break;
        if(motorTrayPosState != TrayPos::TrayPos_Pre)
        {
            if(oldDateTime.msecsTo(QDateTime::currentDateTime()) > OVER_TIME_COUNT)
            {
                scanProcessState = ScanProcess::scanWaitIn;
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitIn);
                }
                isBusy = false;
                return;
            }
        }
        else
        {

            //扫描时电机移动需要的参数设置
            StepperConfig stepperScanConfig;
            stepperScanConfig.m_stepperPeriod               = STEPPER_SCAN_PERIOD;
            stepperScanConfig.m_stepperDistance             = STEPPER_MOVE_DISTANCE;
            stepperScanConfig.m_extraSteps                  = STEPPER_MOVE_EXTRA;
            stepperScanConfig.m_runCurrent                  = STEPPER_MOVE_IRUN;
            stepperScanConfig.m_holdCurrent                 = STEPPER_MOVE_IHOLD;
            stepperScanConfig.m_stallDetectEnable           = STEPPER_MOVE_STALLDETECT_DISABLE;
            stepperScanConfig.m_stallDetectStartThreshold   = STEPPER_MOVE_STALLSTART;
            stepperScanConfig.m_stallDetectRunThreshold     = STEPPER_MOVE_STALLRUN;
            if(!fx3USBThread->setStepperConfig(stepperScanConfig))
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitPre);
                }
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                return;
            }
//            scanErrorCount = 0;
            //扫描器设置
            if(!fx3USBThread->setScanConfig(scanConfig))
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitPre);
                }
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                return;
            }
//            scanErrorCount = 0;
            //激光设置
            if(!fx3USBThread->setLaserConfig(laserConfig))
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitPre);
                }
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                return;
            }
//            scanErrorCount = 0;
            //激光打开
            quint8 laserSet = laserSelect;
            laserSet |= ER3_Laser1Cont | ER3_Laser2Cont;
            if(laserConfig.m_laser1PWMWidth == 0)
            {
                laserSet &= ~ER3_Laser1Cont;
                laserSet &= ~ER3_Laser1Scan;
            }
            if(laserConfig.m_laser2PWMWidth == 0)
            {
                laserSet &= ~ER3_Laser2Cont;
                laserSet &= ~ER3_Laser2Scan;
            }
            if(!fx3USBThread->setLaser(static_cast<ER3_Laser>(laserSet)))
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitPre);
                }
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                return;
            }
//            scanErrorCount = 0;
            //打开扫描采样
            if(!fx3USBThread->setSamplingState(ER3_SamplingScan))
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitPre);
                }
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                return;
            }
//            scanErrorCount = 0;

            sampleBuffer.clear();
            if(!motorMove(TrayPos::TrayPos_Mid, STEPPER_TRAY, false))
            {
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitPre);
                }
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                return;
            }
            scanErrorCount = 0;

            fx3USBThread->transferState = DATA_TRANSFER;
            fx3USBThread->dataEpt->Reset();

            isBusy = true;
            scanProcessState = ScanProcess::scanning;
            oldDateTime = QDateTime::currentDateTime();
//            fx3USBThread->stopDataTransfer = false;

        }
        break;
    case ScanProcess::scanning:
        if(isBusy)
        {
            if(oldDateTime.msecsTo(QDateTime::currentDateTime()) > OVER_TIME_COUNT)
            {
                scanProcessState = ScanProcess::scanStart;
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanning);
                }
                motorMoving = false;
                fx3USBThread->transferState = INTERRUPT_TRANSFER;
                fx3USBThread->stopDataTransfer = true;
                fx3USBThread->setStepperConfig(stepperMoveConfig);
                isBusy = false;
                return;
            }
        }
        else
        {
            scanProcessState = ScanProcess::scanDone;
        }
        break;
    case ScanProcess::scanDone:
        if(motorMoving)
            break;

        motorMove(TrayPos::TrayPos_Pre);
        scanErrorCount = 0;
        oldDateTime = QDateTime::currentDateTime();
        scanProcessState = ScanProcess::scanWaitCountFinish;
        break;
    case ScanProcess::scanWaitCountFinish:
        if(motorMoving)
            break;

        if(motorTrayPosState != TrayPos::TrayPos_Pre)
        {
            if(oldDateTime.msecsTo(QDateTime::currentDateTime()) > OVER_TIME_COUNT)
            {
                scanProcessState = ScanProcess::scanDone;
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitCountFinish);
                }
                isBusy = false;
                return;
            }
        }
        else
        {
            if(!isSampleCountRight)
            {
                scanProcessState = ScanProcess::scanStart;
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitCountFinish);
                }
                isBusy = false;
                return;
            }
            else
            {
                motorMove(TrayPos::TrayPos_Out);
                oldDateTime = QDateTime::currentDateTime();
                scanProcessState = ScanProcess::scanWaitOut;
            }
        }
        break;
    case ScanProcess::scanWaitOut:
        if(motorMoving)
            break;

        if(motorTrayPosState != TrayPos::TrayPos_Out)
        {
            if(oldDateTime.msecsTo(QDateTime::currentDateTime()) > OVER_TIME_COUNT)
            {
                scanProcessState = ScanProcess::scanWaitCountFinish;
                scanErrorCount++;
                if(scanErrorCount > 3)
                {
                    scanErrorCount = 0;
                    scanProcessState = ScanProcess::scanIdle;
                    emit sendError(ScanProcess::scanWaitCountFinish);
                }
                isBusy = false;
                return;
            }
        }
        else
        {
            scanProcessState = ScanProcess::scanIdle;
        }
        break;
    }

}
