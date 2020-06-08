#pragma execution_character_set("utf-8")

#include "MyGraphicsItem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <qlogging.h>
#include <iostream>
#include "DataReceiver.h"
#include <QDateTime>
#include <QThread>
#include "MyCustomPlot.h"
#include "ThumbnailPlot.h"
#include "ColorMap.h"
#include "MyGraphicsScene.h"

MyGraphicsItem::MyGraphicsItem(int _y_pos, MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type)
    : m_scene(scene), m_parent(parent), m_view(nullptr), m_yPos(_y_pos), m_viewCreated(false),
      m_xName(""), m_multilineMode(false), m_multilineCount(1), m_curMulLineCount(0),
      m_baseGraph(nullptr), m_baseLineIndex(-1), m_highLightLineIndex(-1),
      m_hidePlot(false), m_plotType(type), m_keySize(0), m_valueSize(0), m_hisHasData(false)
{
    setPos(0, 0);

    m_xStart = 0;
    m_xEnd = 100;
    m_yStart = 0;
    m_yEnd = 100;
    m_y2Start = 0;
    m_y2End = 100;

    m_colorScale = nullptr;

    // 左侧小图
    m_view = new ThumbnailPlot(this);
    m_view->resize(GRAPHICS_ITEM_WIDTH, GRAPHICS_ITEM_HEIGHT);
    m_view->xAxis->ticker()->setTickCount(8);
    m_view->yAxis->ticker()->setTickCount(8);
    m_view->xAxis2->ticker()->setTickCount(8);
    m_view->yAxis2->ticker()->setTickCount(8);
    m_view->xAxis->setTicks(false);
    m_view->xAxis2->setTicks(false);
    m_view->yAxis->setTicks(false);
    m_view->yAxis2->setTicks(false);
    m_view->xAxis->setSubTicks(false);
    m_view->xAxis2->setSubTicks(false);
    m_view->yAxis->setSubTicks(false);
    m_view->yAxis2->setSubTicks(false);

    Create2DPlot(scene, parent, type);
    Create3DPlot(scene, parent, type);

    m_receiver = new DataReceiver();
    m_receiver->SetDataType(type);
    m_receiver->start();
    m_receiver->SetDataAccepter(this);

    //qRegisterMetaType<DBusData>("DBusData");
    connect(m_receiver, &DataReceiver::DataReady2, this, &MyGraphicsItem::DataIn);

    m_scene->AddItem(this);
}

MyGraphicsItem::~MyGraphicsItem()
{
    delete m_view;
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_SUB:
        delete m_plot;
        break;
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
        delete m_colorMap;
        break;
    default:
        break;
    }

    if (!m_multilineGraphs.empty()) {
        m_multilineGraphs.pop_front();
    }
}

void MyGraphicsItem::Create2DPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type)
{
    if (PLOT_TYPE::PLOT_2D_MAIN != type &&
        PLOT_TYPE::PLOT_2D_SUB != type &&
        PLOT_TYPE::PLOT_2D_HIS_SUB != type &&
        PLOT_TYPE::PLOT_2D_HIS_MAIN != type) {
        return;
    }

    QCPGraph *graph = m_view->addGraph();
    m_curMulLineCount++;
    m_multilineGraphs.append(graph);
    graph->setPen(QPen(Qt::blue));
    m_view->axisRect()->setupFullAxesBox(true);
    graph->rescaleAxes(true);
    QCPTextElement *element = new QCPTextElement(m_view, " ", QFont("微软雅黑", 8));
    m_view->plotLayout()->insertRow(0);
    m_view->plotLayout()->addElement(0, 0, element);
    m_view->plotLayout()->setColumnSpacing(1);
    m_view->plotLayout()->setRowSpacing(1);

    if (PLOT_TYPE::PLOT_2D_HIS_MAIN == type) {
        m_multilineMode = true;
    }

    if (PLOT_TYPE::PLOT_2D_SUB == type || PLOT_TYPE::PLOT_2D_HIS_SUB == type) {
        double time = QDateTime::currentSecsSinceEpoch();
        m_xStart = time;
        m_xEnd = time + DEFALUT_AXIS_STEP;
        m_view->xAxis->setRange(time, time+DEFALUT_AXIS_STEP);
        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);    //日期做X轴
        dateTicker->setDateTimeFormat("yy-MM-dd hh:mm:ss"); //日期格式(可参考QDateTime::fromString()函数)
        m_view->xAxis->setTicker(dateTicker);   //设置X轴为时间轴
        m_recordCount = DEFAULT_2D_RECORD_COUNT;
    }

    m_plot =  new MyCustomPlot(scene, parent, type);
    m_plot->SetRelativeItem(this);
}

