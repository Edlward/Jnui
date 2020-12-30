#ifndef JNUI_GLOBAL_H
#define JNUI_GLOBAL_H
/**@file      jnui_global.h-文件名
* @brief      传输与调用数据模块头文件
* @details    主要包含DAS解调算法、BOTDR解调算法、语音信号处理、共享内存读写等函数接口，用于对光纤线路中采集到的原始采样数据进行处理
* @mainpage   工程概览
* @author     lzy
* @email      jnuerlzy@gmail.com
* @version    V1.0
* @date       2020-12-07
* @copyright  Copyright (c) 2020-2022 广州凌远技术有限公司
************************************************************
* @attention
* 硬件平台：xx   ;
* 软件版本： Qt5.12.3 msvc2017 64bit
* @par 修改日志
* <table>
* <tr><th>Date        <th>Version  <th>Author  <th>Description
* <tr><td>2020/11/30  <td>1.0      <td>lzy     <td>初始版本
* </table>
*
************************************************************
*/
#include <QtCore/qglobal.h>
#include <QString>
#include <QVector>

#if defined(JNUI_LIBRARY)
#  define JNUI_EXPORT Q_DECL_EXPORT
#else
#  define JNUI_EXPORT Q_DECL_IMPORT
#endif

/**
* @brief jnui_global
* 主要用来储存地图数据经纬度信息
*/
struct LongiLatitude
{
    double longitude;
    double latitude;
};


/**
* @brief jnui_global
* 用于通过DBus传输调用数据
*/
typedef struct DBusData
{
    int lineIndex;                                          ///< 线路编号，范围：1~8
    QString lineName;                                       ///< 具体线路的名称，比如“猎豹”
    int isIncludeMapData;                                   ///< 是否支持地图模式
    int dataChannelsAmount;                                 ///< 一条线路中测量的数据位置（通道）数，范围：1到1000000（1百万）
    int dataAmountEachChannel;                              ///< 每个位置对应的数据点数，范围：1到4096
    float dataInOneChannel;                                 ///< 每个位置内的所有数据对应的数据坐标，范围：-3.4e38到3.4e38
    QString labelOfOneChannel;                              ///< 每个位置内的数据对应的LABEL，频率/兆赫兹，应变/微应变，温度/摄氏度，其他待定，比如“猎豹速度(公里/小时)”
    int totalDataAmount;                                    ///< 总的数据点数, 范围：1到4096*20000
    QVector<double> dataContent;                            ///< 对应所有数据点数的数据内容,范围：-3.4e38到3.4e38
    QVector<struct LongiLatitude*> longiLatitudeData;       ///< 对应所有位置的所有经纬度数据
    QVector<double> channelPositionData;                    ///< 数据存放路径位置
    QString labelOfChannelPosition;                         ///< 存放路径的标签
    QVector<double> warningData;                            ///< 错误数据
    QVector<double> zData;                                  ///< z轴数据
    QString baiduMapCenter;                                 ///< 百度地图中心点
    quint64 time;                                           ///< 更新数据的时间，默认为0 (用于历史数据分析)
    int dataCount;                                          ///< 表示历史数据分析时数据的总条数
    int versionDate;                                        ///< 表示历史数据分析时数据的总条数

    /**
     * @brief clear
     * 清空
     */
    void clear()
    {
        dataContent.clear();
        for (auto i : longiLatitudeData) {
            delete i;
        }
        longiLatitudeData.clear();
        channelPositionData.clear();
        warningData.clear();
        zData.clear();
    }

    DBusData() = default;

    DBusData(const DBusData &data)
        : lineIndex(data.lineIndex), lineName(data.lineName), isIncludeMapData(data.isIncludeMapData),
          dataChannelsAmount(data.dataChannelsAmount), dataAmountEachChannel(data.dataAmountEachChannel),
          dataInOneChannel(data.dataInOneChannel), labelOfOneChannel(data.labelOfOneChannel),
          totalDataAmount(data.totalDataAmount), labelOfChannelPosition(data.labelOfChannelPosition),
          baiduMapCenter(data.baiduMapCenter), time(data.time), dataCount(data.dataCount),
          versionDate(data.versionDate)
    {
        for (auto d : data.dataContent) {
            dataContent.append(d);
        }
        for (auto d : data.longiLatitudeData) {
            struct LongiLatitude* lonlat = new LongiLatitude();
            lonlat->longitude = d->longitude;
            lonlat->latitude = d->latitude;
            longiLatitudeData.append(lonlat);
        }
        for (auto d : data.channelPositionData) {
            channelPositionData.append(d);
        }
        for (auto d : data.warningData) {
            warningData.append(d);
        }
        for (auto d : data.zData) {
            zData.append(d);
        }
    }
} DBusData_t;

#endif // JNUI_GLOBAL_H
