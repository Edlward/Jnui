#ifndef COLORMAP_H
#define COLORMAP_H

#pragma execution_character_set("utf-8")

#include "qcustomplot.h"
#include <QObject>
#include "Common.h"
#include "CustomPlotInterface.h"

class MainWindow;
class MyGraphicsItem;
class MyGraphicsScene;
class BaiduMap;

class ColorMap : public QCustomPlot, public CustomPlotInterface
{
    Q_OBJECT
public:
    ColorMap(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);
    virtual ~ColorMap();

    // 更新主图数据
    void UpdateData(QCPColorMapData *mapData);
    // 主要用于更新子图
    void UpdateData(QVector<double> &zData);
    // 更新主图数据
    void UpdateData(const QVector<double> &xData, const QVector<double> &yData,
                    const QVector<double> &zData, QCPColorMapData *mapData, qint64 time=0);

    void SetLonLats(const QVector<double> &lons, const QVector<double> &lats);
    // 更新告警信息
    void UpdateWarnings(const QVector<double> &warnings);

    // 设置百度地图中初始中心位置（如，广州、桂林等）
    void SetBaiduMapCenter(const QString &centerName);
    // 设置可执行程序的路径（加载百度地图的html文件用）
    void SetAppPath(const QString &appPath) override;

    // 通过地图上点击位置的经纬度获取此位置的测量值
    QString GetMeasureDataOfPos(double lon, double lat) override;

    void SetRange(double xStart, double xEnd, double yStart, double yEnd);
    void SetXYLable(const QString &xName, const QString &yName);
    void SetXLabel(const QString &xName);
    void SetYLabel(const QString &yName);

    inline void SetKeySize(int keySize)
    {
        m_keySize = keySize;
    }
    inline void SetValueSize(int valueSize)
    {
        m_valueSize = valueSize;
    }

    void SetColorScaleRange(double lower, double uper)
    {
        if (m_colorScale) {
            m_colorScale->setDataRange(QCPRange(lower, uper));
        }
    }

    void SetColorScalePolicy(const QCPColorGradient &gradient)
    {
        if (m_colorMap) {
            m_colorMap->setGradient(gradient);
        }
    }

    bool SetSubPlotInfo(const QPointF &point, int recordCount = 100);

    void SetRelativeItem(MyGraphicsItem *item);
    void show();
    void hide();
    virtual void ExitBaiduMap() override;

    QPointF GetRecordPoint() const
    {
        return m_startPoint;
    }

    bool IsMainView() const
    {
        return (PLOT_TYPE::PLOT_3D_MAIN == m_plotType || PLOT_TYPE::PLOT_3D_HIS_MAIN == m_plotType);
    }

    bool IsHisView() const
    {
        return (PLOT_TYPE::PLOT_3D_HIS_POS == m_plotType ||
                PLOT_TYPE::PLOT_3D_HIS_MAIN == m_plotType ||
                PLOT_TYPE::PLOT_3D_HIS_TIME == m_plotType);
    }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    // void wheelEvent(QWheelEvent *event) override;

private slots:
   void SetAxisRange(QCPRange);

private:
   void PlayPCM();
    void CreateTextDisplayer(bool xDirection);
    void AdjustTextDisplayerPos();
    void SetAxisRangeByUser(const QString &axisName, int axisIndex);
    void AddChoosePointsGraph();
    void RemoveChoosePointsGraph();
    void GetChoosePointsData(QVector<double> &x, QVector<double> &y);
    bool ShowPressedChoosePoint(const QPointF &point);
    void UpdateChoosePointsGraph();
    bool ChoosePoints(const QPointF &pos);
    bool GetPrevPointOnLine();
    bool GetNextPointOnLine();
    void ShowMenu(QMouseEvent *event);
    void ClearAllChoosedPoints();
    void BuildTimeSubViewDatas(const QPointF &pos, MyGraphicsItem *item);
    void BuildPosSubViewDatas(const QPointF &pos, MyGraphicsItem *item);

    struct ChoosePoint {
        int lineIndex;
        int pointIndex;
        QPointF point;
        double zData;

        ChoosePoint()
            : lineIndex(0), pointIndex(0), zData(0) {}
        ChoosePoint(int l, int p, const QPointF &pos, double z)
            : lineIndex(l), pointIndex(p), zData(z)
        {
            point.setX(pos.x());
            point.setY(pos.y());
        }
    };

private:
    MyGraphicsScene *m_scene;
    QWidget *m_parentWidget;
    PLOT_TYPE m_plotType;
    int m_xStart;
    int m_xEnd;
    int m_yStart;
    int m_yEnd;
    double m_preX;
    QString m_xName;
    QString m_yName;
    QQueue<QVector<double>> m_xDatas;
    QQueue<QVector<double>> m_yDatas;
    QQueue<QVector<double>> m_zDatas;
    QVector<qint64> m_times;
    QCPGraph *m_warnGraph;

    QPair<double, QCPGraph*> *m_xLines[2];      // 横线
    QPair<double, QCPGraph*> *m_yLines[2];      // 纵线
    QPointF m_point1;                           // 第一根线当时的鼠标位置
    QPointF m_point2;                           // 第二根线当时的鼠标位置
    QLabel *m_label;                            // 两根线建的显示文字

    QPointF m_startPoint;                       // 子图开始记录的点坐标
    int m_recordCount;                          // 子图中此点的数据最大条目
    QQueue<QVector<double>*> m_recordZ;         // 子图中z轴的数据

    MyGraphicsItem *m_item;                     // 关联的左侧小图
    bool m_hide;                                // 是否隐藏自己

    QCPColorMap *m_colorMap;
    QCPColorScale *m_colorScale;
    QCPMarginGroup *m_marginGroup;

    int m_keySize;
    int m_valueSize;

    BaiduMap *m_baiduMap;
    QVector<double> m_lons;
    QVector<double> m_lats;
    bool m_isBaiduMap;

    QRubberBand * m_rubberBand;                 // 矩形区域zoom
    QPoint m_origin;                            // 矩形区域开始点

    bool m_pauseDataUpdate;                     // 暂停数据显示更新
    bool m_choosePointMode;                     // 取点模式
    QVector<struct ChoosePoint*> m_points;      // 保存曲线上的取点
    struct ChoosePoint *m_highLightPoint;       // 当前被鼠标点击的取点
    QCPGraph *m_choosePointsGraph;              // 显示取点的图层
    bool m_isShiftPressed;                      // 按住shift键
    bool m_showMenu;                            // 右键菜单显示中

    QString m_baiduMapCenter;                   // 百度地图中初始中心
    // bool m_isWheeled;                           // 鼠标滚轮有没有滚动过（如果滚动后，拖动模式下只拖动，不改变x，y轴范围）
    bool m_dragMode;                            // 拖动模式
    QPoint* m_dragPos;                          // 拖动起始点
};

#endif // COLORMAP_H