void MyGraphicsItem::Create3DPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type)
{
    if (PLOT_TYPE::PLOT_2D_MAIN == type    ||
        PLOT_TYPE::PLOT_2D_SUB == type     ||
        PLOT_TYPE::PLOT_2D_HIS_SUB == type ||
        PLOT_TYPE::PLOT_2D_HIS_MAIN == type) {
        return;
    }

    QCPTextElement *element = new QCPTextElement(m_view, " ", QFont("微软雅黑", 8));
    m_view->plotLayout()->insertRow(0);
    m_view->plotLayout()->addElement(0, 0, element);
    m_view->plotLayout()->setColumnSpacing(1);
    m_view->plotLayout()->setRowSpacing(1);

    m_view->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    m_view->axisRect()->setupFullAxesBox(true);
    m_color = new QCPColorMap(m_view->xAxis, m_view->yAxis);
    m_colorScale = new QCPColorScale(m_view);
    m_colorScale->setDataRange(QCPRange(-120, 40));
    m_colorScale->setBarWidth(10);          // 设置颜色条宽度
    m_colorScale->axis()->setTicks(false);  // 不显示刻度值
    m_view->plotLayout()->addElement(1, 1, m_colorScale);
    m_colorScale->setType(QCPAxis::atRight);
    m_color->setColorScale(m_colorScale);
    m_color->setGradient(QCPColorGradient::gpSpectrum);
    m_marginGroup = new QCPMarginGroup(m_view);
    m_view->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginGroup);
    m_colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginGroup);
    m_view->rescaleAxes();

    if (PLOT_TYPE::PLOT_3D_SUB_TIME == type || PLOT_TYPE::PLOT_3D_HIS_TIME == type) {
        double time = QDateTime::currentSecsSinceEpoch();
        m_xStart = time;
        m_xEnd = time + DEFALUT_AXIS_STEP;
        m_view->xAxis->setRange(time, time+DEFALUT_AXIS_STEP);
        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);    //日期做X轴
        m_view->xAxis->setTicker(dateTicker);   //设置X轴为时间轴
        m_recordCount = DEFAULT_3D_RECORD_COUNT;
    } else if (PLOT_TYPE::PLOT_3D_SUB_POS == type || PLOT_TYPE::PLOT_3D_HIS_POS == type) {
        double time = QDateTime::currentSecsSinceEpoch();
        m_yStart = time;
        m_yEnd = time + DEFALUT_AXIS_STEP;
        m_view->yAxis->setRange(time, time+DEFALUT_AXIS_STEP);
        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);    //日期做y轴
        m_view->yAxis->setTicker(dateTicker);   //设置y轴为时间轴
        m_recordCount = DEFAULT_3D_RECORD_COUNT;
    }

    m_colorMap = new ColorMap(scene, parent, m_plotType);
    m_colorMap->SetRelativeItem(this);
}

void MyGraphicsItem::Destory()
{
    m_receiver->Stop();
    m_scene->RemoveItem(this);
}

QRectF MyGraphicsItem::boundingRect() const
{
    return QRect(0, m_yPos, GRAPHICS_ITEM_WIDTH, GRAPHICS_ITEM_HEIGHT);
}

void MyGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);

    if (m_viewCreated) {
        return;
    }

    m_view->setParent(widget);
    m_viewCreated = true;
    m_view->show();
}

void MyGraphicsItem::ShowMain()
{
    m_scene->SwitchItem(this);
    m_hidePlot = false;
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_SUB:
    {
        m_plot->show();
        break;
    }
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
    {
        m_colorMap->show();
        break;
    }
    default:
        break;
    }
}

void MyGraphicsItem::HideMain()
{
    m_hidePlot = true;
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_SUB:
    {
        m_plot->hide();
        break;
    }
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
    {
        m_colorMap->hide();
        break;
    }
    default:
        return;
    }
}

