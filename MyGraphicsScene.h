#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#pragma execution_character_set("utf-8")

#include <QGraphicsScene>
#include "Common.h"

class MyGraphicsItem;
class MyGraphicsView;
class Jnui;

class MyGraphicsScene : public QGraphicsScene
{
public:
    explicit MyGraphicsScene(QWidget *parent, Jnui *jnui, PLOT_TYPE type);
    virtual ~MyGraphicsScene();

    void Resize(int width, int height);

    MyGraphicsItem * NewSubView(const QPointF &pos, PLOT_TYPE type, int recordCount=0);
    void AddItem(MyGraphicsItem *item);
    void RemoveItem(MyGraphicsItem *item);
    void SwitchItem(MyGraphicsItem *item);

    void UpdateItems(int dx, int dy);

private:
    QWidget *m_parent;
    MyGraphicsView *m_gView;
    QVector<MyGraphicsItem*> m_itemList;
    int m_curY;
    MyGraphicsItem *m_curItem;
    int m_scrollBarPos;
    Jnui *m_jnui;
};

#endif // MYGRAPHICSSCENE_H
