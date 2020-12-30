#ifndef DATARECEIVER_H
#define DATARECEIVER_H

/**@file DataReceiver.h-文件名
* @brief      数据接收和处理线程类
* @details
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

#pragma execution_character_set("utf-8")///<指示char的执行字符集是UTF-8编码。

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QMetaType>
#include "Common.h"
#include <QQueue>

class MyGraphicsItem;///<图形条目
//class DBusClient;
struct DBusData;     ///<DBus数据，用来传输调用数据
class ColorMap;      ///<用来显示主图界面

class DataReceiver : public QThread///< 数据接收和处理线程类
{
Q_OBJECT
public:
    /**
     * @brief DataReceiver类构造函数
     */
    DataReceiver();
    virtual ~DataReceiver();

    /**
     * @brief DataIn
     * 将新数据放入数据队列
     * @param data
     */
    void DataIn(DBusData *data);

    /**
     * @brief SetDataAccepter
     * 设置接收数据的图元
     * @param item
     */
    void SetDataAccepter(MyGraphicsItem *item);

    /**
     * @brief Stop
     * 暂停条目的更新
     */
    void Stop();

    /**
     * @brief SetDataType
     * 设置数据类型
     * @param type
     */
    void SetDataType(PLOT_TYPE type) {
        m_type = type;
    }

    /**
     * @brief SetAppPath
     * 设置可执行程序的路径（加载百度地图的html文件用）
     * @param appPath
     */
    void SetAppPath(const QString &appPath)
    {
        m_path = appPath;
    }

signals:
    /**
     * @brief DataReady2
     * MyGraphicsItem的信号槽
     */
    void DataReady2();

protected:   
    /**
     * @brief run
     * 线程执行函数
     */
    void run() override;

private:

    /**
    * @brief GetDataFromDbus
    * 读取实时测量数据，从共享内存
    * @param data
    */
    void GetDataFromDbus(DBusData *data);

    /**
     * @brief GetHisDatas
     * 读取历史数据，从文件或数据库
     * @param data
     */
    void GetHisDatas(DBusData *data);

    /**
     * @brief ProcessMeasure
     * 2D实时测量数据简单处理
     * @param m_data
     */
    void ProcessMeasure(DBusData *m_data);

    /**
     * @brief Process3DMeasure
     * 3D实时测量数据简单处理
     * @param m_data
     */
    void Process3DMeasure(DBusData *m_data);

    /**
     * @brief ProcessHis
     * 2D历史测量数据简单处理
     * @param m_data
     */
    void ProcessHis(DBusData *m_data);

    /**
     * @brief Process3DHis
     * 3D历史测量数据简单处理
     * @param m_data
     */
    void Process3DHis(DBusData *m_data);

    QMutex m_mutex;                         ///< 线程的互斥锁
    QWaitCondition m_cond;                  ///< 多线程的同步
    bool m_dataIn;                          ///< 是否有新数据
    //DBusClient *m_dbusClient;
    MyGraphicsItem *m_item;                 ///< 图形条目
    bool m_setXY;                           ///< 设置x轴y轴成功标志
    bool m_stop;                            ///< 数据接收完成标志
    PLOT_TYPE m_type;                       ///< 图的类型
    ColorMap *m_colorMap;                   ///< 3D放大图
    bool m_seted;                           ///< 设置图类型的标志
    QQueue<DBusData*> m_datas;              ///< 数据队列
    QString m_path;                         ///< html文件路径
    int m_dataCount;                        ///< 历史数据分析时数据的总条数
    int m_curDataCount;                     ///< 当前的数据条数序号
};

#endif // DATARECEIVER_H
