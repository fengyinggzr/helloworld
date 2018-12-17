#include "FX3USBThread.h"

FX3USBThread::FX3USBThread(QObject *parent) :
    QThread(parent)
{
    started = false;
    transferState = INTERRUPT_TRANSFER;
    count = 0;
}

FX3USBThread::~FX3USBThread()
{

}

/**
 * @brief FX3USBThread::sendCommand
 * @param direction DIR_FROM_DEVICE 或者 DIR_TO_DEVICE，说明数据方向，发送数据还是接受数据
 * @param reqCode 从宏定义中获取的R3_ vendor request commands，说明该请求的具体命令码
 * @param value 根据请求变化值
 * @param index 根据请求变化值, 通常用于传递索引或偏移量。
 * @param data 传输的数据
 * @param length 传输的数据长度
 * @return
 */
bool FX3USBThread::sendCommand(/*CTL_XFER_TGT_TYPE target,  //target 目标值，值为TGT_DEVICE, TGT_INTFC, TGT_ENDPT and TGT_OTHER其中一个，基本为TGT_DEVICE
                               CTL_XFER_REQ_TYPE reqType,*/ //reqType 请求类型，从REQ_STD, REQ_CLASS and REQ_VENDOR中选择，基本为REQ_VENDOR
                               CTL_XFER_DIR_TYPE direction,
                               quint8 reqCode,
                               quint16 value,
                               quint16 index,
                               quint8 *data, LONG &length)
{
    usbDevice->ControlEndPt->TimeOut = 1000;
    usbDevice->ControlEndPt->Target = TGT_DEVICE;
    usbDevice->ControlEndPt->ReqType = REQ_VENDOR;
    usbDevice->ControlEndPt->Direction = direction;
    usbDevice->ControlEndPt->ReqCode = reqCode;
    usbDevice->ControlEndPt->Value = value;
    usbDevice->ControlEndPt->Index = index;
    if(!usbDevice->ControlEndPt->XferData(data, length))
    {
        qDebug()<<"xferdata wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::bogus()
{
    quint8 buff[64];
    LONG length = 0;
    if(!sendCommand(DIR_FROM_DEVICE,
                   R3_BOGUS,
                   0x0000,
                   0x0000,
                   buff, length))
    {
        qDebug()<<"bogus wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::resetDevice()
{
    quint8 buff[64];
    LONG length = 0;
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_RESET,
                    0x0000,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"reset device wrong";
        return false;
    }

    if(!setNotifications(0))
    {
        qDebug()<<"set notification wrong";
        return false;
    }
    return true;
}

////////////////////////////////////////////////

bool FX3USBThread::getR3Info(SR3Info &info)
{
    quint8 buff[64];
    LONG length = sizeof(SR3Info);
    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_INFO,
                    0x0000,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"get info wrong";
        return false;
    }

    memcpy(&info, buff, sizeof(SR3Info));

    return true;
}

bool FX3USBThread::getR3ScanConfig(SR3ScanConfig &scanConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3ScanConfig);
    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_SCAN_CONFIG,
                    0x0000,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"get scanConfig wrong";
        return false;
    }

    memcpy(&scanConfig, buff, sizeof(SR3ScanConfig));
    return true;
}

bool FX3USBThread::setR3ScanConfig(SR3ScanConfig scanConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3ScanConfig);
    memcpy(buff, &scanConfig, sizeof(SR3ScanConfig));
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_SCAN_CONFIG,
                    0x0000,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"set scanconfig wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::getR3StepperConfig(SR3StepperConfig &stepperConfig, quint16 motor)
{
    quint8 buff[64];
    LONG length = sizeof(SR3StepperConfig);

    if(motor != STEPPER_TRAY)
    {
        qDebug()<<"get stepper config wrong motor";
        return false;
    }

    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_STEPPER_CONFIG,
                    static_cast<quint16>(motor << 8),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"get stepperconfig wrong";
        return false;
    }
    memcpy(&stepperConfig, buff, sizeof(SR3StepperConfig));
    return true;
}

