#ifndef JNUI_H
#define JNUI_H
/**@file jnui.h-文件名
* @brief      界面的数据处理
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
#include "jnui_global.h"
#include <QMutex>
#include <QVector>
#include <QString>

class MyGraphicsScene;
class DataReceiver;
struct DBusData;
class QWidget;
/**
 * @brief The Jnui class
 */
class JNUI_EXPORT Jnui
{
public:
    /// parent为主窗体的this指针，type为PLOT_TYPE类型，appPath为html文件位置
    /**
     * @brief MainWindow类构造函数
     * @param [in] QWidget *parent 指定所继承的父类，默认为nullptr
     */
    Jnui(QWidget *parent, int type, const QString &appPath);
    ~Jnui();

    /**
    * @brief UpdateData
    * 更新数据
    * @param data
    */
    void UpdateData(struct DBusData *data);

     /**
     * @brief Resize
     * 更新设置主窗口大小
     * @param width
     * 窗口宽度
     * @param height
     * 窗口高度
     */
    void Resize(qreal width, qreal height);

    friend MyGraphicsScene;///<友类
private:

    /**
     * @brief AddDataReceiver
     * 添加数据接收和处理线程
     * @param receiver
     */
    void AddDataReceiver(DataReceiver *receiver);

    /**
     * @brief RemoveDataReceiver
     * 删除数据接收和处理线程
     * @param receiver
     */
    void RemoveDataReceiver(DataReceiver *receiver);

    MyGraphicsScene *m_scene;           ///< 自定义场景，作为主图和子图等图元的容器
    QVector<DataReceiver*> m_receivers; ///< 数据接收和处理线程数组容器
    QString m_appPath;                  ///< 可执行程序的路径
    QMutex m_mutex;                     ///< 互斥锁
};

#endif // JNUI_H
