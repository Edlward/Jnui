#ifndef MYGRAPHICSITEM_H
#define MYGRAPHICSITEM_H

/**@file MyGraphicsItem.h-文件名
* @brief      缩略图框架的设置和和关联
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

#include <QGraphicsItem>
#include <QObject>
#include <QQueue>
//#include "WorkerThread.h"
#include "qcustomplot.h"
#include "Common.h"
#include "ColorMap.h"

#pragma execution_character_set("utf-8")///<指示char的执行字符集是UTF-8编码。

//QT_CHARTS_USE_NAMESPACE

/**
* @brief MyGraphicsItem
* 2D图表
*/
class TwoDimensionChartView;

/**
* @brief MyGraphicsItem
* 主要用来显示视图
*/
class MainWindow;

/**
* @brief DataReceiver
* 主要用来数据接收和处理线程类
*/
class DataReceiver;

/**
* @brief jnui_global
* 主要用来传输与调用数据
*/
struct DBusData;

/**
* @brief MyCustomPlot
* 3D图表
*/
class MyCustomPlot;

/**
* @brief ThumbnailPlot
* 缩略图呈现
*/
class ThumbnailPlot;

/**
* @brief MyGraphicsScene
* 自定义图表
*/
class MyGraphicsScene;

class MyGraphicsItem : public QObject, public QGraphicsItem
{
Q_OBJECT
Q_INTERFACES(QGraphicsItem)

public:
    /**
    * @brief MyCustomPlot类构造函数
    * @param[in] y_pos为int类型,MyGraphicsScene *scene指定所继承的父类, parent为主窗体的this指针，type为PLOT_TYPE类型
    */
    MyGraphicsItem(int y_pos, MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);
    virtual ~MyGraphicsItem();

    /**
    * @brief
    * 显示主界面
    */
    void ShowMain();

    /**
     * @brief HideMain
     * 隐藏主界面
     */
    void HideMain();

    /**
     * @brief boundingRect
     * 跳回坐标
     * @return
     */
    QRectF boundingRect() const override;

    /**
     * @brief SetXYLable
     * 设置XY的标签
     * @param xName
     * @param yName
     */
    void SetXYLable(const QString &xName, const QString &yName/*,
                    const QString &xFormat = QString("%d"),
                    const QString &yFormat = QString("%d")*/);

    /**
     * @brief SetXYRange
     * 设置XY的范围
     * @param xStart
     * @param xEnd
     * @param yStart
     * @param yEnd
     */
    void SetXYRange(qreal xStart, qreal xEnd, qreal yStart, qreal yEnd);

    /**
     * @brief SetXRange
     * 设置X的范围
     * @param xStart
     * @param xEnd
     */
    void SetXRange(qreal xStart, qreal xEnd);

    /**
     * @brief SetYRange
     * 设置Y的范围
     * @param yStart
     * @param yEnd
     */
    void SetYRange(qreal yStart, qreal yEnd);

    /**
     * @brief SetY2Range
     * 设置Y2的范围
     * @param yStart
     * @param yEnd
     */
    void SetY2Range(qreal yStart, qreal yEnd);

    /**
     * @brief SetXLabel
     * 设置X的标签
     * @param xName
     */
    void SetXLabel(const QString &xName);

    /**
     * @brief SetYLabel
     * 设置Y的标签
     * @param yName
     */
    void SetYLabel(const QString &yName);

    /**
     * @brief UpdateData
     * 更新2D主图
     * @param xData
     * @param yData
     * @param time
     */
    void UpdateData(QVector<double> &xData, QVector<double> &yData, qint64 time=0);

    /**
     * @brief UpdateData
     * 更新2D子图（单个点的数据）
     * @param xData
     * @param yData
     */
    void UpdateData(double xData, double yData);

    /**
     * @brief UpdateData
     * 更新3D主图
     * @param mapData
     */
    void UpdateData(QCPColorMapData *mapData);

    /**
     * @brief UpdateData
     * 更新3D子图
     * @param zData
     */
    void UpdateData(QVector<double> &zData);

