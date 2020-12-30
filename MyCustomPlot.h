
#ifndef MYCUSTOMPLOT_H
#define MYCUSTOMPLOT_H
/**@file MyCustomPlot.h-文件名
* @brief      2D显示
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

    /// 更新主图数据
    void UpdateData(QVector<double> &xData, QVector<double> &yData, qint64 time=0);
    /// 主要用于更新子图（单个点的数据）
    void UpdateData(double xData, double yData);
    /// 设置地图模式的经纬度（线路不变，更新一次就好了）
    void SetLonLats(const QVector<double> &lons, const QVector<double> &lats);
    /// 更新告警信息
    void UpdateWarnings(const QVector<double> &warnings);

    /// 设置百度地图中初始中心位置（如，广州、桂林等）
    void SetBaiduMapCenter(const QString &centerName);
    /// 设置可执行程序的路径（加载百度地图的html文件用）
    void SetAppPath(const QString &appPath) override;

    /// 通过地图上点击位置的经纬度获取此位置的测量值
    QString GetMeasureDataOfPos(double lon, double lat) override;

    /**
     * @brief SetRange
     * 设置x轴和y轴的范围
     * @param xStart
     * @param xEnd
     * @param yStart
     * @param yEnd
     */
    void SetRange(qreal xStart, qreal xEnd, qreal yStart, qreal yEnd);

    /**
     * @brief SetXYLable
     * 设置x轴和y轴为对应轴
     * @param xName
     * @param yName
     */
    void SetXYLable(const QString &xName, const QString &yName);

    /**
     * @brief SetYLabel
     * 设置获取y轴是时间
     * @param yName
     */
    void SetYLabel(const QString &yName);

    /**
     * @brief CreateCrosswiseLine
     * 创建的交叉智慧线
     * @param pos
     */
    void CreateCrosswiseLine(const QPoint &pos);

    /**
     * @brief CreateLengthwiseLine
     * 创建智慧线长度
     * @param pos
     */
    void CreateLengthwiseLine(const QPoint &pos);

    /**
     * @brief ClearLines
     * 清除线
     */
    void ClearLines();

    /**
     * @brief ZoomReset
     * 快速重置
     */
    void ZoomReset();

    /**
     * @brief InterMapMode
     * 进入地图模式
     */
    void InterMapMode();

    /**
     * @brief NewSubView
     * 新子图
     * @param pos
     */
    void NewSubView(const QPointF &pos);

    /**
     * @brief SetSubPlotInfo
     * 子图设置的标志
     * @param point
     * @param recordCount
     * @return
     *- false  设置失败
     *- true   设置成功
     */
    bool SetSubPlotInfo(const QPointF &point, int recordCount = 100);

    /**
     * @brief SetRelativeItem
     * 设置相对条目
     * @param item
     */
    void SetRelativeItem(MyGraphicsItem *item);

    /**
     * @brief show
     * 界面显示
     */
    void show();

    /**
     * @brief hide
     * 界面隐藏
     */
    void hide();

    /**
     * @brief replot
     * 重新绘制界面
     */
    void replot();

    /**
     * @brief SetAxisRangeByUser
     * 用户设置坐标轴的范围
     * @param axisName
     * @param axisIndex
     */
    void SetAxisRangeByUser(const QString &axisName, int axisIndex);

    /**
     * @brief ExitBaiduMap
     * 退出地图模式
     */
    virtual void ExitBaiduMap() override;

    /**
    * @brief  仅应用于历史信息观察模式
    * @return
    * -false  进入观察模式失败
    * -true   进入观察模式成功
    */
    bool SetMultiLineCount(int count);
    /**
    * @brief 生成历史信息子图的数据
    */
    void BuildHisSubPlotDatas(const QPointF &pos, QVector<double> &xDatas, QVector<double> &yDatas);

    friend MySelectMemu;///<MyCustomPlot为友元函数
public slots:
    /**
     * @brief setAxisRange
     * 设置轴范围
     * @param range
     */
    void setAxisRange(const QCPRange &range);

protected:

    /**
    * @brief 鼠标点击事件
    */
    void mousePressEvent(QMouseEvent *event) override;

    /**
    * @brief 鼠标拖动事件
    */
    void mouseMoveEvent(QMouseEvent *event) override;

    /**
    * @brief 鼠标释放事件
    */
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
    * @brief 按键点击事件
    */
    void keyPressEvent(QKeyEvent *event) override;

    /**
    * @brief 按键释放事件
    */
    void keyReleaseEvent(QKeyEvent *event) override;

    /**
    * @brief 转动事件
    */
    void wheelEvent(QWheelEvent *event) override;

