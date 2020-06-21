#pragma execution_character_set("utf-8")
#include "DataReceiver.h"
#include "MyGraphicsItem.h"
#include <iostream>
#include "ColorMap.h"
#include "jnui_global.h"

DataReceiver::DataReceiver()
    : QThread(), m_dataIn(false), m_setXY(false), m_stop(false), m_type(PLOT_TYPE::PLOT_2D_MAIN),
      m_seted(false), m_dataCount(0)
{
}

DataReceiver::~DataReceiver()
{

}

void DataReceiver::run()
{
    while(!m_stop) {
        m_mutex.lock();
        if (m_datas.empty()) {
            m_cond.wait(&m_mutex);
            std::cout << "thread wait" << std::endl;
            m_mutex.unlock();
            continue;
        }
        m_mutex.unlock();

        DBusData *data = m_datas.first();
        m_datas.pop_front();
        //m_mutex.unlock();

        if (!m_stop) {
            if (m_type == PLOT_TYPE::PLOT_2D_MAIN||
                m_type == PLOT_TYPE::PLOT_2D_SUB ||
                m_type == PLOT_TYPE::PLOT_3D_MAIN ||
                m_type == PLOT_TYPE::PLOT_3D_SUB_TIME ||
                m_type == PLOT_TYPE::PLOT_3D_SUB_POS) { // 实时测量
                GetDataFromDbus(data);
            } else if (m_type == PLOT_TYPE::PLOT_2D_HIS_MAIN ||
                       m_type == PLOT_TYPE::PLOT_3D_HIS_MAIN) { // 历史数据
                //std::cout << "precess start" << std::endl;
                GetHisDatas(data);
                //std::cout << "precess end" << std::endl;
            }
        }

        if (m_stop) {
            break;
        }
    }
}

void DataReceiver::GetDataFromDbus(DBusData *data)
{
    switch (m_type) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_SUB:
    {
        ProcessMeasure(data);
        emit DataReady2();
        break;
    }
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    {
        Process3DMeasure(data);
        emit DataReady2();
        break;
    }
    default:
        break;
    }
}

void DataReceiver::DataIn(DBusData *data)
{
    m_mutex.lock();
    m_dataIn = true;
    m_datas.push_back(data);
    //std::cout << "cout:" << m_datas.count() << " path:" << m_path.toStdString() << std::endl;
    m_cond.notify_one();
    m_mutex.unlock();
}

void DataReceiver::SetDataAccepter(MyGraphicsItem *item)
{
    m_item = item;
}

void DataReceiver::Stop()
{
    m_stop = true;
}

void DataReceiver::ProcessMeasure(DBusData *data)
{
    if (!m_seted && m_type != PLOT_TYPE::PLOT_2D_HIS_SUB) {
        if (!m_item->IsSubPlot()) {
            m_item->SetXYLable(data->labelOfChannelPosition, data->labelOfOneChannel);
            m_item->SetXYRange(0, data->channelPositionData[data->dataChannelsAmount-1] + 10, 200, 500);
        } else {
            // 子图不需要设置x，y轴的lable
            m_item->SetXYRange(0, data->channelPositionData[data->dataChannelsAmount-1] + 10, 280, 285);
        }
        // 设置百度地图的中心点
        m_item->SetBaiduMapCenter(data->baiduMapCenter);
        // 设置程序运行路径，目的是为了后面百度地图加载html文件
        m_item->SetAppPath(m_path);

        if (!m_item->IsSubPlot()) {
            QVector<double> lons, lats;
            for (int i = 0; i < data->totalDataAmount; ++i) {
                lons.append(data->longiLatitudeData[i]->longitude);
                lats.append(data->longiLatitudeData[i]->latitude);
            }
            m_item->SetLonLats(lons, lats);
        }

        m_seted = true;
    }

    if (!m_item->IsSubPlot()) { // 2d主图更新
        m_item->UpdateData(data->channelPositionData, data->dataContent);
        m_item->UpdateWarnings(data->warningData);
    } else {    // 2d子图更新
       QPointF pos = m_item->GetSubPlotPoint();
        double x = pos.x();
        if (x == 0) {
            goto out;
        }

        bool get = false;
        double minDiff = 100000; // 随意一个大值
        int index = 0;
        for (int i = 0; i < data->channelPositionData.count(); ++i) {
            double diff = qAbs(data->channelPositionData[i] - x);
            if (diff <= minDiff) {
                minDiff = diff;
                index = i;
                get = true;
            }

            if (get && diff > minDiff) { // x轴数值从小到大
                break;
            }
        }

        m_item->UpdateData(QDateTime::currentSecsSinceEpoch(), data->dataContent[index]);
    }

out:
    data->clear();
    delete data;
}