bool FX3USBThread::setR3StepperConfig(SR3StepperConfig stepperConfig, quint16 motor)
{
    quint8 buff[64];
    LONG length = sizeof(SR3StepperConfig);
    if(motor != STEPPER_TRAY)
    {
        return false;
    }

    memcpy(buff, &stepperConfig, sizeof(SR3StepperConfig));
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_STEPPER_CONFIG,
                    static_cast<quint16>(motor << 8),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"setR3StepperConfig false";
        return false;
    }
    return true;
}

bool FX3USBThread::getR3TempConfig(quint8 which, SR3TempConfig &tempConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3TempConfig);

    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_TEMPERATURE_CONFIG,
                    static_cast<quint16>(which << 8),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"get tempconfig wrong";
        return false;
    }

    memcpy(&tempConfig, buff, sizeof(SR3TempConfig));
    return true;
}

bool FX3USBThread::setR3TempConfig(quint8 which, SR3TempConfig tempConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3TempConfig);
    memcpy(buff, &tempConfig, sizeof(SR3TempConfig));
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_TEMPERATURE_CONFIG,
                    static_cast<quint16>(which << 8),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"set tempconfig wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::getR3CapsensorConfig(quint8 which, SR3CapsensorConfig &capsensorConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3CapsensorConfig);

    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_CAPACITANCE_CONFIG,
                    static_cast<quint16>(which << 8),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"get capsensorconfig wrong";
        return false;
    }

    memcpy(&capsensorConfig, buff, sizeof (SR3CapsensorConfig));
    return true;
}

bool FX3USBThread::setR3CapsensorConfig(quint8 which, SR3CapsensorConfig capsensorConfig)
{
    quint8 buff[64];
    LONG length = sizeof (SR3CapsensorConfig);

    memcpy(buff, &capsensorConfig, sizeof(SR3CapsensorConfig));
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_CAPACITANCE_CONFIG,
                    static_cast<quint16>(which << 8),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"set capsensorconfig wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::getR3LaserConfig(SR3LaserConfig &laserConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3LaserConfig);

    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_LASER_CONFIG,
                    0x0000,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"get capsensorconfig wrong";
        return false;
    }

    memcpy(&laserConfig, buff, sizeof (SR3LaserConfig));
    return true;
}

bool FX3USBThread::setR3LaserConfig(SR3LaserConfig laserConfig)
{
    quint8 buff[64];
    LONG length = sizeof(SR3LaserConfig);
    memcpy(buff, &laserConfig, sizeof(SR3LaserConfig));
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_LASER_CONFIG,
                    0x0000,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"set capsensorconfig wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::readR3VIDandPID(SR3VidAndPid &vidAndPid)
{
    quint8 buff[64];
    LONG length = sizeof (SR3VidAndPid);
    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_EEPROM,
                    0x0000,
                    0xCAFE,
                    buff, length))
    {
        qDebug()<<"read vid and pid wrong";
        return false;
    }
    memcpy(&vidAndPid, buff, sizeof (SR3VidAndPid));
    return true;
}

//////////////////////////////////////////////////////

bool FX3USBThread::getInfo(Info &info)
{
    SR3Info r3Info;
    if(!getR3Info(r3Info))
    {
        return false;
    }
    info.getConfig(r3Info);
    return true;
}

bool FX3USBThread::getScanConfig(ScanConfig &scanConfig)
{
    SR3ScanConfig r3ScanConfig;
    if(!getR3ScanConfig(r3ScanConfig))
    {
        return false;
    }
    scanConfig.getConfig(r3ScanConfig);
    return true;
}

bool FX3USBThread::setScanConfig(ScanConfig scanConfig)
{
    SR3ScanConfig r3ScanConfig;
    scanConfig.setConfig(r3ScanConfig);
    if(!setR3ScanConfig(r3ScanConfig))
    {
        return false;
    }
    return true;
}

