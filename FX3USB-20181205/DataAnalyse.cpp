#include "DataAnalyse.h"

DataAnalyse::DataAnalyse(QObject *parent) :
    QObject(parent)
{

}

DataAnalyse::DataAnalyse(QByteArray buffer)
{
    sampleBuffer = buffer;

}

DataAnalyse::~DataAnalyse()
{

}


bool DataAnalyse::extractRawData()
{
    int i,j;
    int channel;
//    int oneChannelSampleCount = RAW_ONE_CHANNEL_COUNT;
//    char *buffer = sampleBuffer.data();
    float resv[4];

    rawData = new float[RAW_BUFFER_COUNT];
    memset(rawData, 0, RAW_BUFFER_COUNT * sizeof (float));

    QString fileName = QString("./rawdata-%1.csv").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss"));
    QFile *file = new QFile(fileName);
    if(!file->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        return false;
    }
//    else
//    {
        QTextStream out(file);
//        for(int i = 0; i < RAW_ONE_CHANNEL_COUNT / RAW_CHANNEL_COUNT; i++)
//        {
//            out << outData[i].m_v[1][0] << "," << outData[i].m_v[1][1] << "," << outData[i].m_v[1][2] << ","
//                << outData[i].m_v[3][0] << "," << outData[i].m_v[3][1] << "," << outData[i].m_v[3][2] << "\r\n";
//        }
//        file->close();
//    }

    float *tempRawData = rawData;
    quint8 *buff = reinterpret_cast<quint8 *>(sampleBuffer.data());
    for(i = 0; i < RAW_ONE_CHANNEL_COUNT; i++)
    {
        for(j = 0; j < RAW_CHANNEL_COUNT; j++)
        {
            channel = m_chan_map[j];
            if(channel == 1 | channel == 3)
            {
                qint32 value = (buff[2] << 16) | (buff[1] << 8) | buff[0];
                if(value & 0x00800000) //负数
                {
                    value |= 0xFF000000;
                }

                if(j != 0)
                {
                    if(value > static_cast<qint32>(0x007fff00) || value < static_cast<qint32>(0xff800100))  //数据超范围
                    {
                        /////////////////////////////////
                        //重新读数
//                        return false;
                    }
                }

                resv[channel] = static_cast<float>(value) / (0x00800000 / 10);
                out << resv[channel]  << ",";
            }
            buff += 3;
        }
        out << "\r\n";

        for(j = 0; j < RAW_CHANNEL_COUNT; j++)
        {
            channel = m_data_map[j];
            if(channel == 1 || channel == 3)
            {
                tempRawData[j] = resv[channel] * dataScale[j];
            }
        }
        tempRawData += 4;
    }

    file->close();
    demodulateLaserData();


    return true;
}

