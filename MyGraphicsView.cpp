#pragma execution_character_set("utf-8")

#include "MyGraphicsView.h"
#include <iostream>

void MyGraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
}

void MyGraphicsView::scrollContentsBy(int dx, int dy)
{
    //std::cout << "scrollContentsBy x:" << dx << " y:" << dy << std::endl;
    m_scene->UpdateItems(dx, dy);
}