    /**
     * @brief UpdateData
     * 更新3维主图
     * @param xData
     * @param yData
     * @param zData
     * @param mapData
     * @param time
     */
    void UpdateData(const QVector<double> &xData, const QVector<double> &yData,
                    const QVector<double> &zData, QCPColorMapData *mapData, qint64 time=0);

    /**
     * @brief UpdateWarnings
     * 更新告警信息
     * @param warnings
     */
    void UpdateWarnings(const QVector<double> &warnings);

    /**
     * @brief SetLonLats
     * 设置线段颜色
     * @param lons
     * @param lats
     */
    void SetLonLats(const QVector<double> &lons, const QVector<double> &lats);

    /**
     * @brief SetMultiLineCount
     * 设置线的数量
     * @param count
     * @return
     * -false  设置失败
     * -true   设置成功
     */
    bool SetMultiLineCount(int count); // 仅应用于历史信息观察模式

    /**
     * @brief SetBaiduMapCenter
     * 设置百度地图初始地点
     * @param centerName
     */
    void SetBaiduMapCenter(const QString &centerName); // 设置百度地图中初始中心位置（如，广州、桂林等）

    /**
     * @brief SetAppPath
     * 设置可执行程序的路径（加载百度地图的html文件用）
     * @param appPath
     */
    void SetAppPath(const QString &appPath);

    /**
     * @brief SetKeySize
     * 设置图例大小
     * @param keySize
     */
    inline void SetKeySize(int keySize)
    {
        m_keySize = keySize;
        m_colorMap->SetKeySize(keySize);
    }

    /**
     * @brief SetValueSize
     * 设置值大小
     * @param valueSize
     */
    inline void SetValueSize(int valueSize)
    {
        m_valueSize = valueSize;
        m_colorMap->SetValueSize(valueSize);
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

        if (m_colorMap) {
            m_colorMap->SetColorScaleRange(lower, uper);
        }
    }

    /**
     * @brief SetColorScalePolicy
     * 设置色阶规则
     * @param gradient
     */
    void SetColorScalePolicy(const QCPColorGradient &gradient)
    {
        if (m_color) {
            m_color->setGradient(gradient);
        }

        if (m_colorMap) {
            m_colorMap->SetColorScalePolicy(gradient);
        }
    }

    /**
     * @brief resize
     * @param wigth
     * @param height
     */
    void resize(qreal wigth, qreal height);

    /**
     * @brief SetTitle
     * @param name
     */
    void SetTitle(const QString &name);

    /**
     * @brief ResetY
     * @param y_pos
     */
    void ResetY(int y_pos);

    /**
     * @brief ReDraw
     * @param x
     * @param y
     */
    void ReDraw(int x, int y);

    /**
     * @brief SetFocus
     */
    void SetFocus();

    /**
     * @brief RemoveFocus
     */
    void RemoveFocus();

    /**
     * @brief InterMultilineMode
     * @param multilineCount
     */
    void InterMultilineMode(int multilineCount);

    /**
     * @brief ExitMultilineMode
     */
    void ExitMultilineMode();

    /**
     * @brief SetBaseLine
     * @param index
     */
    void SetBaseLine(int index);

    /**
     * @brief RemoveBaseLine
     */
    void RemoveBaseLine();

    /// 设置子图的开始位置（鼠标点击的坐标）和子图中数据条目
    /**
     * @brief SetSubPlotInfo
     * @param point
     * @param recordCount
     * @return
     */
    bool SetSubPlotInfo(const QPointF &point, int recordCount = 100);

    /**
     * @brief SetSubPlotPoint
     * @param pos
     */
    void SetSubPlotPoint(const QPointF &pos);

    /**
     * @brief GetSubPlotPoint
     * @return
     */
    QPointF GetSubPlotPoint() const;

    /**
     * @brief IsSubPlot
     * @return
     */
    inline bool IsSubPlot() const
    {
        return m_plotType == PLOT_TYPE::PLOT_2D_SUB ||
               m_plotType == PLOT_TYPE::PLOT_2D_HIS_SUB ||
               m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS ||
               m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME ||
               m_plotType == PLOT_TYPE::PLOT_3D_HIS_POS ||
               m_plotType == PLOT_TYPE::PLOT_3D_HIS_TIME;
    }

