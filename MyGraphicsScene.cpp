#pragma execution_character_set("utf-8")

#include "MyGraphicsScene.h"
#include "MyGraphicsItem.h"
#include <QMenu>
#include <iostream>
#include "MyGraphicsView.h"
#include "jnui.h"

MyGraphicsScene::MyGraphicsScene(QWidget *parent, Jnui *jnui, PLOT_TYPE type)
    : QGraphicsScene(parent), m_parent(parent), m_curY(0), m_curItem(nullptr), m_scrollBarPos(0),
      m_jnui(jnui)
{
    setSceneRect(0, 0, GRAPHICS_SCENE_WIDTH, 4800);   // 最后一个值大小比gview的最后一个值大就会出现滚动条

    /// 新建左侧缩略图的视图
    m_gView = new MyGraphicsView(this, parent);
    if (!m_gView) {
        return;
    }

    m_gView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_gView->resize(GRAPHICS_SCENE_WIDTH, 3600);
    m_gView->centerOn(0, 0); // 让右侧滚动条初始化时显示在最上面
    m_gView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);// 下面的滚动条隐藏

    // 添加子图item
    MyGraphicsItem *item = new MyGraphicsItem(0, this, parent, type);
    item->SetTitle(QString("主图"));
    item->SetFocus();
    item->ShowMain();
}

MyGraphicsScene::~MyGraphicsScene()
{
    while(!m_itemList.empty()) {
        MyGraphicsItem *item = m_itemList.first();
        m_itemList.pop_front();
        delete item;
    }

    delete m_gView;
}

void MyGraphicsScene::Resize(int width, int height)//调整矩形图像大小
{
    setSceneRect(0, 0, GRAPHICS_SCENE_WIDTH, height*2);
    m_gView->resize(GRAPHICS_SCENE_WIDTH, height);

    for (auto i : m_itemList) {
        i->resize(width - GRAPHICS_SCENE_WIDTH, height);
    }
}

MyGraphicsItem * MyGraphicsScene::NewSubView(const QPointF &pos, PLOT_TYPE type, int recordCount)//新子图
{
    m_curY += GRAPHICS_ITEM_HEIGHT;
    MyGraphicsItem *item = new MyGraphicsItem(m_curY+m_scrollBarPos, this, m_parent, type);
    if (!item) {
        return nullptr;
    }

    if (recordCount == 0) {
        if (PLOT_TYPE::PLOT_2D_SUB == type || PLOT_TYPE::PLOT_2D_HIS_SUB == type) {
            recordCount = MAX_2D_RECORD_COUNT;
        } else {
            recordCount = MAX_3D_RECORD_COUNT;
        }
    }

    item->SetSubPlotInfo(pos, recordCount);
    item->resize(m_parent->width() - GRAPHICS_ITEM_WIDTH, m_parent->height());
    char str[32] = {0};
    if (PLOT_TYPE::PLOT_2D_SUB == type ||
        PLOT_TYPE::PLOT_2D_HIS_SUB == type ||
        PLOT_TYPE::PLOT_3D_SUB_TIME == type ||
        PLOT_TYPE::PLOT_3D_HIS_TIME == type) {
        sprintf(str, "位置:%.2lf", pos.x());
    } else if (PLOT_TYPE::PLOT_3D_SUB_POS == type ||
               PLOT_TYPE::PLOT_3D_HIS_POS == type) {
        sprintf(str, "频率:%.2lf", pos.y());
    }
    item->SetTitle(QString(str));
    item->HideMain();
    return item;
}

void MyGraphicsScene::AddItem(MyGraphicsItem *item)//增加一条数据
{
    if (m_curItem == nullptr) {
        m_curItem = item;
    }

    m_jnui->AddDataReceiver(item->m_receiver);

    addItem(item);
    m_itemList.append(item);

    int cur = 0;
    for (int i = 0; i < m_itemList.count(); ++i) {
        m_itemList[i]->ResetY(cur + m_scrollBarPos);
        m_itemList[i]->boundingRect();
        m_itemList[i]->ReDraw(0, cur + m_scrollBarPos);
        m_itemList[i]->show();
        cur += GRAPHICS_ITEM_HEIGHT;
    }
}

void MyGraphicsScene::RemoveItem(MyGraphicsItem *item)//删除一条数据
{
    m_curY -= GRAPHICS_ITEM_HEIGHT;
    m_itemList.removeOne(item);
    item->HideMain();
    if (m_curItem == item) {
        m_curItem = m_itemList[0];
        m_curItem->ShowMain();
    }
    m_jnui->RemoveDataReceiver(item->m_receiver);
    removeItem(item);
    delete item;

    int cur = 0;
    for (int i = 0; i < m_itemList.count(); ++i) {
        m_itemList[i]->ResetY(cur + m_scrollBarPos);
        m_itemList[i]->boundingRect();
        m_itemList[i]->ReDraw(0, cur + m_scrollBarPos);
        cur += GRAPHICS_ITEM_HEIGHT;
    }
}

void MyGraphicsScene::SwitchItem(MyGraphicsItem *item)
{
    if (m_curItem && m_curItem != item) {
        m_curItem->HideMain();
        m_curItem->RemoveFocus();
    }

    m_curItem = item;
    m_curItem->SetFocus();
}

void MyGraphicsScene::UpdateItems(int dx, int dy)
{
    m_scrollBarPos += dy;   // 当前滚动条的位置

    int cur = 0;
    for (int i = 0; i < m_itemList.count(); ++i) {
        m_itemList[i]->ResetY(cur + m_scrollBarPos);
        m_itemList[i]->boundingRect();
        m_itemList[i]->ReDraw(dx, cur + m_scrollBarPos);
        cur += GRAPHICS_ITEM_HEIGHT;
    }
}
