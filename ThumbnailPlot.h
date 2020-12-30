#ifndef THUMBNAILPLOT_H
#define THUMBNAILPLOT_H

/**@file ThumbnailPlot.h-文件名
* @brief      数据处理模块头文件
* @details    包含mousePressEvent函数接口，用于设置鼠标事件，删除子图
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

#pragma execution_character_set("utf-8")///<指示char的执行字符集是UTF-8编码。

#include "qcustomplot.h"

class MyGraphicsItem;

class ThumbnailPlot : public QCustomPlot
{
public:
    /**
     * @brief ThumbnailPlot类构造函数
     * @param item   MyGraphicsItem的指针
     * @param parent 主窗体的this指针
     * QWidget *parent 指定所继承的父类，默认为nullptr
     */
    ThumbnailPlot(MyGraphicsItem *item, QWidget *parent);
    virtual ~ThumbnailPlot();

protected:
    /**
     * @brief mousePressEvent
     * 鼠标按键事件，用以删除子图
     * @param event 事件
     */
    void mousePressEvent(QMouseEvent *event) override;
private:
    MyGraphicsItem *m_item; ///< 关联的缩略图
};

#endif // THUMBNAILPLOT_H