void MyGraphicsItem::SetXYLable(const QString &xName, const QString &yName)
{
    if (m_xName.compare(QString("")) == 0) {
        m_xName = xName;
        m_yName = yName;

        switch (m_plotType) {
        case PLOT_TYPE::PLOT_2D_MAIN:
        case PLOT_TYPE::PLOT_2D_SUB:
        case PLOT_TYPE::PLOT_2D_HIS_MAIN:
        case PLOT_TYPE::PLOT_2D_HIS_SUB:
            m_plot->SetXYLable(xName, yName);
            break;
        case PLOT_TYPE::PLOT_3D_MAIN:
        case PLOT_TYPE::PLOT_3D_SUB_POS:
        case PLOT_TYPE::PLOT_3D_SUB_TIME:
        case PLOT_TYPE::PLOT_3D_HIS_MAIN:
        case PLOT_TYPE::PLOT_3D_HIS_POS:
        case PLOT_TYPE::PLOT_3D_HIS_TIME:
            m_colorMap->SetXYLable(xName, yName);
            break;
        default:
            break;
        }
    }
}

void MyGraphicsItem::SetXYRange(qreal xStart, qreal xEnd, qreal yStart, qreal yEnd)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_SUB:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
        m_xStart = xStart;
        m_yStart = yStart;
        m_xEnd = xEnd;
        m_yEnd = yEnd;
        m_view->xAxis->setRange(m_xStart, m_xEnd);
        m_view->yAxis->setRange(m_yStart, m_yEnd);
        m_plot->SetRange(xStart, xEnd, yStart, yEnd);
        break;
    case PLOT_TYPE::PLOT_2D_SUB:
        m_yStart = yStart;
        m_yEnd = yEnd;
        m_view->yAxis->setRange(m_yStart, m_yEnd);
        m_plot->SetRange(xStart, xEnd, yStart, yEnd);
        break;
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
        m_xStart = xStart;
        m_yStart = yStart;
        m_xEnd = xEnd;
        m_yEnd = yEnd;
        m_view->xAxis->setRange(m_xStart, m_xEnd);
        m_view->yAxis->setRange(m_yStart, m_yEnd);
        m_colorMap->SetRange(xStart, xEnd, yStart, yEnd);
        break;
    case PLOT_TYPE::PLOT_3D_SUB_POS:
        m_xStart = xStart;
        m_xEnd = xEnd;
        m_view->xAxis->setRange(m_xStart, m_xEnd);
        m_colorMap->SetRange(xStart, xEnd, yStart, yEnd);
        break;
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
        m_yStart = yStart;
        m_yEnd = yEnd;
        m_view->yAxis->setRange(m_yStart, m_yEnd);
        m_colorMap->SetRange(xStart, xEnd, yStart, yEnd);
        break;
    default:
        break;
    }
}

void MyGraphicsItem::SetXRange(qreal xStart, qreal xEnd)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    {
        m_xStart = xStart;
        m_xEnd = xEnd;
        m_view->xAxis->setRange(m_xStart, m_xEnd);
        break;
    }
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_2D_HIS_SUB:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
        break;
    default:
        break;
    }
    m_view->replot();
}

void MyGraphicsItem::SetYRange(qreal yStart, qreal yEnd)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    {
        m_yStart = yStart;
        m_yEnd = yEnd;
        m_view->yAxis->setRange(m_yStart, m_yEnd);
        break;
    }
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    default:
        break;
    }
    m_view->replot();
}

void MyGraphicsItem::SetY2Range(qreal yStart, qreal yEnd)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
    {
        m_y2Start = yStart;
        m_y2End = yEnd;
        m_view->yAxis2->setRange(m_y2Start, m_y2End);
        break;
    }
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    default:
        break;
    }
    m_view->replot();
}

void MyGraphicsItem::SetXLabel(const QString &xName)
{
    if (PLOT_TYPE::PLOT_3D_SUB_POS == m_plotType ||
        PLOT_TYPE::PLOT_3D_HIS_POS == m_plotType) {
//        m_xName = xName;
//        m_view->xAxis->setLabel(xName);

        m_colorMap->SetXLabel(xName);
    }
}

void MyGraphicsItem::SetYLabel(const QString &yName)
{
    if (PLOT_TYPE::PLOT_2D_SUB == m_plotType ||
        PLOT_TYPE::PLOT_2D_HIS_SUB == m_plotType) {
//        m_yName = yName;
//        m_view->yAxis->setLabel(yName);

        m_plot->SetYLabel(yName);
    }

    if (PLOT_TYPE::PLOT_3D_SUB_TIME == m_plotType ||
        PLOT_TYPE::PLOT_3D_HIS_TIME == m_plotType) {
//        m_yName = yName;
//        m_view->yAxis->setLabel(yName);

        m_colorMap->SetYLabel(yName);
    }
}

