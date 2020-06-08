#ifndef MYGRAPHICSVIEW_H
#define MYGRAPHICSVIEW_H

#pragma execution_character_set("utf-8")

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
    void mousePressEvent(QMouseEvent *event) override;
    void scrollContentsBy(int dx, int dy) override;
private:
    MyGraphicsScene *m_scene;
};

#endif // MYGRAPHICSVIEW_H
