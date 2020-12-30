#ifndef COLORMAP_H
#define COLORMAP_H


/**@file CoLorMap.h-文件名
* @brief      3D显示
* @details
* @mainpage   工程概览
* @author     lzy
* @email      jnuerlzy@gmail.com
* @version    V1.0
* @date       2020-12-22
* @copyright  Copyright (c) 2020-2022 广州凌远技术有限公司
************************************************************
* @attention
* 硬件平台：xx   ;
* 软件版本： Qt5.12.3 msvc2017 64bit
* @par 修改日志
* <table>
* <tr><th>Date        <th>Version  <th>Author  <th>Description
* <tr><td>2020/11/30  <td>1.0      <td>lzy     <td>初始版本
* </table>
*
************************************************************
*/

#pragma execution_character_set("utf-8")///<指示char的执行字符集是UTF-8编码。

#include "qcustomplot.h"
#include <QObject>
#include "Common.h"
#include "CustomPlotInterface.h"

class MainWindow;///<显示主界面
class MyGraphicsItem;///<显示图形条目
class MyGraphicsScene;///<显示图形场景
class BaiduMap;///<显示地图界面

class ColorMap : public QCustomPlot, public CustomPlotInterface
{
    Q_OBJECT
public:
    /**
     * @brief ColorMap类构造函数
     * @param scene   指定所继承的父类
     * @param parent  主窗体的this指针
     * @param type    PLOT_TYPE类型
     */
    ColorMap(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);
    virtual ~ColorMap();

    /**
     * @brief UpdateData
     * 更新主图数据
     * @param mapData
     */
    void UpdateData(QCPColorMapData *mapData);

    /**
     * @brief UpdateData
     * 更新子图
     * @param zData
     */
    void UpdateData(QVector<double> &zData);

    /**
     * @brief UpdateData
     * 更新主图数据
     * @param xData
     * @param yData
     * @param zData
     * @param mapData
     * @param time
     */
    void UpdateData(const QVector<double> &xData, const QVector<double> &yData,
                    const QVector<double> &zData, QCPColorMapData *mapData, qint64 time=0);

    /**
     * @brief SetLonLats
     * @param lons
     * @param lats
     */
    void SetLonLats(const QVector<double> &lons, const QVector<double> &lats);
    // 更新告警信息
    /**
     * @brief UpdateWarnings
     * 更新告警信息
     * @param warnings
     */
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

    /**
     * @brief SetColorScaleRange
     * 设置色阶范围
     * @param lower
     * @param uper
     */
    void SetColorScaleRange(double lower, double uper)
    {
        if (m_colorScale) {
            m_colorScale->setDataRange(QCPRange(lower, uper));
        }
    }

    /**
     * @brief SetColorScalePolicy
     * 设置色阶策略
     * @param gradient
     */
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

    /**
     * @brief GetRecordPoint
     * 获取历史规则
     * @return
     */
    QPointF GetRecordPoint() const
    {
        return m_startPoint;
    }

    /**
     * @brief IsMainView
     * 主界面画面
     * @return
     */
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
   /**
     * @brief PlayPCM
     * 调制脉冲
     */
    void PlayPCM();

    /**
     * @brief CreateTextDisplayer
     * 两条线之间距离的显示文本
     * @param xDirection
     */
    void CreateTextDisplayer(bool xDirection);

    /**
     * @brief AdjustTextDisplayerPos
     * 调整两线之间的距离的显示
     */
    void AdjustTextDisplayerPos();

    /**
     * @brief SetAxisRangeByUser
     * 由用户设置轴范围
     * @param axisName
     * @param axisIndex
     */
    void SetAxisRangeByUser(const QString &axisName, int axisIndex);

    /**
     * @brief AddChoosePointsGraph
     * 增加所选择点图
     */
    void AddChoosePointsGraph();

    /**
     * @brief RemoveChoosePointsGraph
     * 移除所选择点图
     */
    void RemoveChoosePointsGraph();

    /**
     * @brief GetChoosePointsData
     * 选择点图数据
     * @param x
     * @param y
     */
    void GetChoosePointsData(QVector<double> &x, QVector<double> &y);

    /**
     * @brief ShowPressedChoosePoint
     * 显示所按下点图的信息的标志
     * @param point
     * @return
     */
    bool ShowPressedChoosePoint(const QPointF &point);

    /**
     * @brief UpdateChoosePointsGraph
     * 更新选择点图
     */
    void UpdateChoosePointsGraph();

    /**
     * @brief ChoosePoints
     * 选择点成功的标志
     * @param pos
     * @return
     * -false  标点失败
     * -true   标点完成
     */
    bool ChoosePoints(const QPointF &pos);

