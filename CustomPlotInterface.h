#ifndef CUSTOMPLOTINTERFACE_H
#define CUSTOMPLOTINTERFACE_H

/**@file CustomPlotInterface.h-文件名
* @brief      指令文件
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

#include <QString>
#include <QVector>

/**
 * @brief The CustomPlotInterface class
 */
class CustomPlotInterface
{
public:
    /**
     * @brief ExitBaiduMap
     * 退出百度地图模式
     */
    virtual void ExitBaiduMap() = 0;

    /**
     * @brief GetMeasureDataOfPos
     * 获得测量数据
     * @param lon
     * @param lat
     * @return
     */
    virtual QString GetMeasureDataOfPos(double lon, double lat) = 0;

    /**
     * @brief SetAppPath
     * 设置可执行程序的路径（加载百度地图的html文件用）
     * @param appPath
     */
    virtual void SetAppPath(const QString &appPath) = 0;

protected:
    QVector<double> m_warnings;             ///< 错误信息储存
    QString m_appPath;                      ///< 设置可执行程序的路径（加载百度地图的html文件用）
};

#endif // CUSTOMPLOTINTERFACE_H