void MyGraphicsItem::UpdateData(QVector<double> &xData, QVector<double> &yData, qint64 time)
{
    //std::cout << "update date count:" << m_xDatas.count() << std::endl;
    QVector<double> tmp_yData = yData;
    m_multilineLock.lock();
    if (m_baseLineIndex >= 0) { // 已经设置基线
        for (int j = 0; j < xData.count(); ++j) {
            tmp_yData[j] = yData[j] - m_yBase[j];
        }
    }

    if (time > 0) {
        m_times.push_back(time);
    }

    if (!m_multilineMode) {
        if (m_multilineGraphs.count() > 0) {
            if (!m_xDatas.empty()) {
                m_xDatas.pop_front();
                m_yDatas.pop_front();
            }
            m_multilineGraphs[0]->setData(xData, tmp_yData, true);
            m_xDatas.push_back(xData);
            m_yDatas.push_back(tmp_yData);
        }
        m_multilineLock.unlock();
        goto update;
    }

    if (m_plotType == PLOT_TYPE::PLOT_2D_HIS_MAIN && m_curMulLineCount == m_multilineCount) {
        m_multilineLock.unlock();
        return;
    }

    if (m_curMulLineCount == m_multilineCount) {
        m_xDatas.pop_front();
        m_yDatas.pop_front();
        for (int i = 0; i < m_multilineCount - 1; ++i) {
            m_multilineGraphs[i]->setData(m_xDatas[i], m_yDatas[i]);
        }
        m_multilineGraphs[m_multilineCount - 1]->setData(xData, tmp_yData);
        m_xDatas.push_back(xData);
        m_yDatas.push_back(tmp_yData);
    } else {
        m_multilineGraphs[m_curMulLineCount++]->setData(xData, tmp_yData);
        m_xDatas.push_back(xData);
        m_yDatas.push_back(tmp_yData);
    }
    m_multilineLock.unlock();

update:
    m_plot->UpdateData(xData, yData, time);
}

void MyGraphicsItem::UpdateData(double xData, double yData)
{
    if (m_recordX.count() >= m_recordCount) {
        m_recordX.pop_front();
    }
    m_recordX.append(xData);

    if (m_recordY.count() >= m_recordCount) {
        m_recordY.pop_front();
    }
    m_recordY.append(yData);
    if (m_multilineGraphs.count() > 0) {
        m_multilineGraphs[0]->setData(m_recordX, m_recordY, true);
    }

    if (m_xEnd - 50 < xData) {
        m_multilineGraphs[0]->rescaleAxes(true);
        m_xEnd = xData + DEFALUT_AXIS_STEP;
        m_view->xAxis->setRange(m_xStart, m_xEnd);
    }

    m_plot->UpdateData(xData, yData);
}

void MyGraphicsItem::UpdateData(QCPColorMapData *mapData)
{
    if (m_plotType != PLOT_TYPE::PLOT_3D_MAIN && m_plotType != PLOT_TYPE::PLOT_3D_HIS_MAIN &&
        m_plotType != PLOT_TYPE::PLOT_3D_HIS_POS && m_plotType != PLOT_TYPE::PLOT_3D_HIS_TIME) {
        return;
    }

    m_color->setData(mapData);

    QCPColorMapData *data = new QCPColorMapData(*mapData);
    if (data) {
        m_colorMap->UpdateData(data);
    }
}

