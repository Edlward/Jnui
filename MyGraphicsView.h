#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

/**@file MyGraphicsView.h-文件名
* @brief      数据处理模块头文件
* @details    主要包含更新坐标系标尺大小的函数接口，用于在大图呈现区改变坐标轴大小
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

#include <QGraphicsView>
#include "MyGraphicsScene.h"


class MyGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    MyGraphicsView(MyGraphicsScene *scene, QWidget *parent = nullptr)
        : QGraphicsView(reinterpret_cast<QGraphicsScene*>(scene), parent),
          m_scene(scene)
    {};
    virtual ~MyGraphicsView() {};

protected:
    /**
     * @brief mousePressEvent
     * 鼠标按下事件
     * @param event 事件类型
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief scrollContentsBy
     * 更新坐标轴
     * @param dx x坐标轴范围
     * @param dy y坐标轴范围
     */
    void scrollContentsBy(int dx, int dy) override;
private:
    MyGraphicsScene *m_scene;
};

#endif // MYGRAPHICSVIEW_H
