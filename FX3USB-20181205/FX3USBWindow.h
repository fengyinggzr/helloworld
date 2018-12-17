#ifndef FX3USBWINDOW_H
#define FX3USBWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <FX3USBDevice.h>

namespace Ui {
class FX3USBWindow;
}

class FX3USBWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FX3USBWindow(QWidget *parent = nullptr);
    ~FX3USBWindow();

public:
    FX3USBDevice *fx3USBDevice;
    void initUSBDevice();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::FX3USBWindow *ui;
};

#endif // FX3USBWINDOW_H
