#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QObject>
#include <QQueue>
//#include "WorkerThread.h"
#include "qcustomplot.h"
#include "Common.h"
#include "ColorMap.h"

#pragma execution_character_set("utf-8")

//QT_CHARTS_USE_NAMESPACE

class TwoDimensionChartView;
class MainWindow;
class DataReceiver;
struct DBusData;
class MyCustomPlot;
class ThumbnailPlot;
class MyGraphicsScene;

class MyGraphicsItem : public QObject, public QGraphicsItem
{
Q_OBJECT
Q_INTERFACES(QGraphicsItem)

public:
    MyGraphicsItem(int y_pos, MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);
    virtual ~MyGraphicsItem();
    void ShowMain();
    void HideMain();

    QRectF boundingRect() const override;

    void SetXYLable(const QString &xName, const QString &yName/*,
                    const QString &xFormat = QString("%d"),
                    const QString &yFormat = QString("%d")*/);
    void SetXYRange(qreal xStart, qreal xEnd, qreal yStart, qreal yEnd);
    void SetXRange(qreal xStart, qreal xEnd);
    void SetYRange(qreal yStart, qreal yEnd);
    void SetY2Range(qreal yStart, qreal yEnd);

    void SetXLabel(const QString &xName);
    void SetYLabel(const QString &yName);

    // 更新2维主图
    void UpdateData(QVector<double> &xData, QVector<double> &yData, qint64 time=0);
    // 更新2维子图（单个点的数据）
    void UpdateData(double xData, double yData);
    // 更新3维主图
    void UpdateData(QCPColorMapData *mapData);
    // 更新3维子图
    void UpdateData(QVector<double> &zData);
    // 更新3维主图
    void UpdateData(const QVector<double> &xData, const QVector<double> &yData,
                    const QVector<double> &zData, QCPColorMapData *mapData, qint64 time=0);
    // 更新告警信息
    void UpdateWarnings(const QVector<double> &warnings);

    void SetLonLats(const QVector<double> &lons, const QVector<double> &lats);
    bool SetMultiLineCount(int count); // 仅应用于历史信息观察模式

    void SetBaiduMapCenter(const QString &centerName); // 设置百度地图中初始中心位置（如，广州、桂林等）
    // 设置可执行程序的路径（加载百度地图的html文件用）
    void SetAppPath(const QString &appPath);

    inline void SetKeySize(int keySize)
    {
        m_keySize = keySize;
        m_colorMap->SetKeySize(keySize);
    }
    inline void SetValueSize(int valueSize)
    {
        m_valueSize = valueSize;
        m_colorMap->SetValueSize(valueSize);
    }

    void SetColorScaleRange(double lower, double uper)
    {
        if (m_colorScale) {
            m_colorScale->setDataRange(QCPRange(lower, uper));
        }

        if (m_colorMap) {
            m_colorMap->SetColorScaleRange(lower, uper);
        }
    }

    void SetColorScalePolicy(const QCPColorGradient &gradient)
    {
        if (m_color) {
            m_color->setGradient(gradient);
        }

        if (m_colorMap) {
            m_colorMap->SetColorScalePolicy(gradient);
        }
    }

    void resize(qreal wigth, qreal height);
    void SetTitle(const QString &name);
    void ResetY(int y_pos);
    void ReDraw(int x, int y);
    void SetFocus();
    void RemoveFocus();

    void InterMultilineMode(int multilineCount);
    void ExitMultilineMode();
    void SetBaseLine(int index);
    void RemoveBaseLine();

    // 设置子图的开始位置（鼠标点击的坐标）和子图中数据条目
    bool SetSubPlotInfo(const QPointF &point, int recordCount = 100);

    void SetSubPlotPoint(const QPointF &pos);
    QPointF GetSubPlotPoint() const;
    inline bool IsSubPlot() const
    {
        return m_plotType == PLOT_TYPE::PLOT_2D_SUB ||
               m_plotType == PLOT_TYPE::PLOT_2D_HIS_SUB ||
               m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS ||
               m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME ||
               m_plotType == PLOT_TYPE::PLOT_3D_HIS_POS ||
               m_plotType == PLOT_TYPE::PLOT_3D_HIS_TIME;
    }

    void Destory();

    void replot();

    friend DataReceiver;
    friend ThumbnailPlot;
    friend MyGraphicsScene;

public slots:
    void DataIn();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    void Create2DPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);
    void Create3DPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);

private:
    MyGraphicsItem() = delete;
    MyGraphicsScene *m_scene;
    QWidget *m_parent;
    ThumbnailPlot *m_view;
    QCPColorMap *m_color;
    QCPColorScale *m_colorScale;
    QCPMarginGroup *m_marginGroup;
    int m_yPos;                             // 当前item在左侧scene中的纵向偏移距离
    bool m_viewCreated;

    int m_xStart;
    int m_xEnd;
    int m_yStart;
    int m_yEnd;
    int m_y2Start;                              // 选中基线后使用
    int m_y2End;                                // 选中基线后使用
    QString m_xName;
    QString m_yName;

    QVector<QCPGraph*> m_multilineGraphs;   // 同时显示的线
    bool m_multilineMode;                   // 是否余晖模式
    int m_multilineCount;                   // 余晖模式中最多同时显示的线条数
    int m_curMulLineCount;                  // 余晖模式中当前的线条数
    QQueue<QVector<double>> m_xDatas;
    QQueue<QVector<double>> m_yDatas;
    QVector<qint64> m_times;                // 保存历史主图中各条线的时间点
    QVector<double> m_xBase;
    QVector<double> m_yBase;
    QCPGraph *m_baseGraph;
    QMutex m_multilineLock;
    int m_baseLineIndex;                    // 余晖模式下选定基线的index
    int m_highLightLineIndex;               // 当前点击的线

    QPointF m_startPoint;                   // 子图开始记录的点坐标
    int m_recordCount;                      // 子图中此点的数据最大条目
    QVector<double> m_recordX;              // 2D子图中x轴的数据
    QVector<double> m_recordY;              // 2D子图中y轴的数据
    QQueue<QVector<double>*> m_recordZ;     // 3D子图中z轴的数据

    MyCustomPlot *m_plot;                   // 2D放大图
    ColorMap *m_colorMap;                   // 3D放大图
    bool m_hidePlot;                        // 是否隐藏放大图
    PLOT_TYPE m_plotType;

    int m_keySize;
    int m_valueSize;

    bool m_hisHasData;                      // 3D历史数据时，是否已经第一条数据进来

    DataReceiver *m_receiver;
    //WorkerThread *m_worker;
};

#endif // MYGRAPHICSITEM_H