void MyGraphicsItem::UpdateData(QVector<double> &zData)
{
    if (m_recordCount <= m_recordZ.count()) {
        QVector<double> *tmp = m_recordZ.first();
        m_recordZ.pop_front();
        delete tmp;

        tmp = new QVector<double>(zData);
        m_recordZ.push_back(tmp);
        if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS || m_plotType == PLOT_TYPE::PLOT_3D_HIS_POS) {
            m_valueSize++;
        } else if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME || m_plotType == PLOT_TYPE::PLOT_3D_HIS_TIME) {
            m_keySize++;
        }
    } else {
        QVector<double> *tmp = new QVector<double>(zData);
        m_recordZ.push_back(tmp);
        if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS || m_plotType == PLOT_TYPE::PLOT_3D_HIS_POS) {
            m_valueSize++;
        } else if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME || m_plotType == PLOT_TYPE::PLOT_3D_HIS_TIME) {
            m_keySize++;
        }
    }

    double time = QDateTime::currentSecsSinceEpoch();
    QCPColorMapData *data = nullptr;
    if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME || m_plotType == PLOT_TYPE::PLOT_3D_HIS_TIME) {
        if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME && time >= m_xEnd) {
            m_xEnd = time + DEFALUT_AXIS_STEP;
            m_view->xAxis->setRange(m_xStart, m_xEnd);
        }
        if (m_plotType == PLOT_TYPE::PLOT_3D_HIS_TIME) {
            time = m_xEnd;
        }
        data = new QCPColorMapData(m_keySize, m_valueSize,
                QCPRange(m_xStart, time), QCPRange(m_yStart, m_yEnd));
        if (!data) {
            return;
        }
        for (int i = 0; i < m_keySize; ++i) {
            QVector<double> *tmp = m_recordZ[i];
            for (int j = 0; j < m_valueSize; ++j) {
                data->setCell(i, j, (*tmp)[j]);
            }
        }
    } else if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS || m_plotType == PLOT_TYPE::PLOT_3D_HIS_POS) {
        if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS && time >= m_yEnd) {
            m_yEnd = time + DEFALUT_AXIS_STEP;
            m_view->yAxis->setRange(m_yStart, m_yEnd);
        }
        if (m_plotType == PLOT_TYPE::PLOT_3D_HIS_POS) {
            time = m_yEnd;
        }
        data = new QCPColorMapData(m_keySize, m_valueSize,
                QCPRange(m_xStart, m_xEnd), QCPRange(m_yStart, time));
        if (!data) {
            return;
        }
        for (int i = 0; i < m_valueSize; ++i) {
            QVector<double> *tmp = m_recordZ[i];
            for (int j = 0; j < m_keySize; ++j) {
                data->setCell(j, i, (*tmp)[j]);
            }
        }
    } else {
        std::cout << "error plot type" << std::endl;
        return;
    }

    m_color->setData(data);

    m_colorMap->UpdateData(zData);
}

void MyGraphicsItem::UpdateData(const QVector<double> &xData, const QVector<double> &yData,
                                const QVector<double> &zData, QCPColorMapData *mapData, qint64 time)
{
    QCPColorMapData *data = nullptr;
    if (time == 0) { // 数据采集模式
        m_color->setData(mapData);

        data = new QCPColorMapData(*mapData);
        if (!data) {
            std::cout << "error: new failed!" << std::endl;
            return;
        }
    } else {        // 观察历史数据模式
        if (!m_hisHasData) {
            m_color->setData(mapData);

            m_hisHasData = true;
            data = new QCPColorMapData(*mapData);
            if (!data) {
                std::cout << "error: new failed!" << std::endl;
                return;
            }
        } else {
            data = mapData;
        }
    }

    m_colorMap->UpdateData(xData, yData, zData, data, time);
}

void MyGraphicsItem::UpdateWarnings(const QVector<double> &warnings)
{
    if (PLOT_TYPE::PLOT_2D_MAIN == m_plotType || PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType) {
        m_plot->UpdateWarnings(warnings);
    } else if (PLOT_TYPE::PLOT_3D_MAIN == m_plotType || PLOT_TYPE::PLOT_3D_HIS_MAIN == m_plotType) {
        m_colorMap->UpdateWarnings(warnings);
    }
}

void MyGraphicsItem::SetLonLats(const QVector<double> &lons, const QVector<double> &lats)
{
    if (PLOT_TYPE::PLOT_2D_MAIN == m_plotType || PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType) {
        m_plot->SetLonLats(lons, lats);
    } else if (PLOT_TYPE::PLOT_3D_MAIN == m_plotType || PLOT_TYPE::PLOT_3D_HIS_MAIN == m_plotType) {
        m_colorMap->SetLonLats(lons, lats);
    }
}

void MyGraphicsItem::resize(qreal wigth, qreal height)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_MAIN:
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_2D_HIS_MAIN:
    case PLOT_TYPE::PLOT_2D_HIS_SUB:
    {
        m_plot->resize(wigth, height);
        break;
    }
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
    {
        m_colorMap->resize(wigth, height);
        break;
    }
    default:
        break;
    }
}