    /**
     * @brief Destory
     * 析构函数
     */
    void Destory();

    /**
     * @brief replot
     * 重新呈现
     */
    void replot();

    /**
     * @brief show
     * 显示图表
     */
    void show();


    friend DataReceiver;///<设置DataReceiver为友元
    friend ThumbnailPlot; ///<设置ThumbnailPlot为友元
    friend MyGraphicsScene;///<设置MyGraphicsScene为友元

public slots:
    /**
     * @brief DataIn
     * 用以连接DataReceiver
     */
    void DataIn();

protected:
    /**
     * @brief paint
     * 绘制
     * @param painter
     * @param option
     * @param widget
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    /**
     * @brief Create2DPlot
     * 创建2D图表
     * @param scene
     * @param parent
     * @param type
     */
    void Create2DPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);

    /**
     * @brief Create3DPlot
     * 创建3D图表
     * @param scene
     * @param parent
     * @param type
     */
    void Create3DPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type);

private:

    /**
     * @brief MyGraphicsItem
     * 将MyGraphicsItem()函数定义成已删除的函数
     */
    MyGraphicsItem() = delete;              ///< 自定义场景
    MyGraphicsScene *m_scene;               ///< 父类窗口
    QWidget *m_parent;                      ///< 缩略图绘图
    ThumbnailPlot *m_view;                  ///< 色谱图
    QCPColorMap *m_color;                   ///< 设置色条
    QCPColorScale *m_colorScale;            ///< 图边缘
    QCPMarginGroup *m_marginGroup;          ///< 当前item在左侧scene中的纵向偏移距离
    int m_yPos;                             /// 当前item在左侧scene中的纵向偏移距离
    bool m_viewCreated;                     ///< 判断视图创建状态

    int m_xStart;                           ///< 所选择纵向范围的左端
    int m_xEnd;                             ///< 所选择纵向范围的右端
    int m_yStart;                           ///< 所选择横向范围的上端
    int m_yEnd;                             ///< 所选择横线范围的下端
    int m_y2Start;                          // 选中基线后使用
    int m_y2End;                            // 选中基线后使用
    QString m_xName;                        ///< 所选择纵线的名称
    QString m_yName;                        ///< 所选择横线的名称

    QVector<QCPGraph*> m_multilineGraphs;   /// 同时显示的线
    bool m_multilineMode;                   ///是否余晖模式
    int m_multilineCount;                   /// 余晖模式中最多同时显示的线条数
    int m_curMulLineCount;                  /// 余晖模式中当前的线条数
    QQueue<QVector<double>> m_xDatas;       ///< 保存各条线x轴的数据
    QQueue<QVector<double>> m_yDatas;       ///< 保存各条线y轴的数据
    QVector<qint64> m_times;                /// 保存历史主图中各条线的时间点
    QVector<double> m_xBase;                ///< x线的基础
    QVector<double> m_yBase;                ///< Y线的基础
    QCPGraph *m_baseGraph;                  ///< 基础图
    QMutex m_multilineLock;                 ///< 多线程数据锁定
    int m_baseLineIndex;                    /// 余晖模式下选定基线的index
    int m_highLightLineIndex;               ///当前点击的线

    QPointF m_startPoint;                   /// 子图开始记录的点坐标
    int m_recordCount;                      /// 子图中此点的数据最大条目
    QVector<double> m_recordX;              /// 2D子图中x轴的数据
    QVector<double> m_recordY;              /// 2D子图中y轴的数据
    QQueue<QVector<double>*> m_recordZ;     /// 3D子图中z轴的数据

    MyCustomPlot *m_plot;                   /// 2D放大图
    ColorMap *m_colorMap;                   /// 3D放大图
    bool m_hidePlot;                        /// 是否隐藏放大图
    PLOT_TYPE m_plotType;

    int m_keySize;                          ///< 图例大小
    int m_valueSize;                        ///< 数值大小

    bool m_hisHasData;                      /// 3D历史数据时，是否已经第一条数据进来

    DataReceiver *m_receiver;               ///< 数据接收和处理线程数组容器
    //WorkerThread *m_worker;
};

#endif // MYGRAPHICSITEM_H