private:
    struct ChoosePoint;                                                         ///< 选择点
    void CreateTextDisplayer(bool xDirection);                                  ///< 创建文本显示
    void AdjustTextDisplayerPos();                                              ///< 调整文本显示
    bool ChoosePoints(const QPointF &pos);                                      ///< 选择点成功标志位
    bool SetHighLightLine(const QPointF &pos);                                  ///< 设置突出线成功标注位
    bool GetPrevPointOnLine();                                                  ///< 获得上一个点成功标志位
    bool GetNextPointOnLine();                                                  ///< 获得下一个点成功标志位
    void MapXPoints();                                                          ///< 地图x点
    void AddChoosePointsGraph();                                                ///< 添加选择点图
    void RemoveChoosePointsGraph();                                             ///< 移除选择点图
    void GetChoosePointsData(QVector<double> &x, QVector<double> &y);           ///< 获得选择点数据
    bool ShowPressedChoosePoint(const QPointF &point);                          ///< 显示按下选择点图
    void UpdateChoosePointsGraph();                                             ///< 更新选择点图
    void InterMultilineMode();                                                  ///< 进入多线模式
    void ExitMultilineMode();                                                   ///< 退出多线模式
    void SetBaseLine(int index);                                                ///< 设置基础线
    void RemoveBaseLine();                                                      ///< 移除基础线
    void InterChoosePointMode();                                                ///< 进入选择点模式
    void ExitChoosePointMode();                                                 ///< 退出选择点模式
    void ClearAllChoosedPoints();                                               ///< 清空所有选择点
    void ShowMenu();                                                            ///< 显示菜单
    void ExitMenu();                                                            ///< 退出菜单
    bool CanChooseBaseLine();                                                   ///< 判断选择基础线状态
    bool CanEnterBaiduMap();                                                    ///< 判断输入百度地图状态

    /**
     * @brief IsMainView
     * 判断主界面状态
     * @return
     */
    virtual bool IsMainView() {
        return ((PLOT_TYPE::PLOT_2D_MAIN == m_plotType) || (PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType));
    }
    /**
     * @brief The ChoosePoint struct
     * 取点属性
     */
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

private:
    MyGraphicsScene *m_scene;                        ///< 自定义场景
    QWidget *m_parentWidget;                         ///< 父类窗口
    PLOT_TYPE m_plotType;                            ///< 图的类型
    int m_xStart;                                    ///< 所选择纵向范围的左端
    int m_xEnd;                                      ///< 所选择纵向范围的右端
    int m_yStart;                                    ///< 所选择横向范围的上端
    int m_yEnd;                                      ///< 所选择横线范围的下端
    int m_y2Start;                                   /// 选中基线后使用
    int m_y2End;                                     /// 选中基线后使用
    double m_preX;                                   ///< 原始时域相位数据，用于语音增强处理
    QString m_xName;                                 ///< 所选择纵线的名称
    QString m_yName;                                 ///< 所选择横线的名称
    int m_multilineCount;                            ///< 多线总数
    QVector<QCPGraph*> m_multilineGraphs;            ///< 多线图
    QVector<QCPGraph*> m_mulWarnGraphs;              ///< 多经过图
    int m_curMulLineCount;                           ///< 现在多线总数
    int m_curMulWarnCount;                           ///< 现在错误总数
    bool m_multilineMode;                            ///< 多线程总线
    QQueue<QVector<double>> m_xDatas;                ///< 保存各条线x轴的数据
    QQueue<QVector<double>> m_yDatas;                ///< 保存各条线y轴的数据
    QVector<qint64> m_times;                         ///< 保存历史主图中各条线的时间点
    QVector<double> m_xBase;                         ///< x线的基础
    QVector<double> m_yBase;                         ///< Y线的基础
    QCPGraph *m_baseGraph;                           ///< 基础图
    //QQueue<QVector<double>> m_xWarns;           ///< 保存各条线的告警x轴数据
    //QQueue<QVector<double>> m_yWarns;           ///< 保存各条线的告警y轴数据
    QVector<double> m_xWarns;                   ///< 保存告警x轴数据
    QVector<double> m_yWarns;                   ///< 保存告警y轴数据
    QMutex m_multilineLock;                     ///< 锁定多线程数据
    int m_baseLineIndex;                        ///< 余晖模式下选定基线的index
    int m_highLightLineIndex;                   ///< 当前点击的线

    QPair<double, QCPGraph*> *m_xLines[2];      ///< 横线
    QPair<double, QCPGraph*> *m_yLines[2];      ///< 纵线
    QPointF m_point1;                           ///< 第一根线当时的鼠标位置
    QPointF m_point2;                           ///< 第二根线当时的鼠标位置
    QLabel *m_label;                            ///< 两根线建的显示文字

    QPointF m_startPoint;                       ///< 子图开始记录的点坐标
    int m_recordCount;                          ///< 子图中此点的数据最大条目
    QVector<double> m_recordX;                  ///< 子图中x轴的数据
    QVector<double> m_recordY;                  ///< 子图中y轴的数据

    MyGraphicsItem *m_item;                     ///< 关联的左侧小图
    bool m_hide;                                ///< 是否隐藏自己
    BaiduMap *m_baiduMap;                       ///<  百度地图界面
    QVector<double> m_lons;                     ///<  经度
    QVector<double> m_lats;                     ///<  纬度
    bool m_isBaiduMap;                          ///<  判断是否是地图界面

    QRubberBand * m_rubberBand;                 ///< 矩形区域zoom
    QPoint m_origin;                            ///< 矩形区域开始点

    bool m_pauseDataUpdate;                     ///< 暂停数据显示更新
    bool m_dragMode;                            ///< 拖动模式
    QPoint* m_dragPos;                          ///< 拖动起始点

    bool m_choosePointMode;                     ///< 取点模式
    QVector<QVector<struct ChoosePoint*>*> m_points;///< 保存曲线上的取点
    struct ChoosePoint *m_highLightPoint;       ///< 当前被鼠标点击的取点
    QCPGraph *m_choosePointsGraph;              ///< 显示取点的图层
    bool m_isShiftPressed;                      ///< 按住shift键
    bool m_showMenu;                            ///< 右键菜单显示中

    QString m_baiduMapCenter;                   ///< 百度地图中初始中心
    bool m_isWheeled;                           ///< 鼠标滚轮有没有滚动过（如果滚动后，拖动模式下只拖动，不改变x，y轴范围）
};

#endif // MYCUSTOMPLOT_H
