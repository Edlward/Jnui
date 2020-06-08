#ifndef MYCUSTOMPLOT_H
#define MYCUSTOMPLOT_H

#pragma execution_character_set("utf-8")

#include "qcustomplot.h"
#include <QVector>
#include <QQueue>
#include <QPair>
#include "Common.h"
#include "MySelectMemu.h"
#include "CustomPlotInterface.h"

class MyGraphicsItem;
class MainWindow;
class MyGraphicsScene;
class BaiduMap;

class MyCustomPlot : public QCustomPlot, public CustomPlotInterface
{
public:
    MyCustomPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);
    virtual ~MyCustomPlot();

    // 更新主图数据
    void UpdateData(QVector<double> &xData, QVector<double> &yData, qint64 time=0);
    // 主要用于更新子图（单个点的数据）
    void UpdateData(double xData, double yData);
    // 设置地图模式的经纬度（线路不变，更新一次就好了）
    void SetLonLats(const QVector<double> &lons, const QVector<double> &lats);
    // 更新告警信息
    void UpdateWarnings(const QVector<double> &warnings);

    // 设置百度地图中初始中心位置（如，广州、桂林等）
    void SetBaiduMapCenter(const QString &centerName);
    // 设置可执行程序的路径（加载百度地图的html文件用）
    void SetAppPath(const QString &appPath) override;

    // 通过地图上点击位置的经纬度获取此位置的测量值
    QString GetMeasureDataOfPos(double lon, double lat) override;

    void SetRange(qreal xStart, qreal xEnd, qreal yStart, qreal yEnd);

    void SetXYLable(const QString &xName, const QString &yName);
    void SetYLabel(const QString &yName);

    void CreateCrosswiseLine(const QPoint &pos);
    void CreateLengthwiseLine(const QPoint &pos);
    void ClearLines();
    void ZoomReset();
    void InterMapMode();
    void NewSubView(const QPointF &pos);
    bool SetSubPlotInfo(const QPointF &point, int recordCount = 100);

    void SetRelativeItem(MyGraphicsItem *item);
    void show();
    void hide();
    void replot();

    void SetAxisRangeByUser(const QString &axisName, int axisIndex);

    virtual void ExitBaiduMap() override;

    // 仅应用于历史信息观察模式
    bool SetMultiLineCount(int count);
    // 生成历史信息子图的数据
    void BuildHisSubPlotDatas(const QPointF &pos, QVector<double> &xDatas, QVector<double> &yDatas);

    friend MySelectMemu;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct ChoosePoint;
    void CreateTextDisplayer(bool xDirection);
    void AdjustTextDisplayerPos();
    bool ChoosePoints(const QPointF &pos);
    bool SetHighLightLine(const QPointF &pos);
    bool GetPrevPointOnLine();
    bool GetNextPointOnLine();
    void MapXPoints();
    void AddChoosePointsGraph();
    void RemoveChoosePointsGraph();
    void GetChoosePointsData(QVector<double> &x, QVector<double> &y);
    bool ShowPressedChoosePoint(const QPointF &point);
    void UpdateChoosePointsGraph();
    void InterMultilineMode();
    void ExitMultilineMode();
    void SetBaseLine(int index);
    void RemoveBaseLine();
    void InterChoosePointMode();
    void ExitChoosePointMode();
    void ClearAllChoosedPoints();
    void ShowMenu();
    void ExitMenu();

    virtual bool IsMainView() {
        return ((PLOT_TYPE::PLOT_2D_MAIN == m_plotType) || (PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType));
    }

    struct ChoosePoint {
        int lineIndex;
        int pointIndex;
        QPointF point;

        ChoosePoint()
            : lineIndex(0), pointIndex(0) {}
        ChoosePoint(int l, int p, const QPointF &pos)
            : lineIndex(l), pointIndex(p)
        {
            point.setX(pos.x());
            point.setY(pos.y());
        }
    };

private slots:
   void SetAxisRange(QCPRange);

private:
    MyGraphicsScene *m_scene;
    QWidget *m_parentWidget;
    PLOT_TYPE m_plotType;
    int m_xStart;
    int m_xEnd;
    int m_yStart;
    int m_yEnd;
    int m_y2Start;                              // 选中基线后使用
    int m_y2End;                                // 选中基线后使用
    double m_preX;
    QString m_xName;
    QString m_yName;
    int m_multilineCount;
    QVector<QCPGraph*> m_multilineGraphs;
    QVector<QCPGraph*> m_mulWarnGraphs;
    int m_curMulLineCount;
    int m_curMulWarnCount;
    bool m_multilineMode;
    QQueue<QVector<double>> m_xDatas;           // 保存各条线x轴的数据
    QQueue<QVector<double>> m_yDatas;           // 保存各条线y轴的数据
    QVector<qint64> m_times;                    // 保存历史主图中各条线的时间点
    QVector<double> m_xBase;
    QVector<double> m_yBase;
    QCPGraph *m_baseGraph;
    //QQueue<QVector<double>> m_xWarns;           // 保存各条线的告警x轴数据
    //QQueue<QVector<double>> m_yWarns;           // 保存各条线的告警y轴数据
    QVector<double> m_xWarns;                   // 保存告警x轴数据
    QVector<double> m_yWarns;                   // 保存告警y轴数据
    QMutex m_multilineLock;
    int m_baseLineIndex;                        // 余晖模式下选定基线的index
    int m_highLightLineIndex;                   // 当前点击的线

    QPair<double, QCPGraph*> *m_xLines[2];      // 横线
    QPair<double, QCPGraph*> *m_yLines[2];      // 纵线
    QPointF m_point1;                           // 第一根线当时的鼠标位置
    QPointF m_point2;                           // 第二根线当时的鼠标位置
    QLabel *m_label;                            // 两根线建的显示文字

    QPointF m_startPoint;                       // 子图开始记录的点坐标
    int m_recordCount;                          // 子图中此点的数据最大条目
    QVector<double> m_recordX;                  // 子图中x轴的数据
    QVector<double> m_recordY;                  // 子图中y轴的数据

    MyGraphicsItem *m_item;                     // 关联的左侧小图
    bool m_hide;                                // 是否隐藏自己

    BaiduMap *m_baiduMap;
    QVector<double> m_lons;
    QVector<double> m_lats;
    bool m_isBaiduMap;

    QRubberBand * m_rubberBand;                 // 矩形区域zoom
    QPoint m_origin;                            // 矩形区域开始点

    bool m_pauseDataUpdate;                     // 暂停数据显示更新
    bool m_dragMode;                            // 拖动模式
    QPoint* m_dragPos;                          // 拖动起始点

    bool m_choosePointMode;                     // 取点模式
    QVector<QVector<struct ChoosePoint*>*> m_points;// 保存曲线上的取点
    struct ChoosePoint *m_highLightPoint;       // 当前被鼠标点击的取点
    QCPGraph *m_choosePointsGraph;              // 显示取点的图层
    bool m_isShiftPressed;                      // 按住shift键
    bool m_showMenu;                            // 右键菜单显示中

    QString m_baiduMapCenter;                   // 百度地图中初始中心
    bool m_isWheeled;                           // 鼠标滚轮有没有滚动过（如果滚动后，拖动模式下只拖动，不改变x，y轴范围）
};

#endif // MYCUSTOMPLOT_H