bool DataAnalyse::demodulateLaserData()
{

    //生成 3kHz 频率的调幅数据
    double sin32[32];
    double wt32 = 0.0;
    memset(sin32, 0, 32 * sizeof (double));
    for (int i = 0; i < 32; i++)
    {
        if (i < 16)
        {
            sin32[i] = 1.0 + qCos(M_PI * (i + 8) / 8.0);
            wt32 += sin32[i];
        }
        else
        {
            sin32[i] = -sin32[i - 16];
        }
    }


//   rawData 通道1数据 与 3kHz 调幅数据同步
    size_t idx = 0;
    size_t offset1 = 0;
    double sigMax1 = -1E6;
    for (idx = 0; idx < 32; idx++)
    {
        int f2 = static_cast<int>(idx);
        double sumLow = 0.0;
        double sumHigh = 0.0;
        for (int i = 0; i < RAW_BUFFER_COUNT; i += 4)
        {
            int t1 = f2 & 0x1F;
            if (t1 >= 1 && t1 <= 16)
            {
                sumHigh += static_cast<double>(rawData[i + 1]) * sin32[t1];
            }
            else
            {
                sumLow -= static_cast<double>(rawData[i + 1]) * sin32[t1];
            }
            f2++;
        }
        double sig = sumHigh - sumLow;
        if (sig > sigMax1)
        {
            sigMax1 = sig;
            offset1 = idx;
        }
    }

//   rawData 通道3数据 与 3kHz 调幅数据同步
    size_t offset3 = 0;
    double sigMax3 = -1E6;
    for (idx = 0; idx < 32; idx++)
    {
        int f2 = static_cast<int>(idx);
        double sumLow = 0.0;
        double sumHigh = 0.0;
        for (int i = 0; i < RAW_BUFFER_COUNT; i += 4)
        {
            int t1 = f2 & 0x1F;
            if (t1 >= 1 && t1 <= 16)
            {
                sumHigh += static_cast<double>(rawData[i + 3]) * sin32[t1];
            }
            else
            {
                sumLow -= static_cast<double>(rawData[i + 3]) * sin32[t1];
            }
            f2++;
        }
        double sig = sumHigh - sumLow;
//		Trace("Ch 3 idx=%d, sig=%g\n", idx, sig);
        if (sig > sigMax3)
        {
            sigMax3 = sig;
            offset3 = idx;
        }
    }

    //////////////////////////////////////////////
    ///半周期同步解调半周期数据
    // Local demod buffers.（局部变量 解调器缓冲）
    double demod_pos[2] = { 0.0 };
    double demod_neg[2] = { 0.0 };
    double demod_pos_out_old[2] = { 0.0 };
    double demod_neg_out_old[2] = { 0.0 };
    double demod_pos_out[2] = { 0.0 };
    double demod_neg_out[2] = { 0.0 };

    size_t f2 = offset1;
    size_t f4 = offset3;

    if (sigMax1 > 0.0 && sigMax1 > sigMax3 && sigMax3 < 100.0)
    {
        f4 = offset1;
    }

    //bool flip1 = false;
    bool flip2 = false;
    //bool flip3 = false;
    bool flip4 = false;

    memset(outData, 0, sizeof (OutData) * (RAW_ONE_CHANNEL_COUNT / 4 + 256));

    double vmax1 = 0.0;
    double vmax2 = 0.0;
    size_t counter = 0;
    size_t index = 0;
    int t1;

    for (int i = 0; i < RAW_BUFFER_COUNT; i += 4)
    {
        t1 = f2 & 0x1F;
        if (!(t1 & 0x10))
        {
            if (!flip2)
            {
                flip2 = true;
                demod_pos_out_old[0] = demod_pos_out[0];
                demod_neg_out_old[0] = demod_neg_out[0];
                demod_pos_out[0] = demod_pos[0] / wt32;
                demod_neg_out[0] = demod_neg[0] / wt32;
                demod_pos[0] = 0.0;
                demod_neg[0] = 0.0;
            }
            demod_pos[0] += static_cast<double>(rawData[i + 1]) * sin32[t1 & 0x0F];
        }
        else
        {
            if (flip2)
            {
                flip2 = false;
            }
            demod_neg[0] += static_cast<double>(rawData[i + 1]) * sin32[t1 & 0x0F];
        }
        t1 = f4 & 0x1F;
        if (!(t1 & 0x10))
        {
            if (!flip4)
            {
                flip4 = true;
                demod_pos_out_old[1] = demod_pos_out[1];
                demod_neg_out_old[1] = demod_neg_out[1];
                demod_pos_out[1] = demod_pos[1] / wt32;
                demod_neg_out[1] = demod_neg[1] / wt32;
                demod_pos[1] = 0.0;
                demod_neg[1] = 0.0;
            }
            demod_pos[1] += static_cast<double>(rawData[i + 3]) * sin32[t1 & 0x0F];
        }
        else
        {
            if (flip4)
            {
                flip4 = false;
            }
            demod_neg[1] += static_cast<double>(rawData[i + 3]) * sin32[t1 & 0x0F];
        }
        f2++;
        f4++;
        if ((counter & 0x3) == 0)
        {
            //m_pCleanData[m_doAnalyze][index].m_v[0][0] = (float)((demod_neg_out[0] + demod_neg_out_old[0]) / 2);
            //m_pCleanData[m_doAnalyze][index].m_v[0][1] = (float)((demod_pos_out[0] + demod_pos_out_old[0]) / 2);
            //m_pCleanData[m_doAnalyze][index].m_v[0][2] = m_pCleanData[m_doAnalyze][index].m_v[0][1] - m_pCleanData[m_doAnalyze][index].m_v[0][0];

            outData[index].m_v[1][0] = static_cast<float>((demod_neg_out[0] + demod_neg_out_old[0]) / 2.0);
            outData[index].m_v[1][1] = static_cast<float>((demod_pos_out[0] + demod_pos_out_old[0]) / 2.0);
            outData[index].m_v[1][2] = outData[index].m_v[1][1] - outData[index].m_v[1][0];

//#if 1
//			m_pCleanData[m_doAnalyze][index].m_v[2][0] = (float)((demod_neg_out[2] + demod_neg_out_old[2]) / 2);
//			m_pCleanData[m_doAnalyze][index].m_v[2][1] = (float)((demod_pos_out[2] + demod_pos_out_old[2]) / 2);
//			m_pCleanData[m_doAnalyze][index].m_v[2][2] =outData[index].m_v[2][1] - outData[index].m_v[2][0];
//#endif

            outData[index].m_v[3][0] = static_cast<float>((demod_neg_out[1] + demod_neg_out_old[1]) / 2.0);
            outData[index].m_v[3][1] = static_cast<float>((demod_pos_out[1] + demod_pos_out_old[1]) / 2.0);
            outData[index].m_v[3][2] = outData[index].m_v[3][1] - outData[index].m_v[3][0];

            index++;
        }
        counter++;
    }

    QString fileName = QString("./outdata-%1.csv").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss"));
    QFile *file = new QFile(fileName);
    if(!file->open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        return false;
    }
    else
    {
        QTextStream out(file);
        for(int i = 0; i < RAW_ONE_CHANNEL_COUNT / RAW_CHANNEL_COUNT; i++)
        {
            out << outData[i].m_v[1][0] << "," << outData[i].m_v[1][1] << "," << outData[i].m_v[1][2] << ","
                << outData[i].m_v[3][0] << "," << outData[i].m_v[3][1] << "," << outData[i].m_v[3][2] << "\r\n";
        }
        file->close();
    }

    delete []rawData;

    return true;
}