bool FX3USBThread::getStepperConfig(StepperConfig &stepperConfig, quint16 motor)
{
    SR3StepperConfig r3StepperConfig;
    if(!getR3StepperConfig(r3StepperConfig, motor))
    {
        return false;
    }
    stepperConfig.getConfig(r3StepperConfig);
    return true;
}

bool FX3USBThread::setStepperConfig(StepperConfig stepperConfig, quint16 motor)
{
    SR3StepperConfig r3StepperConfig;
    stepperConfig.setConfig(r3StepperConfig);
    if(!setR3StepperConfig(r3StepperConfig, motor))
    {
        return false;
    }
    return true;
}

bool FX3USBThread::getTempConfig(quint8 which, TempConfig &tempConfig)
{
    SR3TempConfig r3TempConfig;
    if(!getR3TempConfig(which, r3TempConfig))
    {
        return false;
    }
    tempConfig.getConfig(r3TempConfig);
    return true;
}

bool FX3USBThread::setTempConfig(quint8 which, TempConfig tempConfig)
{
    SR3TempConfig r3TempConfig;
    tempConfig.setConfig(r3TempConfig);
    if(setR3TempConfig(which, r3TempConfig))
    {
        return false;
    }
    return true;
}

bool FX3USBThread::getCapsensorConfig(quint8 which, CapsensorConfig &capsensorConfig)
{
    SR3CapsensorConfig r3CapsensorConfig;
    if(!getR3CapsensorConfig(which, r3CapsensorConfig))
    {
        return false;
    }
    capsensorConfig.getConfig(r3CapsensorConfig);
    return true;
}

bool FX3USBThread::setCapsensorConfig(quint8 which, CapsensorConfig capsensorConfig)
{
    SR3CapsensorConfig r3CapsensorConfig;
    capsensorConfig.setConfig(r3CapsensorConfig);
    if(!setR3CapsensorConfig(which, r3CapsensorConfig))
    {
        return false;
    }
    return true;
}

bool FX3USBThread::getLaserConfig(LaserConfig &laserConfig)
{
    SR3LaserConfig r3LaserConfig;
    if(!getR3LaserConfig(r3LaserConfig))
    {
        return false;
    }
    laserConfig.getConfig(r3LaserConfig);
    return true;
}

bool FX3USBThread::setLaserConfig(LaserConfig laserConfig)
{
    SR3LaserConfig r3LaserConfig;
    laserConfig.setConfig(r3LaserConfig);
    if(!setR3LaserConfig(r3LaserConfig))
    {
        return false;
    }
    return true;
}

bool FX3USBThread::readVIDAndPID(VidAndPid &vidAndPid)
{
    SR3VidAndPid r3VidandPid;
    if(!readR3VIDandPID(r3VidandPid))
    {
        return false;
    }
    vidAndPid.getConfig(r3VidandPid);
    return true;
}

////////////////////////////////////////////

bool FX3USBThread::setNotifications(quint16 notifyBits)
{
    quint8 buff[64];
    LONG length = 0;

    if(!sendCommand(DIR_TO_DEVICE,
                   R3_NOTIFICATIONS,
                   notifyBits,
                   0x0000,
                   buff, length))
    {
        qDebug()<<"set notification wrong";
        return false;
    }
    return true;
}

///
/// \brief FX3USBThread::requestEventResult
/// 请求相关事件通知结果（单次，一对一）
/// \param event
/// \return
///
bool FX3USBThread::requestEventResult(quint16 event)
{
    quint8 buff[64];
    LONG length = 0;

    if(!sendCommand(DIR_TO_DEVICE,
                   R3_SEND_EVENT,
                   event,
                   0x0000,
                   buff, length))
    {
        qDebug()<<"sendEvent wrong";
        return false;
    }
    return true;

}

