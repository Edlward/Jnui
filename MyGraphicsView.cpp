#pragma execution_character_set("utf-8")

#include "MyGraphicsView.h"
#include <iostream>

void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);//监视鼠标事件
}

void MyGraphicsView::scrollContentsBy(int dx, int dy)
{
    //std::cout << "scrollContentsBy x:" << dx << " y:" << dy << std::endl;
    m_scene->UpdateItems(dx, dy);//更新设置坐标轴
}
