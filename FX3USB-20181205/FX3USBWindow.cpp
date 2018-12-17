#include "FX3USBWindow.h"
#include "ui_FX3USBWindow.h"

FX3USBWindow::FX3USBWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FX3USBWindow)
{
    ui->setupUi(this);

    initUSBDevice();
}

FX3USBWindow::~FX3USBWindow()
{
    delete fx3USBDevice;
    delete ui;
}

void FX3USBWindow::initUSBDevice()
{
    fx3USBDevice = new FX3USBDevice();
}

void FX3USBWindow::on_pushButton_clicked()
{
    if(fx3USBDevice->openDeviceDriver())
    {
        qDebug()<<"open right usb";
    }
    else
    {
        qDebug()<<"open wrong usb";
    }
}

void FX3USBWindow::on_pushButton_2_clicked()
{
    if(fx3USBDevice->closeDeviceDriver())
    {
        qDebug()<<"close device usb right";
    }
    else
    {
        qDebug()<<"close device usb wrong";
    }

}

void FX3USBWindow::on_pushButton_3_clicked()
{
//    StepperConfig stepperConfig;
//    stepperConfig.m_stepperPeriod = 200;
//    stepperConfig.m_stepperDistance = 8191;
//    stepperConfig.m_extraSteps = 12;
//    stepperConfig.m_runCurrent = ER3_Current100QuarterStep;
//    stepperConfig.m_holdCurrent = 1;
//    stepperConfig.m_stallDetectEnable = 1;
//    stepperConfig.m_stallDetectStartThreshold = 6;
//    stepperConfig.m_stallDetectRunThreshold = 6;
//    fx3USBDevice->fx3USBThread->setStepperConfig(stepperConfig);
////    fx3USBDevice->fx3USBThread->moveStepper(ER3_TrayIn, STEPPER_TRAY, 0);
//    int stepsIndex = 33000;
//    fx3USBDevice->fx3USBThread->moveStepper(ER3_TrayEncoder, STEPPER_TRAY, 0);

//    fx3USBDevice->motorMoveHome();
    fx3USBDevice->motorMove(TrayPos::TrayPos_Home);
}

//void FX3USBWindow::on_pushButton_4_clicked()
//{
//    fx3USBDevice->fx3USBThread->setStepperHome(STEPPER_TRAY);
//}

void FX3USBWindow::on_pushButton_5_clicked()
{
//    bool ret = fx3USBDevice->fx3USBThread->setStepperConfig(fx3USBDevice->stepperConfig);
//    fx3USBDevice->motorTrayPosState = 0;
//    fx3USBDevice->motorTrayPosState = TrayPos::TrayPos_Mid;
    int pos = fx3USBDevice->motorPosition[fx3USBDevice->motorTrayPosState];
    if(pos + 1000 <= ENC_LARGE)
    {
        pos += 1000;
        fx3USBDevice->stepperMoveConfig.m_stallDetectEnable = STEPPER_MOVE_STALLDETECT_DISABLE;
        fx3USBDevice->fx3USBThread->setStepperConfig(fx3USBDevice->stepperMoveConfig);
        fx3USBDevice->motorPosition[fx3USBDevice->motorTrayPosState] = pos;
        fx3USBDevice->motorMoveAbs(pos, STEPPER_TRAY);
        fx3USBDevice->motorTrayPosTarget = fx3USBDevice->motorTrayPosState;
    }
}

void FX3USBWindow::on_pushButton_7_clicked()
{
//    bool ret = fx3USBDevice->fx3USBThread->setStepperConfig(fx3USBDevice->stepperConfig);
//    fx3USBDevice->motorTrayPosState = 0;
//    fx3USBDevice->motorTrayPosState = TrayPos::TrayPos_Mid;
    int pos = fx3USBDevice->motorPosition[fx3USBDevice->motorTrayPosState];
    if(pos - 1000 >= ENC_ZERO)
    {
        pos -= 1000;
        fx3USBDevice->stepperMoveConfig.m_stallDetectEnable = STEPPER_MOVE_STALLDETECT_DISABLE;
        fx3USBDevice->fx3USBThread->setStepperConfig(fx3USBDevice->stepperMoveConfig);
        fx3USBDevice->motorPosition[fx3USBDevice->motorTrayPosState] = pos;
        fx3USBDevice->motorMoveAbs(pos, STEPPER_TRAY);
        fx3USBDevice->motorTrayPosTarget = fx3USBDevice->motorTrayPosState;
    }
}

void FX3USBWindow::on_pushButton_8_clicked()
{
    fx3USBDevice->motorMove(TrayPos::TrayPos_Mid);
}

void FX3USBWindow::on_pushButton_10_clicked()
{
    fx3USBDevice->motorMove(TrayPos::TrayPos_In);
}

void FX3USBWindow::on_pushButton_9_clicked()
{
    fx3USBDevice->motorMove(TrayPos::TrayPos_Out);
}

void FX3USBWindow::on_pushButton_4_clicked()
{
    fx3USBDevice->scanProcessState = ScanProcess::scanStart;
}
