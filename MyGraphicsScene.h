#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

/**@file MyGraphicsScene.h-文件名
* @brief      自定义场景
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

#include <QGraphicsScene>
#include "Common.h"

class MyGraphicsItem;///<自定义框图
class MyGraphicsView;///<界面的数据处理
class Jnui;          ///<自定义场景

class MyGraphicsScene : public QGraphicsScene
{
public:
    /**
     * @brief MyGraphicsScene类构造函数
     * @param parent 主窗体的this指针
     * @param jnui   Jnui的指针
     * @param type   PLOT_TYPE类型
     */
    explicit MyGraphicsScene(QWidget *parent, Jnui *jnui, PLOT_TYPE type);
    virtual ~MyGraphicsScene();

    /**
     * @brief Resize
     * 调整矩形画面的大小
     * @param width
     * 宽度
     * @param height
     * 高度
     */
    void Resize(int width, int height);

    /**
     * @brief NewSubView
     * 新建子图
     * @param pos         QPointF的指针
     * @param type        PLOT_TYPE类型
     * @param recordCount
     * @return
     */
    MyGraphicsItem * NewSubView(const QPointF &pos, PLOT_TYPE type, int recordCount=0);

    /**
     * @brief AddItem
     * 添加条目
     * @param item
     */
    void AddItem(MyGraphicsItem *item);

    /**
     * @brief RemoveItem
     * 移除条目
     * @param item
     */
    void RemoveItem(MyGraphicsItem *item);

    /**
     * @brief SwitchItem
     * 改变条目
     * @param item
     */
    void SwitchItem(MyGraphicsItem *item);

    /**
     * @brief UpdateItems
     * 当移动左侧缩略图滚动条时，更新场景中图元的位置
     * @param dx
     * @param dy
     */
    void UpdateItems(int dx, int dy);

private:
    QWidget *m_parent;                          ///< 父类窗口
    MyGraphicsView *m_gView;                    ///< 自定义框架
    QVector<MyGraphicsItem*> m_itemList;        ///< 左侧缩略图列表
    int m_curY;                                 ///< 现在的Y轴数量
    MyGraphicsItem *m_curItem;                  ///< 现在的条目数量
    int m_scrollBarPos;                         ///< 在界面上移动的图形条目的位置
    Jnui *m_jnui;                               ///< 界面的数据处理
};

#endif // MYGRAPHICSSCENE_H