    /**
     * @brief GetPrevPointOnLine
     * 得到历史图在线点成功的标志
     * @return
     *     * - 返回值false  表示得到历史图在线点失败
    * - 返回值true   表示得到历史图在线点成功
     */
    bool GetPrevPointOnLine();

    /**
     * @brief GetNextPointOnLine
     * 得到下一个在线点成功的标志
     * @return
     * -false  得到下一个在线点失败
     * -true   得到下一个在线点成功
     */
    bool GetNextPointOnLine();

    /**
     * @brief ShowMenu
     * 显示菜单
     * @param event
     */
    void ShowMenu(QMouseEvent *event);

    /**
     * @brief ClearAllChoosedPoints
     * 清空所有点
     */
    void ClearAllChoosedPoints();

    /**
     * @brief BuildTimeSubViewDatas
     * 创建时间子视图
     * @param pos
     * @param item
     */
    void BuildTimeSubViewDatas(const QPointF &pos, MyGraphicsItem *item);

    /**
     * @brief BuildPosSubViewDatas
     * 创建位置子视图
     * @param pos
     * @param item
     */
    void BuildPosSubViewDatas(const QPointF &pos, MyGraphicsItem *item);

    /**
     * @brief CanEnterBaiduMap
     * @return
     * -false  进入地图模式失败
     * -true   进入地图模式成功
     */
    bool CanEnterBaiduMap();

    /**
     * @brief The ChoosePoint struct
     * 选择点
     */
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
    MyGraphicsScene *m_scene;                   ///< 自定义场景
    QWidget *m_parentWidget;                    ///< 父类窗口
    PLOT_TYPE m_plotType;                       ///< 图的类型
    int m_xStart;                               ///< 所选择纵向范围的左端
    int m_xEnd;                                 ///< 所选择纵向范围的右端
    int m_yStart;                               ///< 所选择横向范围的上端
    int m_yEnd;                                 ///< 所选择横线范围的下端
    double m_preX;                              ///< 原始时域相位数据，用于语音增强处理
    QString m_xName;                            ///< 所选择纵线的名称
    QString m_yName;                            ///< 所选择横线的名称
    QQueue<QVector<double>> m_xDatas;           ///< 3D中x轴的数据
    QQueue<QVector<double>> m_yDatas;           ///< 3D中y轴的数据
    QQueue<QVector<double>> m_zDatas;           ///< 3D中z轴的数据
    QVector<qint64> m_times;                    ///< 判断模式
    QCPGraph *m_warnGraph;                      ///< 错误图

    QPair<double, QCPGraph*> *m_xLines[2];      ///< 横线
    QPair<double, QCPGraph*> *m_yLines[2];      ///< 纵线
    QPointF m_point1;                           ///<  第一根线当时的鼠标位置
    QPointF m_point2;                           ///<  第二根线当时的鼠标位置
    QLabel *m_label;                            ///<  两根线建的显示文字

    QPointF m_startPoint;                       ///<  子图开始记录的点坐标
    int m_recordCount;                          ///<  子图中此点的数据最大条目
    QQueue<QVector<double>*> m_recordZ;         ///<  子图中z轴的数据

    MyGraphicsItem *m_item;                     ///<  关联的左侧小图
    bool m_hide;                                ///<  是否隐藏自己

    QCPColorMap *m_colorMap;                    ///<  色谱图
    QCPColorScale *m_colorScale;                ///<  设置色条
    QCPMarginGroup *m_marginGroup;              ///<  图边缘

    int m_keySize;                              ///<  图例的大小
    int m_valueSize;                            ///<  值的大小

    BaiduMap *m_baiduMap;                       ///<  百度地图界面
    QVector<double> m_lons;                     ///<  经度
    QVector<double> m_lats;                     ///<  纬度
    bool m_isBaiduMap;                          ///<  判断百度地图界面状态

    QRubberBand * m_rubberBand;                 ///<  矩形区域zoom
    QPoint m_origin;                            ///<  矩形区域开始点

    bool m_pauseDataUpdate;                     ///<  暂停数据显示更新
    bool m_choosePointMode;                     ///<  取点模式
    QVector<struct ChoosePoint*> m_points;      ///<  保存曲线上的取点
    struct ChoosePoint *m_highLightPoint;       ///<  当前被鼠标点击的取点
    QCPGraph *m_choosePointsGraph;              ///<  显示取点的图层
    bool m_isShiftPressed;                      ///<  按住shift键
    bool m_showMenu;                            ///<  右键菜单显示中

    QString m_baiduMapCenter;                   ///<  百度地图中初始中心
    // bool m_isWheeled;                          // 鼠标滚轮有没有滚动过（如果滚动后，拖动模式下只拖动，不改变x，y轴范围）
    bool m_dragMode;                            ///<  拖动模式
    QPoint* m_dragPos;                          ///<  拖动起始点
};

#endif // COLORMAP_H