void DataReceiver::Process3DMeasure(DBusData *data)
{
    if (!m_seted ) {
        m_item->SetXYLable(data->labelOfChannelPosition,
                           data->labelOfOneChannel);
        m_item->SetXYRange(0, data->channelPositionData[data->dataChannelsAmount-1],
            0, data->dataContent[data->dataAmountEachChannel-1]);

        m_item->SetBaiduMapCenter(data->baiduMapCenter);
        m_item->SetAppPath(m_path);

        if (m_type == PLOT_TYPE::PLOT_3D_MAIN) {
            QVector<double> lons, lats;
            for (int i = 0; i < data->dataChannelsAmount; ++i) {
                lons.push_back(data->longiLatitudeData[i]->longitude);
                lats.push_back(data->longiLatitudeData[i]->latitude);
            }
            m_item->SetLonLats(lons, lats);
        }

        m_seted = true;
    }

    if (m_type == PLOT_TYPE::PLOT_3D_MAIN) {
        QVector<double> lons, lats;
        QCPColorMapData *mapdata = new QCPColorMapData(data->dataChannelsAmount,
            data->dataAmountEachChannel,
            QCPRange(0, data->channelPositionData[data->dataChannelsAmount-1]),
            QCPRange(0, data->dataContent[data->dataAmountEachChannel-1]));

        int z = 0;
        for (int i = 0; i < data->dataChannelsAmount; ++i) {
            for (int j = 0; j < data->dataAmountEachChannel; ++j) {
                mapdata->setCell(i, j, (data->zData[z++]));
            }
        }

        m_item->UpdateData(data->channelPositionData, data->dataContent, data->zData, mapdata);
        m_item->UpdateWarnings(data->warningData);
    } else if (m_type == PLOT_TYPE::PLOT_3D_SUB_TIME) {
        QVector<double> zdata;
        QPointF point = m_item->GetSubPlotPoint();
        int index = 0;
        bool get = false;
        double minDiff = 100000; // 随意一个大值
        double x = point.x();
        for (int i = 0; i < data->dataChannelsAmount; ++i) {
            double diff = qAbs(data->channelPositionData[i] - x);
            if (diff <= minDiff) {
                minDiff = diff;
                index = i;
                get = true;
            }

            if (get && diff > minDiff) { // x轴数值从小到大
                break;
            }
        }

        int z = 0;
        for (int j = 0; j < data->dataAmountEachChannel; ++j) {
            zdata.append(data->zData[index*j + z++]);
        }

        m_item->SetValueSize(data->dataAmountEachChannel);
        m_item->UpdateData(zdata);
    } else if (m_type == PLOT_TYPE::PLOT_3D_SUB_POS) {
        QVector<double> zdata;
        QPointF point = m_item->GetSubPlotPoint();
        int index = 0;
        bool get = false;
        double minDiff = 100000; // 随意一个大值
        double y = point.y();
        for (int i = 0; i < data->dataAmountEachChannel; ++i) {
            double diff = qAbs(data->dataContent[i] - y);
            if (diff <= minDiff) {
                minDiff = diff;
                index = i;
                get = true;
            }

            if (get && diff > minDiff) { // x轴数值从小到大
                break;
            }
        }
        int z = 0;
        for (int j = 0; j < data->dataChannelsAmount; ++j) {
            zdata.append(data->zData[index*j + z++]);
        }

        m_item->SetKeySize(data->dataChannelsAmount);
        m_item->UpdateData(zdata);
    }

    data->clear();
    delete data;
}

void DataReceiver::ProcessHis(DBusData *data)
{
    QVector<double> lons, lats;
    for (int i = 0; i < data->totalDataAmount; ++i) {
        lons.append(data->longiLatitudeData[i]->longitude);
        lats.append(data->longiLatitudeData[i]->latitude);
    }

    if (!m_seted) {
        m_item->SetBaiduMapCenter(data->baiduMapCenter);
        m_item->SetAppPath(m_path);
        m_item->SetXYLable(data->labelOfChannelPosition, data->labelOfOneChannel);
        m_item->SetXYRange(0, data->channelPositionData[data->dataChannelsAmount-1] + 10, 200, 500);
        m_item->SetLonLats(lons, lats);
        m_seted = true;
        m_item->SetMultiLineCount(data->dataCount);
        std::cout << "data count=" << data->dataCount << std::endl;
        m_dataCount = data->dataCount;
    }

    m_item->UpdateData(data->channelPositionData, data->dataContent, data->time);
    m_item->UpdateWarnings(data->warningData);

    data->clear();
    delete data;
}

void DataReceiver::Process3DHis(DBusData *data)
{
    if (!m_seted) {
        m_item->SetXYLable(data->labelOfChannelPosition,
                           data->labelOfOneChannel);
        m_item->SetXYRange(0, data->channelPositionData[data->dataChannelsAmount-1],
            0, data->dataContent[data->dataAmountEachChannel-1]);
        m_item->SetBaiduMapCenter(data->baiduMapCenter);
        m_item->SetAppPath(m_path);
        m_item->SetMultiLineCount(data->dataCount);
        m_dataCount = data->dataCount;
        m_seted = true;
    }

    QCPColorMapData *mapdata = new QCPColorMapData(data->dataChannelsAmount,
        data->dataAmountEachChannel,
        QCPRange(0, data->channelPositionData[data->dataChannelsAmount-1]),
        QCPRange(0, data->dataContent[data->dataAmountEachChannel-1]));
    int z = 0;
    for (int i = 0; i < data->dataChannelsAmount; ++i) {
        for (int j = 0; j < data->dataAmountEachChannel; ++j) {
            mapdata->setCell(i, j, (data->zData[z++]));
        }
    }

    QVector<double> lons, lats;
    for (int i = 0; i < data->dataChannelsAmount; ++i) {
        lons.push_back(data->longiLatitudeData[i]->longitude);
        lats.push_back(data->longiLatitudeData[i]->latitude);
    }

    m_item->UpdateData(data->channelPositionData,  data->dataContent, data->zData, mapdata, data->time);
    m_item->SetLonLats(lons, lats);

    data->clear();
    delete data;
}

void DataReceiver::GetHisDatas(DBusData *data)
{
    switch (m_type) {
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    {
        m_curDataCount++;
        ProcessHis(data);
        break;
    }
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    {
        m_curDataCount++;
        Process3DHis(data);
        break;
    }
    default:
        break;
    }

    if (m_dataCount !=0 && m_dataCount == m_curDataCount) {
        emit DataReady2();
        m_dataCount = 0;
        m_curDataCount = 0;
    }
}