void MyGraphicsItem::SetTitle(const QString &name)
{
    QCPTextElement *element = static_cast<QCPTextElement *>(m_view->plotLayout()->element(0, 0));
    element->setText(name);
}

void MyGraphicsItem::ResetY(int y_pos)
{
    m_yPos = y_pos;
}

void MyGraphicsItem::ReDraw(int x, int y)
{
    m_view->move(x, y); // 将子窗体放到相对父窗体的坐标位置
}

void MyGraphicsItem::SetFocus()
{
    m_view->setBackground(QBrush(Qt::lightGray));
    m_view->replot();
}

void MyGraphicsItem::RemoveFocus()
{
    m_view->setBackground(QBrush(Qt::white));
    m_view->replot();
}

//void MyGraphicsItem::DataReady(DBusData *data)
//{
//    //m_worker->Notify(data);
//}

void MyGraphicsItem::DataIn()
{
    m_view->replot();
    if (!m_hidePlot) {
        switch (m_plotType) {
        case PLOT_TYPE::PLOT_2D_MAIN:
        case PLOT_TYPE::PLOT_2D_SUB:
        case PLOT_TYPE::PLOT_2D_HIS_MAIN:
        case PLOT_TYPE::PLOT_2D_HIS_SUB:
            m_plot->replot();
            break;
        case PLOT_TYPE::PLOT_3D_MAIN:
        case PLOT_TYPE::PLOT_3D_SUB_POS:
        case PLOT_TYPE::PLOT_3D_SUB_TIME:
        case PLOT_TYPE::PLOT_3D_HIS_MAIN:
        case PLOT_TYPE::PLOT_3D_HIS_POS:
        case PLOT_TYPE::PLOT_3D_HIS_TIME:
            m_colorMap->replot();
            break;
        default:
            break;
        }
    }
}

bool MyGraphicsItem::SetSubPlotInfo(const QPointF &point, int recordCount)
{
    bool ret = true;
    SetSubPlotPoint(point);
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_2D_SUB:
    case PLOT_TYPE::PLOT_2D_HIS_SUB:
    {
        if (recordCount <= MAX_2D_RECORD_COUNT) {
            m_recordCount = recordCount;
        } else {
            ret = false;
            goto out;
        }
        m_startPoint = point;
        ret = m_plot->SetSubPlotInfo(point, recordCount);
        break;
    }
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
    {
        if (recordCount <= MAX_3D_RECORD_COUNT) {
            m_recordCount = recordCount;
        } else {
            ret = false;
            goto out;
        }
        m_startPoint = point;
        ret = m_colorMap->SetSubPlotInfo(point, recordCount);
        break;
    }
    default:
        ret = false;
        break;
    }

out:
    return ret;
}

void MyGraphicsItem::InterMultilineMode(int multilineCount)
{
    m_multilineLock.lock();
    m_multilineCount = multilineCount;
    m_multilineMode = true;
    for (int i = 0; i < m_multilineCount - 1; ++i) {
        QCPGraph *graph = m_view->addGraph();
        m_multilineGraphs.append(graph);
        graph->setPen(GetColor(m_multilineGraphs.count()));
    }
    m_multilineLock.unlock();
}

void MyGraphicsItem::ExitMultilineMode()
{
    m_multilineLock.lock();
    m_multilineMode = false;
    m_multilineCount = 1;
    m_curMulLineCount = 1;

    while (m_multilineGraphs.count() > 1) {
        QCPGraph *graph = m_multilineGraphs.last();
        m_view->removeGraph(graph);
        m_multilineGraphs.pop_back();
    }

    m_xDatas.clear();
    m_yDatas.clear();
    m_multilineLock.unlock();
    m_view->replot();
}

void MyGraphicsItem::SetSubPlotPoint(const QPointF &pos)
{
    m_startPoint = pos;
}

QPointF MyGraphicsItem::GetSubPlotPoint() const
{
    return m_startPoint;
}