bool FX3USBThread::readEEPROM(quint16 addr, quint8 len, quint8 *data)
{
    quint8 buff[64];
    LONG length = len;
    if(len == 0 || len > 64)
    {
        qDebug()<<"CReliaFx3USB::USB_ReadEEPROM: Error - length must be 1..64";
        return false;
    }

    if(addr + len > EEPROM_SIZE_BYTES - EEPROM_USER_OFFSET)
    {
        qDebug()<<QString("CReliaFx3USB::USB_ReadEEPROM: Error - addr out of range 0..%1-len").arg(QString::number(EEPROM_SIZE_BYTES - EEPROM_USER_OFFSET));
        return false;
    }

    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_EEPROM,
                    addr,
                    0xCAFE,
                    buff, length))
    {
        qDebug()<<"read eeprom wrong";
        return false;
    }
    memcpy(data, buff, static_cast<quint32>(length) * sizeof(quint8));

    return true;
}

bool FX3USBThread::writeEEPROM(quint16 addr, quint8 len, quint8 *data)
{
    quint8 buff[64];
    LONG length = len;
    if(len == 0 || len > 64)
    {
        qDebug()<<"CReliaFx3USB::USB_WriteEEPROM: Error - length must be 1..64";
        return false;
    }

    if(addr + len > EEPROM_SIZE_BYTES - EEPROM_USER_OFFSET)
    {
        qDebug()<<QString("CReliaFx3USB::USB_WriteEEPROM: Error - addr out of range 0..%1-len").arg(QString::number(EEPROM_SIZE_BYTES - EEPROM_USER_OFFSET));
        return false;
    }

    memcpy(buff, data, len * sizeof(quint8));
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_EEPROM,
                    addr,
                    0xCFAE,
                    buff, length))
    {
        qDebug()<<"write eeprom wrong";
        return false;
    }

    return true;
}

//FPGA读取
bool FX3USBThread::peek(quint16 addr, quint8 &value)
{
    quint8 buff[64];
    LONG length = 1;
    if(!sendCommand(DIR_FROM_DEVICE,
                    R3_PEEK_POKE,
                    0,
                    addr,
                    buff, length))
    {
        qDebug()<<"peek wrong";
        return false;
    }
    value = buff[0];
    return true;
}

//FPGA写入
bool FX3USBThread::poke(quint16 addr, quint8 value)
{
    quint8 buff[64];
    LONG length = 0;
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_PEEK_POKE,
                    value,
                    addr,
                    buff, length))
    {
        qDebug()<<"poke wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::setLaser(ER3_Laser laserSet)
{
    quint8 buff[64];
    LONG length = 0;
    if ( laserSet < 0 || laserSet > 15)
    {
        qDebug()<<"CReliaFx3USB::USB_SetLaser: Error - lset out of range 0..15";
        return false;
    }
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_LASER_ON,
                    static_cast<quint16>(laserSet),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"set laser wrong";
        return false;
    }

    return true;
}

bool FX3USBThread::moveStepper(ER3_Position pos, quint16 motor, int index)
{
    quint8 buff[64];
    LONG length = 0;
    if(motor != STEPPER_TRAY)
    {
        return false;
    }
    if (pos != ER3_TrayIn && pos != ER3_TrayEncoder)
    {
        qDebug()<<QString("CReliaFx3USB::USB_MoveStepper: Tray %1 Error - Position %2 not allowed").arg(motor).arg(pos);
        return false;
    }
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_STEPPER_MOVE,
                    static_cast<quint16>((motor << 8 ) | pos),
                    static_cast<quint16>(index),
                    buff, length))
    {
        return false;
    }
    return true;
}

/**
 * @brief FX3USBThread::setStepperHome
 * 设置当前电机位置为原点位值
 * @param motor
 * @return
 */
