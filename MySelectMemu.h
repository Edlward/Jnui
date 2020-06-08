#ifndef MYSELECTMEMU_H
#define MYSELECTMEMU_H

#pragma execution_character_set("utf-8")

#include <QTreeWidget>

class MyCustomPlot;
class QCPAxis;

class MySelectMemu : public QTreeWidget
{
public:
    MySelectMemu(MyCustomPlot *plot, const QPoint &pos, int lineCount);
    virtual ~MySelectMemu();
public slots:
    void CreateCrosswiseLine();
    void CreateLengthwiseLine();
    void NewSubView();
    void ClearLines();
//    void ZoomIn();
//    void ZoomOut();
    void ZoomReset();
    void InterMultilineMode();
    void ExitMultilineMode();
    void InterMapMode();
    void ChooseBaseLine();
    void SetXRange();
    void SetYRange();

private:
    MyCustomPlot *m_plot;
    QPoint m_pos;
    int m_lineCount;
    QMenu *m_subMenu;
};

#endif // MYSELECTMEMU_H