void MyGraphicsItem::SetBaseLine(int index)
{
    m_multilineLock.lock();
    m_baseLineIndex = index - 1;
    if (!m_baseGraph) {
        //m_view->yAxis2->grid()->setSubGridVisible(true);
        m_view->yAxis2->setRange(DEFAULT_2D_Y2AXIS_START, DEFAULT_2D_Y2AXIS_END);   // todo
        m_view->yAxis->setRange(-m_yStart, m_yEnd);
        m_yStart = -m_yStart;
        m_y2Start = DEFAULT_2D_Y2AXIS_START;
        m_y2End = DEFAULT_2D_Y2AXIS_END;
        m_baseGraph = m_view->addGraph(m_view->xAxis, m_view->yAxis);
        m_baseGraph->setPen(GetColor(1));
    }

    m_xBase = m_xDatas[m_baseLineIndex];
    m_yBase = m_yDatas[m_baseLineIndex];
    m_baseGraph->setData(m_xBase, m_yBase);

    int count = m_xDatas.count();
    for (int i = 0; i < count; ++i) {
        int count2 = m_xDatas[i].count();
        for (int j = 0; j < count2; ++j) {
            m_yDatas[i][j] = m_yDatas[i][j] - m_yBase[j];
        }

        m_multilineGraphs[i]->setValueAxis(m_view->yAxis2);
        m_multilineGraphs[i]->setData(m_xDatas[i], m_yDatas[i]);
    }

    m_multilineLock.unlock();
    m_view->replot();
}

void MyGraphicsItem::RemoveBaseLine()
{
    if (m_baseLineIndex < 0) {
        return;
    }

    m_multilineLock.lock();
    if (m_baseGraph) {
        m_view->removeGraph(m_baseGraph);
        m_baseGraph = nullptr;
        m_view->yAxis2->setTicks(false);
        m_view->yAxis2->setVisible(true);
        m_view->yAxis2->setSubTicks(false);
        m_view->yAxis2->grid()->setSubGridVisible(false);
        m_view->yAxis2->setTickLabels(false);
    }

    m_view->yAxis->setRange(m_yStart, m_yEnd);
    int count = m_xDatas.count();
    for (int i = 0; i < count; ++i) {
        int count2 = m_xDatas[i].count();
        for (int j = 0; j < count2; ++j) {
            m_yDatas[i][j] = m_yDatas[i][j] + m_yBase[j];
        }

        m_multilineGraphs[i]->setValueAxis(m_view->yAxis);
        m_multilineGraphs[i]->setData(m_xDatas[i], m_yDatas[i]);
    }

    m_baseLineIndex = -1;
    m_multilineLock.unlock();
    m_view->replot();
}

bool MyGraphicsItem::SetMultiLineCount(int count)
{
    if (PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType) {
        std::cout << ".........:" << count << std::endl;
        m_multilineLock.lock();
        for (int i = 0; i < count - 1; ++i) {
            QCPGraph *graph = m_view->addGraph();
            if (!graph) {
                m_multilineLock.unlock();
                return false;
            }
            m_multilineGraphs.append(graph);
            graph->setPen(GetColor(m_multilineGraphs.count()));
        }

        m_multilineCount = count;
        m_curMulLineCount = 0;
        m_multilineLock.unlock();

        return m_plot->SetMultiLineCount(count);
    } else {
        return false;
    }
}

void MyGraphicsItem::replot()
{
    if (PLOT_TYPE::PLOT_2D_HIS_SUB == m_plotType) {
        m_view->replot();
        m_plot->replot();
    }

    if (PLOT_TYPE::PLOT_3D_HIS_POS == m_plotType ||
        PLOT_TYPE::PLOT_3D_HIS_TIME == m_plotType) {
        m_view->replot();
        m_colorMap->replot();
    }
}

void MyGraphicsItem::SetBaiduMapCenter(const QString &centerName)
{
    if (PLOT_TYPE::PLOT_2D_MAIN == m_plotType || PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType) {
        m_plot->SetBaiduMapCenter(centerName);
    } else if (PLOT_TYPE::PLOT_3D_MAIN == m_plotType || PLOT_TYPE::PLOT_3D_HIS_MAIN == m_plotType) {
        m_colorMap->SetBaiduMapCenter(centerName);
    }
}

void MyGraphicsItem::SetAppPath(const QString &appPath)
{
    if (PLOT_TYPE::PLOT_2D_MAIN == m_plotType || PLOT_TYPE::PLOT_2D_HIS_MAIN == m_plotType) {
        m_plot->SetAppPath(appPath);
    } else if (PLOT_TYPE::PLOT_3D_MAIN == m_plotType || PLOT_TYPE::PLOT_3D_HIS_MAIN == m_plotType) {
        m_colorMap->SetAppPath(appPath);
    }
}