bool FX3USBThread::setStepperHome(quint16 motor)
{
    quint8 buff[64];
    LONG length = 0;
    if(motor != STEPPER_TRAY)
    {
        return false;
    }
    if(!sendCommand(DIR_TO_DEVICE,
                    R3_STEPPER_SET_HOME,
                    motor,
                    0x0000,
                    buff, length))
    {
        qDebug()<<"set stepper home wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::setSamplingState(ER3_Sampling samplingState)
{
    quint8 buff[64];
    LONG length = 0;

    if(!sendCommand(DIR_TO_DEVICE,
                    R3_SAMPLING_ON,
                    static_cast<quint16>(samplingState),
                    0x0000,
                    buff, length))
    {
        qDebug()<<"setSamplingState wrong";
        return false;
    }
    return true;
}

bool FX3USBThread::openDeviceDriver()
{
    closeDeviceDriver();

    usbDevice = new CCyUSBDevice();
    if(usbDevice == nullptr)
    {
        qDebug()<<"open usbdevice wrong";
        return false;
    }
    else
    {
        int deviceCount = usbDevice->DeviceCount();
        unsigned char id;
        for(id = 0; id < deviceCount; id++)
        {
            usbDevice->Open(id);
            if(usbDevice->VendorID == VENDOR_ID && usbDevice->ProductID == PRODUCT_ID)
                break;
            if(usbDevice->VendorID == OLD_VENDOR_ID && usbDevice->ProductID == OLD_PRODUCT_ID)
                break;
        }

        int endPointCount = usbDevice->EndPointCount();
        for(int endNumber = 1; endNumber < endPointCount; endNumber++)
        {
            switch (usbDevice->EndPoints[endNumber]->Address)
            {
            case 0x81:
                interruptEpt = reinterpret_cast<CCyInterruptEndPoint *>(usbDevice->EndPoints[endNumber]);
                break;
            case 0x86:
                dataEpt = reinterpret_cast<CCyBulkEndPoint *>(usbDevice->EndPoints[endNumber]);
                break;
            default:
                break;
            }
        }

        if(interruptEpt == nullptr || dataEpt == nullptr)
        {
            qDebug()<<"wrong create work";
            return false;
        }
    }

    startThread();
    this->start();

    sendNotification(USB_NOTIFY_CONNECTED, 0);

    return true;
}

bool FX3USBThread::closeDeviceDriver()
{
    if(started)
    {
        stopThread();
        while(isRunning);
    }
    if(usbDevice != nullptr)
    {
//        usbDevice->Close();
        delete usbDevice;
//        delete usbDevice;
        usbDevice = nullptr;
        interruptEpt = nullptr;
        dataEpt = nullptr;
    }
    return true;
}

void FX3USBThread::startThread()
{
    started = true;
}

void FX3USBThread::stopThread()
{
    started = false;
}

void FX3USBThread::run()
{
    quint8 interruptBuff[INTERRUPT_BUFF_SIZE];
    quint8 dataBuff[DATA_BUFF_SIZE];
    LONG interruptLength = INTERRUPT_BUFF_SIZE * sizeof (quint8);
    LONG dataLength = DATA_BUFF_SIZE * sizeof (quint8);

    OVERLAPPED interruptOverlap;
    OVERLAPPED dataOverlap;
    memset(&interruptOverlap, 0, sizeof (OVERLAPPED));
    memset(&dataOverlap, 0, sizeof (OVERLAPPED));
    interruptOverlap.hEvent = CreateEventA(nullptr, false, false, "RELIA3_INTERRUPT_EP");
    dataOverlap.hEvent = CreateEventA(nullptr, false, false, "RELIA3_DATA_EP");

    bool doingInterruptTransfer = false;
    bool doingDataTransfer = false;

    quint8 *interruptContext = nullptr;
    quint8 *dataContext = nullptr;

    isRunning = true;
    stopDataTransfer = false;

    while(started)
    {
        QThread::msleep(10);

        if(transferState == INTERRUPT_TRANSFER && !doingInterruptTransfer)
        {
            doingInterruptTransfer = true;
            ResetEvent(interruptOverlap.hEvent);
            interruptContext = interruptEpt->BeginDataXfer(interruptBuff, interruptLength, &interruptOverlap);
        }
        if(transferState == DATA_TRANSFER && !doingDataTransfer)
        {
            doingDataTransfer = true;
            ResetEvent(dataOverlap.hEvent);
            dataContext = dataEpt->BeginDataXfer(dataBuff, dataLength, &dataOverlap);
            qDebug()<<"dataept BeginDataXfer ntstatus:"<<interruptEpt->NtStatus<<" usbdstatus:"<<interruptEpt->UsbdStatus;
        }

        if(doingInterruptTransfer)
        {
            bool ret = interruptEpt->WaitForXfer(&interruptOverlap, 10);
            if(ret)
            {
                doingInterruptTransfer = false;
                interruptEpt->FinishDataXfer(interruptBuff, interruptLength, &interruptOverlap, interruptContext);
                ResetEvent(interruptOverlap.hEvent);
                if(interruptLength)
                {
                    QByteArray array;
                    array.append(reinterpret_cast<char *>(interruptBuff), interruptLength);
//                    qDebug()<<"interrupt finish("<<QString::number(interruptLength)<<"):"<<array.toHex();
                    interruptQueue.append(array);
//                    qDebug()<<"size:"<<interruptQueue.size();
                    emit sendNotification(USB_NOTIFY_INTERRUPT_PACKET, 0);
                }
            }
        }

        if(doingDataTransfer)
        {
            bool ret = dataEpt->WaitForXfer(&dataOverlap, 0);
            qDebug()<<"dataept WaitForXfer ntstatus:"<<interruptEpt->NtStatus<<" utbdstatus:"<<interruptEpt->UsbdStatus;
            if(ret)
            {
                doingDataTransfer = false;
                dataEpt->FinishDataXfer(dataBuff, dataLength, &dataOverlap, dataContext);
                ResetEvent(dataOverlap.hEvent);
                qDebug()<<"dataept FinishDataXfer resetevent";
                if(dataLength)
                {
                    QByteArray array;
                    array.append(reinterpret_cast<char *>(dataBuff), dataLength);
                    qDebug()<<"datalength:"<<dataLength;
                    qDebug()<<"dataqueue thread length:"<<dataQueue.size();
                    dataQueue.append(array);
                    qDebug()<<"size:"<<array.size();
                    if(dataLength != DATA_BUFF_SIZE * sizeof(quint8))
//                    if(count >= 16)
                    {
                        count = 0;
                        transferState = INTERRUPT_TRANSFER;
                        array.clear();
                        array.resize(8);
                        array[0] = ER3Notify_ContDone & 0xFF;
                        array[1] = (ER3Notify_ContDone >> 8 ) & 0xFF;
                        interruptQueue.append(array);

                        emit sendNotification(USB_NOTIFY_DATA_PACKET, 0);
                        emit sendNotification(USB_NOTIFY_INTERRUPT_PACKET, 0);

                    }
                    else
                    {
                        count++;
                        emit sendNotification(USB_NOTIFY_DATA_PACKET, 0);
                    }
                }
            }
        }
        if(stopDataTransfer)
        {
            stopDataTransfer = false;
            if(doingDataTransfer)
            {
                transferState = INTERRUPT_TRANSFER;
                doingDataTransfer = false;
                dataEpt->FinishDataXfer(dataBuff, dataLength, &dataOverlap, dataContext);
            }
        }
    }

    if(doingInterruptTransfer)
    {
        interruptEpt->Abort();
        interruptEpt->FinishDataXfer(interruptBuff, interruptLength, &interruptOverlap, interruptContext);
    }
    if(doingDataTransfer)
    {
        dataEpt->Abort();
        dataEpt->FinishDataXfer(dataBuff, dataLength, &dataOverlap, dataContext);

    }
    if(interruptOverlap.hEvent)
    {
        CloseHandle(interruptOverlap.hEvent);
        interruptOverlap.hEvent = nullptr;
    }
    if(dataOverlap.hEvent)
    {
        CloseHandle(dataOverlap.hEvent);
        dataOverlap.hEvent = nullptr;
    }
    isRunning = false;
}
