#ifndef JNUI_GLOBAL_H
#define JNUI_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QString>
#include <QVector>

#if defined(JNUI_LIBRARY)
#  define JNUI_EXPORT Q_DECL_EXPORT
#else
#  define JNUI_EXPORT Q_DECL_IMPORT
#endif

struct LongiLatitude
{
    double longitude;
    double latitude;
};

typedef struct DBusData
{
    int lineIndex; // 线路编号，范围：1~8
    QString lineName; // 具体线路的名称，比如“猎豹”
    int isIncludeMapData; // 是否支持地图模式
    int dataChannelsAmount; // 一条线路中测量的数据位置（通道）数，范围：1到1000000（1百万）
    int dataAmountEachChannel; // 每个位置对应的数据点数，范围：1到4096
    float dataInOneChannel; // 每个位置内的所有数据对应的数据坐标，范围：-3.4e38到3.4e38
    QString labelOfOneChannel; // 每个位置内的数据对应的LABEL，频率/兆赫兹，应变/微应变，温度/摄氏度，其他待定，比如“猎豹速度(公里/小时)”
    int totalDataAmount;    // 总的数据点数, 范围：1到4096*20000
    QVector<double> dataContent;  // 对应所有数据点数的数据内容,范围：-3.4e38到3.4e38
    QVector<struct LongiLatitude*> longiLatitudeData; // 对应所有位置的所有经纬度数据
    QVector<double> channelPositionData;
    QString labelOfChannelPosition;
    QVector<double> warningData;
    QVector<double> zData;
    QString baiduMapCenter; // 百度地图中心点
    quint64 time;           // 更新数据的时间，默认为0 (用于历史数据分析)
    int dataCount;          // 表示历史数据分析时数据的总条数
    int versionDate;

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
