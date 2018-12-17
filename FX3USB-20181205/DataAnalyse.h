#ifndef DATAANALYSE_H
#define DATAANALYSE_H

#include <QObject>
#include <QtMath>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QDateTime>
#include <QByteArray>

///////////////////////////////////////
//分析采样数据，解析数据，计算结果
#define RAW_BUFFER_COUNT            266240      //获取的原始数据点数
#define RAW_ONE_CHANNEL_COUNT       66560       //原始单通道数据点数
#define RAW_CHANNEL_COUNT           4           //原始数据通道数
//////////////////////////////////////////////////////
//解析数据
/* Descramble input channels into expected order. */
const int m_chan_map[4] = {0, 2, 3, 1};
/* Map input channels to analysis channels. */
const int m_data_map[4] = {1, 1, 2, 3};

struct OutData
{
    float m_v[4][3];
};

class DataAnalyse : public QObject
{
    Q_OBJECT
public:
    explicit DataAnalyse(QObject *parent = nullptr);
    DataAnalyse(QByteArray buffer);
    ~DataAnalyse();

    QByteArray sampleBuffer;
    //数据缩放比例
    float dataScale[4] = {1.0, 1.0, 1.0, 1.0};
    //原始数据一共266240点，为4*66560，排列顺序为通道0第1点，通道1第1点，通道2第1点，通道3第1点，通道0第2点，通道1第2点，通道2第2点，通道3第2点，类推
    float *rawData;
    OutData outData[RAW_ONE_CHANNEL_COUNT / 4 + 256];
    int dataBuff;
    bool extractRawData();
    bool demodulateLaserData();

signals:

public slots:
};

#endif // DATAANALYSE_H
