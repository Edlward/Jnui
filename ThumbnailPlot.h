#ifndef THUMBNAILPLOT_H
#define THUMBNAILPLOT_H

#pragma execution_character_set("utf-8")

#include "qcustomplot.h"

class MyGraphicsItem;

class ThumbnailPlot : public QCustomPlot
{
public:
    ThumbnailPlot(MyGraphicsItem *item, QWidget *parent);
    virtual ~ThumbnailPlot();

protected:
    void mousePressEvent(QMouseEvent *event) override;
private:
    MyGraphicsItem *m_item;
};

#endif // THUMBNAILPLOT_H
