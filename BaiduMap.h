#ifndef BAIDUMAP_H
#define BAIDUMAP_H

/**@file BaiduMap.h-文件名
* @brief      百度地图文件
* @details    包括了JsBridge类和BaiduMap类，JsBridge主要是用与给地图界面传递数值，BaiduMap是用来显示地图界面的图形和文字
* @mainpage   工程概览
* @author     lzy
* @email      jnuerlzy@gmail.com
* @version    V1.0
* @date       2020-12-22
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

#include <QObject>
#include <QWidget>
#include <QVariantList>
#include "CustomPlotInterface.h"

#pragma execution_character_set("utf-8")///<指示char的执行字符集是UTF-8编码。

class QWebEngineView;
class QWebChannel;
class QCustomPlot;
class BaiduMap;

struct LonLat
{
    double lon;
    double lat;
};

class JsBridge : public QObject
{
Q_OBJECT
public:
    /**
     * @brief JsBridge
     * @param baiduMap
     * @param centerName
     */
    JsBridge(BaiduMap *baiduMap, const QString &centerName);
    ~JsBridge();

    /**
     * @brief SetData
     * @param lons
     * @param lats
     */
    void SetData(const QVector<double> &lons, const QVector<double> &lats);

    /**
     * @brief UpdateWarnings
     * @param warnings
     */
    void UpdateWarnings(const QVector<double> &warnings);

signals:

    /**
     * @brief sigUpdateData
     * 发送信号
     */
    void sigUpdateData();
public slots:

    /**
     * @brief GetData
     * 获取数据
     * @return
     */
    QVariantList GetData();

    /**
     * @brief GetWarnings
     * 获取警告
     * @return
     */
    QVariantList GetWarnings();

    /**
     * @brief SwitchDataView
     * 数据窗口的开关
     */
    void SwitchDataView();

    /**
     * @brief GetCenterName
     * 得到中心点
     * @return
     */
    QString GetCenterName();

    /**
     * @brief GetMeasureDataOfPos
     * 得到回测的信息
     * @param lon
     * @param lat
     * @return
     */
    QString GetMeasureDataOfPos(QString lon, QString lat);

private:
    ///
    QVariantList m_data;                        ///< 数据储存
    QVariantList m_warnings;                    ///< 错误信息的储存
    BaiduMap *m_baiduMap;                       ///< 中心点经度纬度的信息储存
    QVector<double> m_lons;                     ///< 中心点经度
    QVector<double> m_lats;                     ///< 中心点纬度
    QString m_baiduMapCenter;                   ///< 中心点名字
};
/**
 * @brief The BaiduMap class
 * 用来显示地图的界面，包括一些信息和数据。
 */
class BaiduMap : public QWidget
{
public:
    /**
     * @brief BaiduMapp类构造函数
     * @param parent     主窗体的this指针
     * @param plot       指向CustomPlotInterface所继承的父类
     * @param centerName 指向函数
     * @param appPath    html文件位置
     */
    BaiduMap(QWidget *parent, CustomPlotInterface *plot, const QString &centerName, const QString &appPath);
    ~BaiduMap();

    /**
     * @brief resize
     * 界面长端、框格数据
     * @param wigth
     * @param height
     */
    void resize(qreal wigth, qreal height);

    /**
     * @brief UpdateData
     * 更新数据
     * @param lons
     * @param lats
     */
    void UpdateData(const QVector<double> &lons, const QVector<double> &lats);

    /**
     * @brief UpdateWarnings
     * 更新错误信息
     * @param warnings
     */
    void UpdateWarnings(const QVector<double> &warnings);

    /**
     * @brief SwitchDataView
     * 开关数据界面
     */
    void SwitchDataView();

    /**
     * @brief show
     * 界面显示
     */
    void show();

    /**
     * @brief hide
     * 界面隐藏
     */
    void hide();

    /**
     * @brief GetMeasureDataOfPos
     * 鼠标捕捉的经纬度信息
     * @param lon
     * @param lat
     * @return
     */
    QString GetMeasureDataOfPos(double lon, double lat);

private:
    JsBridge *m_bridge;                         ///< 储存桥梁信息
    QWebChannel *m_channel;                     ///< 中心点名字
    QWebEngineView *m_webView;                  ///< 方便实现页面对Qt对象的使用（搜索途径）
    CustomPlotInterface *m_plot;                ///< 提供一个访问web页面的widget
    bool m_dataUpdated;                         ///< 更新数据
};

#endif // BAIDUMAP_H
