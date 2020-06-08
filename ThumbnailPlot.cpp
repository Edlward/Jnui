#pragma execution_character_set("utf-8")

#include "ThumbnailPlot.h"
#include "MyGraphicsItem.h"
#include <iostream>
#include "MyGraphicsScene.h"

ThumbnailPlot::ThumbnailPlot(MyGraphicsItem *item)
    : m_item(item)
{

}

ThumbnailPlot::~ThumbnailPlot()
{

}

void ThumbnailPlot::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_item->ShowMain();
        m_item->SetFocus();
    } else {
        if (!m_item->IsSubPlot()) {
            return;
        }

        QMenu menu(m_item->m_parent);   // 不要设置成this，会引起double free
        QAction action1(tr("删除子图"), m_item->m_parent);
        action1.setData(1);
        menu.addAction(&action1);

        connect(&action1, &QAction::triggered, this, [&] () {
            m_item->Destory();
        });
        menu.exec(QCursor::pos());
    }
}
