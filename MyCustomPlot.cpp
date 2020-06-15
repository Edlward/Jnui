#pragma execution_character_set("utf-8")
#include "MyCustomPlot.h"
#include <iostream>
#include "MySelectMemu.h"
#include <QColor>
#include "MyGraphicsItem.h"
#include <QTextBrowser>
#include "MyGraphicsScene.h"
#include "BaiduMap.h"

MyCustomPlot::MyCustomPlot(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type)
    : QCustomPlot(parent), m_scene(scene), m_parentWidget(parent), m_plotType(type), m_xStart(0),
      m_xEnd(0), m_yStart(0), m_yEnd(0), m_preX(0), m_multilineCount(1),
      m_curMulLineCount(0), m_curMulWarnCount(0), m_multilineMode(false), m_baseGraph(nullptr),
      m_baseLineIndex(-1), m_highLightLineIndex(-1), m_label(nullptr), m_recordCount(0),
      m_item(nullptr), m_baiduMap(nullptr), m_isBaiduMap(false), m_rubberBand(nullptr),
      m_pauseDataUpdate(false), m_dragMode(false), m_choosePointMode(false),
      m_highLightPoint(nullptr), m_choosePointsGraph(nullptr), m_isShiftPressed(false),
      m_showMenu(false), m_isWheeled(false)
{
    qRegisterMetaType<QCPRange>("QCPRange");
    if (PLOT_TYPE::PLOT_2D_SUB == type || PLOT_TYPE::PLOT_2D_HIS_SUB == type) {
        double time = QDateTime::currentSecsSinceEpoch();
        xAxis->setRange(time, time + DEFALUT_AXIS_STEP);
        m_xStart = time;
        m_xEnd = time + DEFALUT_AXIS_STEP;
        yAxis->setRange(0, 500); // 初始值无所谓，接口会设置

        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);    //日期做X轴
        dateTicker->setDateTimeFormat("yyyy-MM-dd hh:mm:ss");
        xAxis->setTicker(dateTicker);                           //设置X轴为时间轴
        m_xName = "Time(s)";
        xAxis->setLabel(m_xName);
    }

    if (PLOT_TYPE::PLOT_2D_HIS_MAIN == type) {
        m_multilineMode = true;
    }

    QCPGraph *graph = addGraph();
    m_multilineGraphs.append(graph);
    m_curMulLineCount++;

    QCPGraph *warnGraph = addGraph();
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(6);
    warnGraph->setPen(pen);
    warnGraph->setLineStyle(QCPGraph::lsNone);
    warnGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssStar, 4));
    m_mulWarnGraphs.push_back(warnGraph);
    m_curMulWarnCount++;

    graph->setPen(QPen(Qt::blue));
    axisRect()->setupFullAxesBox(false);
    setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    resize(parent->width(), parent->height());
    graph->rescaleAxes(true);
    rescaleAxes();

    xAxis->setTicks(true);
    xAxis->setSubTicks(true);
    yAxis->setTicks(true);
    yAxis->setSubTicks(true);
    yAxis->setSubTickPen(QPen(Qt::red));
    yAxis->grid()->setSubGridVisible(true);
    xAxis->grid()->setSubGridVisible(true);

    m_xLines[0] = nullptr;
    m_xLines[1] = nullptr;
    m_yLines[0] = nullptr;
    m_yLines[1] = nullptr;

    move(GRAPHICS_SCENE_WIDTH, 0);
    show();
}

MyCustomPlot::~MyCustomPlot()
{
    while(!m_points.empty()) {
        auto vec = m_points.first();
        while (!vec->empty()) {
            auto p = vec->first();
            delete p;
            vec->pop_front();
        }

        delete vec;
        m_points.pop_front();
    }
}

void MyCustomPlot::SetRange(qreal xStart, qreal xEnd, qreal yStart, qreal yEnd)
{
    if (PLOT_TYPE::PLOT_2D_SUB == m_plotType) {
        m_yStart = yStart;
        m_yEnd = yEnd;
        yAxis->setRange(yStart, yEnd);
    } else {
        m_xStart = xStart;
        m_xEnd = xEnd;
        m_yStart = yStart;
        m_yEnd = yEnd;
        xAxis->setRange(xStart, xEnd);
        yAxis->setRange(yStart, yEnd);
    }
}

void MyCustomPlot::UpdateData(QVector<double> &xData, QVector<double> &yData, qint64 time)
{
    //std::cout << "cur lines:" << m_xDatas.count() << std::endl;
    m_multilineLock.lock();
    if (m_pauseDataUpdate) {
        m_multilineLock.unlock();
        return;
    }

    if (m_baseLineIndex >= 0) { // 已经设置基线
        for (int j = 0; j < xData.count(); ++j) {
            yData[j] = yData[j] - m_yBase[j];
        }
    }

    if (time > 0) {
        m_times.push_back(time);
    }

    if (!m_multilineMode) {
        m_multilineGraphs[0]->setData(xData, yData, true);
        if (!m_xDatas.empty()) {
            m_xDatas.pop_front();
            m_yDatas.pop_front();
        }
        m_xDatas.push_back(xData);
        m_yDatas.push_back(yData);
        m_multilineLock.unlock();
        return;
    }

    if (m_curMulLineCount == m_multilineCount) {
        m_xDatas.pop_front();
        m_yDatas.pop_front();
        for (int i = 0; i < m_multilineCount - 1; ++i) {
            m_multilineGraphs[i]->setData(m_xDatas[i], m_yDatas[i]);
        }
        m_multilineGraphs[m_multilineCount - 1]->setData(xData, yData);
    } else {
        m_multilineGraphs[m_curMulLineCount++]->setData(xData, yData);
    }
    m_xDatas.push_back(xData);
    m_yDatas.push_back(yData);
    m_multilineLock.unlock();
}

void MyCustomPlot::UpdateData(double xData, double yData)
{
    if (m_pauseDataUpdate) {
        return;
    }

    if (m_recordX.count() >= m_recordCount) {
        m_recordX.pop_front();
        m_xStart = m_recordX.first();
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
        xAxis->setRange(m_xStart, m_xEnd);
    }
}

void MyCustomPlot::SetLonLats(const QVector<double> &lons, const QVector<double> &lats)
{
    if (m_lons.count() == 0) {
        m_lons = lons;
        m_lats = lats;
    }

    if (m_isBaiduMap && m_baiduMap) {
        m_baiduMap->UpdateData(m_lons, m_lats);
    }
}

void MyCustomPlot::UpdateWarnings(const QVector<double> &warnings)
{
    if (m_pauseDataUpdate) {
        return;
    }

    int count = warnings.count();
    QVector<double> xvec, yvec;
    if (!m_warnings.empty()) {
        m_warnings.clear();
    }

    for (int i = 0; i < count; ++i) {
        if (warnings[i] > 0) {
            xvec.push_back(m_xDatas[0][i]);
            yvec.push_back(m_yDatas[0][i]);
        }

        m_warnings.append(warnings[i]);
    }

    if (!m_isBaiduMap) {
        m_multilineLock.lock();
        if (!m_multilineMode && m_baseLineIndex < 0) {
            if (m_xWarns.count()) {
                m_xWarns.clear();
                m_yWarns.clear();
            }

            m_xWarns = xvec;
            m_yWarns = yvec;

            m_mulWarnGraphs[0]->setData(xvec, yvec, true);
            m_multilineLock.unlock();
            return;
        }
        m_multilineLock.unlock();
    } else {
        if (m_baiduMap) {
            m_baiduMap->UpdateWarnings(warnings);
        }
    }
}

void MyCustomPlot::InterMultilineMode()
{
    if (m_multilineMode) {
        return;
    }

    bool ok;
    int input = QInputDialog::getInt(this, tr("lines"),
                                 tr("请输入同时显示线的个数:"), 2, 2, MAX_MULTILINE_COUNT, 1, &ok);
    if (ok) {
        m_item->InterMultilineMode(input);

        for (int i = 0; i < input - 1; ++i) {
            QCPGraph *graph = addGraph();
            m_multilineGraphs.append(graph);
            graph->setPen(GetColor(m_multilineGraphs.count()));
        }
        m_multilineLock.lock();
        m_multilineCount = input;
        m_multilineMode = true;
        QVector<double> xDatas, yDatas;
        m_mulWarnGraphs[0]->setData(xDatas, yDatas);
        m_multilineLock.unlock();
        replot();
    }
}

void MyCustomPlot::ExitMultilineMode()
{
    RemoveBaseLine();

    m_multilineLock.lock();
    m_multilineMode = false;
    m_multilineCount = 1;
    m_curMulLineCount = 1;
    m_curMulWarnCount = 1;

    while (m_multilineGraphs.count() > 1) {
        QCPGraph *graph = m_multilineGraphs.last();
        removeGraph(graph);
        m_multilineGraphs.pop_back();
    }

    m_xDatas.clear();
    m_yDatas.clear();
    //m_xWarns.clear();
    //m_yWarns.clear();

    ExitChoosePointMode();
    m_multilineLock.unlock();

    m_item->ExitMultilineMode();
    replot();
}

void MyCustomPlot::ZoomReset()
{
    m_isWheeled = false;
    xAxis->setRange(m_xStart, m_xEnd);
    yAxis->setRange(m_yStart, m_yEnd);
    replot();
}

void MyCustomPlot::SetXYLable(const QString &xName, const QString &yName)
{
    m_xName = xName;
    m_yName = yName;
    xAxis->setLabel(xName);
    yAxis->setLabel(yName);
}

void MyCustomPlot::SetYLabel(const QString &yName)
{
    if (PLOT_TYPE::PLOT_2D_SUB == m_plotType ||
        PLOT_TYPE::PLOT_2D_HIS_SUB == m_plotType) {
        m_yName = yName;
        yAxis->setLabel(yName);
    }
}

void MyCustomPlot::NewSubView(const QPointF &pos)
{
    double x = xAxis->pixelToCoord(pos.x());
    double y = yAxis->pixelToCoord(pos.y());
    QPointF real_pos(x, y);
    if (m_plotType == PLOT_TYPE::PLOT_2D_MAIN) {
        MyGraphicsItem *sub = m_scene->NewSubView(real_pos, PLOT_TYPE::PLOT_2D_SUB);
        if (sub) {
            sub->SetXYRange(0, 0,
                        DEFAULT_2D_SUB_YAXIS_RANGE_START,
                        DEFAULT_2D_SUB_YAXIS_RANGE_END);
            sub->SetYLabel(m_yName);
        }
    } else if (m_plotType == PLOT_TYPE::PLOT_2D_HIS_MAIN) {
        MyGraphicsItem *sub = m_scene->NewSubView(real_pos, PLOT_TYPE::PLOT_2D_HIS_SUB);
        if (!sub) {
            return;
        }
        // 更新子图数据
        QVector<double> xDatas, yDatas;
        BuildHisSubPlotDatas(real_pos, xDatas, yDatas);
        if (xDatas.empty()) {
            std::cout<< "error:new sub view failed" << std::endl;
            return;
        }
        sub->SetYLabel(m_yName);
        sub->SetXYRange(xDatas.first(), xDatas.last(),
                        DEFAULT_2D_SUB_YAXIS_RANGE_START,
                        DEFAULT_2D_SUB_YAXIS_RANGE_END);
        sub->UpdateData(xDatas, yDatas);
        sub->replot();
    }
}

bool MyCustomPlot::SetSubPlotInfo(const QPointF &point, int recordCount)
{
    if (recordCount > MAX_2D_RECORD_COUNT) {
        m_startPoint = point;
        m_recordCount = MAX_2D_RECORD_COUNT;
        return false;
    }
    m_startPoint = point;
    m_recordCount = recordCount;

    return true;
}

void MyCustomPlot::CreateCrosswiseLine(const QPoint &pos)
{
    if (m_xLines[0] != nullptr) {
        return;
    }
    if (m_yLines[0] == nullptr || m_yLines[1] == nullptr) {
        double x = xAxis->pixelToCoord(pos.x());
        QPen pen(Qt::black);
        QCPGraph *graph = addGraph();
        graph->setPen(pen);
        QVector<double> xVec;
        xVec.append(x);
        xVec.append(x);
        QVector<double> yVec;
        yVec.append(m_yStart);
        yVec.append(m_yEnd);
        graph->setData(xVec, yVec);
        if (m_yLines[0] == nullptr) {
            m_yLines[0] = new QPair<double, QCPGraph*>(x, graph);
            m_point1.setX(pos.x());
            m_point1.setY(pos.y());
        } else {
            m_yLines[1] = new QPair<double, QCPGraph*>(x, graph);
            m_point2.setX(pos.x());
            m_point2.setY(pos.y());
            CreateTextDisplayer(true);
        }

        if (!m_hide) {
            replot();
        }
    }
}

void MyCustomPlot::CreateLengthwiseLine(const QPoint &pos)
{
    if (m_yLines[0] != nullptr) {
        return;
    }
    if (m_xLines[0] == nullptr || m_xLines[1] == nullptr) {
        double y = yAxis->pixelToCoord(pos.y());
        QPen pen(Qt::black);
        QCPGraph *graph = addGraph();
        graph->setPen(pen);
        QVector<double> xVec;
        xVec.append(m_xStart);
        xVec.append(m_xEnd);
        QVector<double> yVec;
        yVec.append(y);
        yVec.append(y);
        graph->setData(xVec, yVec);
        if (m_xLines[0] == nullptr) {
            m_xLines[0] = new QPair<double, QCPGraph*>(y, graph);
            m_point1.setX(pos.x());
            m_point1.setY(pos.y());
        } else {
            m_xLines[1] = new QPair<double, QCPGraph*>(y, graph);
            m_point2.setX(pos.x());
            m_point2.setY(pos.y());
            CreateTextDisplayer(false);
        }

        if (!m_hide) {
            replot();
        }
    }
}

void MyCustomPlot::ClearLines()
{
    if (m_xLines[0] != nullptr) {
        QCPGraph *graph = m_xLines[0]->second;
        removeGraph(graph);
        delete m_xLines[0];
        m_xLines[0] = nullptr;
    }

    if (m_xLines[1] != nullptr) {
        QCPGraph *graph = m_xLines[1]->second;
        removeGraph(graph);
        delete m_xLines[1];
        m_xLines[1] = nullptr;
    }

    if (m_yLines[0] != nullptr) {
        QCPGraph *graph = m_yLines[0]->second;
        removeGraph(graph);
        delete m_yLines[0];
        m_yLines[0] = nullptr;
    }

    if (m_yLines[1] != nullptr) {
        QCPGraph *graph = m_yLines[1]->second;
        removeGraph(graph);
        delete m_yLines[1];
        m_yLines[1] = nullptr;
    }

    if (m_label) {
        delete m_label;
        m_label = nullptr;
    }

    if (!m_hide) {
        replot();
    }
}

void MyCustomPlot::mousePressEvent(QMouseEvent *event)
{
    if (m_dragMode) {
        m_dragPos = new QPoint(event->pos());
        if (!m_dragPos) {
            return;
        }
    }

    // 先判断选点
    if (m_choosePointMode && event->button() == Qt::LeftButton) {
        if (ChoosePoints(event->localPos())) {
            return;
        }
    } else if (m_multilineMode && event->button() == Qt::LeftButton) {    // 判断是否是选基线
        if (SetHighLightLine(event->localPos())) {
            return;
        }
    }

    if (event->button() == Qt::RightButton) {
        // 显示右键下拉框
        MySelectMemu menu(this, event->pos(), m_multilineCount);
    } else if (event->button() == Qt::LeftButton) {
        bool moveLine = false;
        if (m_xLines[0] != nullptr) {
            double y = yAxis->pixelToCoord(event->localPos().y());
            if (qAbs(m_xLines[0]->first - y) <= 2) {
                m_preX = m_xLines[0]->first;
                moveLine = true;
                //std::cout<< "pre:" << m_preX << std::endl;
            } else if (m_xLines[1] != nullptr && qAbs(m_xLines[1]->first - y) <= 2) {
                m_preX = m_xLines[1]->first;
                moveLine = true;
                //std::cout<< "pre:" << m_preX << std::endl;
            }
        } else if (m_yLines[0] != nullptr) {
            double x = xAxis->pixelToCoord(event->localPos().x());
            if (qAbs(m_yLines[0]->first - x) <= 2) {
                m_preX = m_yLines[0]->first;
                moveLine = true;
                //std::cout<< "pre:" << m_preX << std::endl;
            } else if (m_yLines[1] != nullptr && qAbs(m_yLines[1]->first - x) <= 2) {
                m_preX = m_yLines[1]->first;
                moveLine = true;
                //std::cout<< "pre:" << m_preX << std::endl;
            }
        }

        if (!moveLine && !m_dragMode) {
            m_origin = event->pos();
            m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
            if (!m_rubberBand) {
                return;
            }

            m_rubberBand->setGeometry(QRect(m_origin, QSize()));
            m_rubberBand->show();
        }
    }
}

void MyCustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (m_showMenu) {
        return;
    }

    if (m_rubberBand) { // 矩形选择
        m_rubberBand->setGeometry(QRect(m_origin, event->pos()).normalized());
        return;
    }

    if (m_preX) {
        if (m_xLines[0] != nullptr) {
            double y = yAxis->pixelToCoord(event->localPos().y());
            if (m_xLines[0]->first == m_preX) {
                m_preX = m_xLines[0]->first;
                QVector<double> xVec;
                xVec.append(m_xStart);
                xVec.append(m_xEnd);
                QVector<double> yVec;
                yVec.append(y);
                yVec.append(y);
                m_xLines[0]->second->setData(xVec, yVec);
                m_point1.setX(event->localPos().x());
                m_point1.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            } else if (m_xLines[1] != nullptr && m_xLines[1]->first  == m_preX) {
                m_preX = m_xLines[1]->first;
                QVector<double> xVec;
                xVec.append(m_xStart);
                xVec.append(m_xEnd);
                QVector<double> yVec;
                yVec.append(y);
                yVec.append(y);
                m_xLines[1]->second->setData(xVec, yVec);
                m_point2.setX(event->localPos().x());
                m_point2.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            }
        } else if (m_yLines[0] != nullptr) {
            double x = xAxis->pixelToCoord(event->localPos().x());
            if (m_yLines[0]->first  == m_preX) {
                m_preX = m_yLines[0]->first;
                QVector<double> xVec;
                xVec.append(x);
                xVec.append(x);
                QVector<double> yVec;
                yVec.append(m_yStart);
                yVec.append(m_yEnd);
                m_yLines[0]->second->setData(xVec, yVec);
                m_point1.setX(event->localPos().x());
                m_point1.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            } else if (m_yLines[1] != nullptr && m_yLines[1]->first  == m_preX) {
                m_preX = m_yLines[1]->first;
                QVector<double> xVec;
                xVec.append(x);
                xVec.append(x);
                QVector<double> yVec;
                yVec.append(m_yStart);
                yVec.append(m_yEnd);
                //m_yLines[1]->first = x;
                m_yLines[1]->second->setData(xVec, yVec);
                m_point2.setX(event->localPos().x());
                m_point2.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            }
        }
    } else {
        ShowPressedChoosePoint(event->localPos());
    }
}

void MyCustomPlot::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragMode && m_dragPos) {
        int x = xAxis->pixelToCoord(m_dragPos->x());
        int y = yAxis->pixelToCoord(m_dragPos->y()) - yAxis->pixelToCoord(event->pos().y());

        if (m_isWheeled) { // 鼠标滚轮滚动过，此时拖动模式只临时改变范围，后续需要还原
            //std::cout << "wheel" << std::endl;
            x = x - xAxis->pixelToCoord(event->pos().x());
            int xStart = xAxis->range().lower;
            int xEnd = xAxis->range().upper;
            int yStart = yAxis->range().lower;
            int yEnd = yAxis->range().upper;
            xStart += x;
            xEnd += x;
            xAxis->setRange(xStart, xEnd);
            yStart += y;
            yEnd += y;
            yAxis->setRange(yStart, yEnd);
            if (m_baseLineIndex >= 0) {
                int yStart2 = yAxis2->range().lower;
                int yEnd2 = yAxis2->range().upper;
                yStart2 += y;
                yEnd2 += y;
                yAxis2->setRange(yStart2, yEnd2);
            }
            goto show;
        }

        if (m_baseLineIndex < 0 || x < (m_xEnd - m_xStart)/2) {
            m_yStart = m_yStart + y;
            m_yEnd = m_yEnd + y;
            yAxis->setRange(m_yStart, m_yEnd);
            m_item->SetYRange(m_yStart, m_yEnd);
        } else {
            m_y2Start = m_y2Start + y;
            m_y2End = m_y2End + y;
            yAxis2->setRange(m_y2Start, m_y2End);
            m_item->SetY2Range(m_y2Start, m_y2End);
        }

show:
        replot();

        delete m_dragPos;
        m_dragPos = nullptr;
        return;
    }

    if (m_rubberBand) {  // 鼠标框放大
        m_isWheeled = true;
        const QRect zoomRect = m_rubberBand->geometry();
        int xp1, yp1, xp2, yp2;
        zoomRect.getCoords(&xp1, &yp1, &xp2, &yp2);
        double x1 = xAxis->pixelToCoord(xp1);
        double x2 = xAxis->pixelToCoord(xp2);
        double y1 = yAxis->pixelToCoord(yp1);
        double y2 = yAxis->pixelToCoord(yp2);
        //std::cout << x1 << " " << x2 << " " << y1 << " " << y2 << std::endl;
        if (qAbs(x1 - x2) >=1 && qAbs(y1 - y2) >= 1) {
            xAxis->setRange(x1, x2);
            yAxis->setRange(y1, y2);
        }

        m_rubberBand->hide();
        delete m_rubberBand;
        m_rubberBand = nullptr;
        replot();

        return;
    }

    if (m_preX) {
        if (m_xLines[0] != nullptr) {
            double y = yAxis->pixelToCoord(event->localPos().y());
            if (m_xLines[0]->first == m_preX) {
                m_preX = m_xLines[0]->first;
                QVector<double> xVec;
                xVec.append(m_xStart);
                xVec.append(m_xEnd);
                QVector<double> yVec;
                yVec.append(y);
                yVec.append(y);
                m_xLines[0]->first = y;
                m_xLines[0]->second->setData(xVec, yVec);
                m_point1.setX(event->localPos().x());
                m_point1.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            } else if (m_xLines[1] != nullptr && m_xLines[1]->first  == m_preX) {
                m_preX = m_xLines[1]->first;
                QVector<double> xVec;
                xVec.append(m_xStart);
                xVec.append(m_xEnd);
                QVector<double> yVec;
                yVec.append(y);
                yVec.append(y);
                m_xLines[1]->first = y;
                m_xLines[1]->second->setData(xVec, yVec);
                m_point2.setX(event->localPos().x());
                m_point2.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            }
        } else if (m_yLines[0] != nullptr) {
            double x = xAxis->pixelToCoord(event->localPos().x());
            if (m_yLines[0]->first  == m_preX) {
                m_preX = m_yLines[0]->first;
                QVector<double> xVec;
                xVec.append(x);
                xVec.append(x);
                QVector<double> yVec;
                yVec.append(m_yStart);
                yVec.append(m_yEnd);
                m_yLines[0]->first = x;
                m_yLines[0]->second->setData(xVec, yVec);
                m_point1.setX(event->localPos().x());
                m_point1.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            } else if (m_yLines[1] != nullptr && m_yLines[1]->first  == m_preX) {
                m_preX = m_yLines[1]->first;
                QVector<double> xVec;
                xVec.append(x);
                xVec.append(x);
                QVector<double> yVec;
                yVec.append(m_yStart);
                yVec.append(m_yEnd);
                m_yLines[1]->first = x;
                m_yLines[1]->second->setData(xVec, yVec);
                m_point2.setX(event->localPos().x());
                m_point2.setY(event->localPos().y());
                AdjustTextDisplayerPos();
                replot();
            }
        }

        m_preX = 0;
    }
}

void MyCustomPlot::InterMapMode()
{
    if (PLOT_TYPE::PLOT_2D_MAIN != m_plotType && PLOT_TYPE::PLOT_2D_HIS_MAIN != m_plotType) {
        return;
    }

    if (!m_baiduMap) {
        m_baiduMap = new BaiduMap(m_parentWidget, this, m_baiduMapCenter, m_appPath);
        if (!m_baiduMap) {
            return;
        }
    }

    m_baiduMap->UpdateData(m_lons, m_lats);
    this->hide();   // hide this widget
    m_baiduMap->show();
    m_isBaiduMap = true;
}

void MyCustomPlot::SetAxisRangeByUser(const QString &axisName, int axisIndex)
{
    QDialog dialog(this);
    dialog.window()->setWindowTitle("范围设置");
    dialog.window()->setWindowFlags(dialog.window()->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    QFormLayout form(&dialog);
    form.addRow(new QLabel(axisName));
    // Value1
    QString value1 = QString("最小值: ");
    QSpinBox *spinbox1 = new QSpinBox(&dialog);
    spinbox1->setRange(0, 1000000);
    form.addRow(value1, spinbox1);
    // Value2
    QString value2 = QString("最大值: ");
    QSpinBox *spinbox2 = new QSpinBox(&dialog);
    spinbox2->setRange(0, 1000000);
    form.addRow(value2, spinbox2);
    // Add Cancel and OK button
    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);
    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    // Process when OK button is clicked
    if (dialog.exec() == QDialog::Accepted) {
        int start = spinbox1->value();
        int end = spinbox2->value();
        if (axisIndex == 1) {
            m_xStart = start;
            m_xEnd = end;
            xAxis->setRange(start, end);
            m_item->SetXRange(start, end);
        } else if (axisIndex == 2) {
            m_yStart = start;
            m_yEnd = end;
            yAxis->setRange(start, end);
            m_item->SetYRange(start, end);
        }
        replot();
    }
}

void MyCustomPlot::SetRelativeItem(MyGraphicsItem *item)
{
    m_item = item;
}

void MyCustomPlot::show()
{
    m_hide = false;
    if (!m_isBaiduMap) {
        QCustomPlot::show();
    } else {
        m_baiduMap->show();
    }
}

void MyCustomPlot::hide()
{
    m_hide = true;
    if (!m_isBaiduMap) {
        QCustomPlot::hide();
    } else {
        m_baiduMap->hide();
    }
}

void MyCustomPlot::ExitBaiduMap()
{
    m_isBaiduMap = false;
    QCustomPlot::show();
}

void MyCustomPlot::replot()
{
    QCustomPlot::replot();
}

void MyCustomPlot::SetAxisRange(QCPRange)
{
    xAxis->setRangeLower(m_xStart);
    yAxis->setRangeLower(m_yStart);
}

void MyCustomPlot::CreateTextDisplayer(bool xDirection)
{
    m_label = new QLabel(this);
    if (!m_label) {
        return;
    }

    double x, y, diff;
    QString str("两条线间的距离：%1");
    if (xDirection) {
        diff = qAbs(m_yLines[1]->first - m_yLines[0]->first);
        m_label->resize(250, 20);
        x = m_point2.x() - (m_point2.x() - m_point1.x())/2 - 125;
        y = this->height()/2;
    } else {
        m_label->resize(250, 20);
        diff = qAbs(m_xLines[1]->first - m_xLines[0]->first);
        y = m_point2.y() - (m_point2.y() - m_point1.y())/2 - 10;
        x = this->width()/2 - 125;
    }

    str = str.arg(diff);
    m_label->setText(str);
    m_label->move(x, y);
    m_label->show();
}

void MyCustomPlot::AdjustTextDisplayerPos()
{
    if (!m_label) {
        return;
    }

    double x, y, diff;
    QString str("两条线间的距离：%1");
    if (m_xLines[0] == nullptr) {
        diff = qAbs(m_yLines[1]->first - m_yLines[0]->first);
        x = m_point2.x() - (m_point2.x() - m_point1.x())/2 - 125;
        y = this->height()/2;
    } else {
        diff = qAbs(m_xLines[1]->first - m_xLines[0]->first);
        y = m_point2.y() - (m_point2.y() - m_point1.y())/2 - 10;
        x = this->width()/2 - 125;
    }

    str = str.arg(diff);
    m_label->setText(str);
    m_label->move(x, y);
    m_label->show();
}

void MyCustomPlot::SetBaseLine(int index)
{
    if (m_xDatas.count() < index) {
        return;
    }
    m_multilineLock.lock();
    m_baseLineIndex = index - 1;
    if (!m_baseGraph) {
        yAxis2->setTicks(true);
        yAxis2->setVisible(true);
        yAxis2->setNumberFormat("gbc");
        yAxis2->setSubTicks(true);
        yAxis2->grid()->setSubGridVisible(true);
        yAxis2->setTickLabels(true);
        yAxis2->setRange(DEFAULT_2D_Y2AXIS_START, DEFAULT_2D_Y2AXIS_END);
        yAxis->setRange(-m_yStart, m_yEnd);
        m_yStart = -m_yStart;
        m_y2Start = DEFAULT_2D_Y2AXIS_START;
        m_y2End = DEFAULT_2D_Y2AXIS_END;
        m_baseGraph = addGraph(xAxis, yAxis);
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

        m_multilineGraphs[i]->setValueAxis(yAxis2);
        m_multilineGraphs[i]->setData(m_xDatas[i], m_yDatas[i]);
    }

    if (m_mulWarnGraphs[0]) {
        QVector<double> xDatas, yDatas;
        m_mulWarnGraphs[0]->setData(xDatas, yDatas);
    }

    m_multilineLock.unlock();
    replot();

    m_item->SetBaseLine(index);

    std::cout << "set base line:" << index << std::endl;
}

void MyCustomPlot::RemoveBaseLine()
{
    if (m_baseLineIndex < 0) {
        return;
    }

    m_multilineLock.lock();
    if (m_baseGraph) {
        removeGraph(m_baseGraph);
        m_baseGraph = nullptr;
        yAxis2->setTicks(false);
        yAxis2->setVisible(true);
        yAxis2->setSubTicks(false);
        yAxis2->grid()->setSubGridVisible(false);
        yAxis2->setTickLabels(false);
    }

    yAxis->setRange(m_yStart, m_yEnd);
    int count = m_xDatas.count();
    for (int i = 0; i < count; ++i) {
        int count2 = m_xDatas[i].count();
        for (int j = 0; j < count2; ++j) {
            m_yDatas[i][j] = m_yDatas[i][j] + m_yBase[j];
        }

        m_multilineGraphs[i]->setValueAxis(yAxis);
        m_multilineGraphs[i]->setData(m_xDatas[i], m_yDatas[i]);
    }

    m_baseLineIndex = -1;
    m_multilineLock.unlock();
    replot();
    m_item->RemoveBaseLine();
}

bool MyCustomPlot::ChoosePoints(const QPointF &pos)
{
    double minDiff = 1000;
    int xIndex, yIndex;
    bool get = false;
    double x = xAxis->pixelToCoord(pos.x());
    double y = yAxis->pixelToCoord(pos.y());
    int count = m_xDatas[0].count();
    auto &vec = m_xDatas[0];        // 因为x轴的数值都是固定的，所以只拿一个的就可以了
    for (int j = 0; j < count; ++j) {
        double diff = qAbs(vec[j] - x);
        if (diff <= minDiff && diff <= 0.5) {
            minDiff = diff;
            xIndex = j;
            get = true;
        }

        if (get && diff > minDiff) {
            break;
        }
    }

    if (!get) {
        return false;
    }

    get = false;
    minDiff = 1000;
    for (int i = 0; i < m_xDatas.count(); ++i) {
        double diff = qAbs(m_yDatas[i][xIndex] - y);
        if (minDiff >= diff) {
            yIndex = i;
            minDiff = diff;
        }
    }

    if (minDiff > 0.5) {
        return false;
    }

    if (m_isShiftPressed) {
        m_points[yIndex]->push_back(new struct ChoosePoint(yIndex, xIndex,
                                                       QPointF(m_xDatas[yIndex][xIndex],
                                                               m_yDatas[yIndex][xIndex])));
    } else {
        std::cout << "cur vector count: " << m_points.count() << std::endl;
        if (m_points[yIndex]->empty()) {
            m_points[yIndex]->push_back(new struct ChoosePoint(yIndex, xIndex,
                                                               QPointF(m_xDatas[yIndex][xIndex],
                                                                       m_yDatas[yIndex][xIndex])));
        } else {
            while (m_points[yIndex]->count() > 1) {
                auto p = m_points[yIndex]->last();
                m_points[yIndex]->pop_back();
                delete p;
            }

            m_points[yIndex]->first()->lineIndex = yIndex;
            m_points[yIndex]->first()->pointIndex = xIndex;
            m_points[yIndex]->first()->point.setX(m_xDatas[yIndex][xIndex]);
            m_points[yIndex]->first()->point.setY(m_yDatas[yIndex][xIndex]);
        }
    }

    if (m_choosePointsGraph) {
        UpdateChoosePointsGraph();
    }

    return false;
}

void MyCustomPlot::UpdateChoosePointsGraph()
{
    if (!m_choosePointsGraph) {
        return;
    }

    QVector<double> xData, yData;
    GetChoosePointsData(xData, yData);
    m_choosePointsGraph->setData(xData, yData);
    replot();
}

void MyCustomPlot::keyPressEvent(QKeyEvent *event)
{
    if (m_highLightPoint) {
        if (event->key() == Qt::Key_Left) {
            if (GetPrevPointOnLine()) {
                UpdateChoosePointsGraph();
            }
        } else if (event->key() == Qt::Key_Right) {
            if (GetNextPointOnLine()) {
                UpdateChoosePointsGraph();
            }
        } else if (event->key() == Qt::Key_Delete) {
            for (auto vec : m_points) {
                for (auto p : *vec) {
                    if (p == m_highLightPoint) {
                        vec->removeOne(p);
                        delete m_highLightPoint;
                        m_highLightPoint = nullptr;
                        UpdateChoosePointsGraph();
                    }
                }
            }
        }
    }

    if (event->key() == Qt::Key_Shift) {
        m_isShiftPressed = true;
    }
}

void MyCustomPlot::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift) {
        m_isShiftPressed = false;
    }
}

void MyCustomPlot::ClearAllChoosedPoints()
{
    for (auto vec : m_points) {
        while (!vec->empty()) {
            auto p = vec->first();
            delete p;
            vec->pop_front();
        }
    }

    UpdateChoosePointsGraph();
}

bool MyCustomPlot::SetHighLightLine(const QPointF &pos)
{
    if (m_baseLineIndex != -1 || !m_multilineMode) {
        return false;
    }

    double minDiff = 1;
    int xIndex, yIndex;
    bool get = false;
    double x = xAxis->pixelToCoord(pos.x());
    double y = yAxis->pixelToCoord(pos.y());
    int count = m_xDatas[0].count();
    auto &vec = m_xDatas[0];        // 因为x轴的数值都是固定的，所以只拿一个的就可以了
    for (int j = 0; j < count; ++j) {
        double diff = qAbs(vec[j] - x);
        if (diff <= minDiff && diff <= 1) {
            minDiff = diff;
            xIndex = j;
            get = true;
        }

        if (get && diff > minDiff) {
            break;
        }
    }

    if (!get) {
        return false;
    }

    get = false;
    minDiff = 1;
    for (int i = 0; i < m_xDatas.count(); ++i) {
        int diff = qAbs(m_yDatas[i][xIndex] - y);
        if (minDiff >= diff && diff < 1) {
            get = true;
            yIndex = i;
        }
    }

    if (!get) {
        return false;
    }

    if (m_highLightLineIndex >= 0) {    // 之前有高亮的线，使其变细
        QPen pen;
        pen.setColor(GetColor(m_highLightLineIndex+1));
        pen.setWidth(1);
        m_multilineGraphs[m_highLightLineIndex]->setPen(pen);
    }

    QPen pen;
    pen.setColor(GetColor(yIndex+1));
    pen.setWidth(4);
    m_multilineGraphs[yIndex]->setPen(pen);
    m_highLightLineIndex = yIndex;
    replot();

    return false;
}

bool MyCustomPlot::GetPrevPointOnLine()
{
    if (!m_highLightPoint) {
        return false;
    }

    if (m_highLightPoint->pointIndex > 0) {
        m_highLightPoint->pointIndex--;
        m_highLightPoint->point.setX(m_xDatas[m_highLightPoint->lineIndex][m_highLightPoint->pointIndex]);
        m_highLightPoint->point.setY(m_yDatas[m_highLightPoint->lineIndex][m_highLightPoint->pointIndex]);
        return true;
    }

    return false;
}

bool MyCustomPlot::GetNextPointOnLine()
{
    if (!m_highLightPoint) {
        return false;
    }

    if (m_xDatas[m_highLightPoint->lineIndex].count() >= m_highLightPoint->pointIndex) {
        m_highLightPoint->pointIndex++;
        m_highLightPoint->point.setX(m_xDatas[m_highLightPoint->lineIndex][m_highLightPoint->pointIndex]);
        m_highLightPoint->point.setY(m_yDatas[m_highLightPoint->lineIndex][m_highLightPoint->pointIndex]);
        return true;
    }

    return false;
}

void MyCustomPlot::AddChoosePointsGraph()
{
    if (m_choosePointMode) {
        m_choosePointsGraph = addGraph();
        QPen pen;
        pen.setColor(Qt::red);
        pen.setWidth(4);
        m_choosePointsGraph->setPen(pen);
        m_choosePointsGraph->setLineStyle(QCPGraph::lsNone);
        m_choosePointsGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 2));
    }
}

void MyCustomPlot::RemoveChoosePointsGraph()
{
    if (m_choosePointsGraph) {
        removeGraph(m_choosePointsGraph);
    }
}

void MyCustomPlot::GetChoosePointsData(QVector<double> &x, QVector<double> &y)
{
    for (auto vec : m_points) {
        for (auto &p : *vec) {
            x.push_back(p->point.x());
            y.push_back(p->point.y());
        }
    }
}

bool MyCustomPlot::ShowPressedChoosePoint(const QPointF &point)
{
    double x = xAxis->pixelToCoord(point.x());
    double y = yAxis->pixelToCoord(point.y());
    for (auto vec : m_points) {
        for (auto &p : *vec) {
            //std::cout << "show pressed x1:" << x << " x2:" << p.x() << " y1:" << y << " y2:" << p.y() << std::endl;
            if ((qAbs(p->point.x() - x) <= 1) && (qAbs(p->point.y() - y) < 1)) {
                m_highLightPoint = p;
                // show tip
                QString str("%1,%2");
                str = str.arg(p->point.x()).arg(p->point.y());
                QToolTip::showText(QPoint(point.x() + GRAPHICS_SCENE_WIDTH, point.y()), str, this);
                return true;
            }
        }
    }

    m_highLightPoint = nullptr;
    return false;
}

bool MyCustomPlot::SetMultiLineCount(int count)
{
    if (m_plotType != PLOT_TYPE::PLOT_2D_HIS_MAIN) {
        return false;
    }

    for (int i = 0; i < count - 1; ++i) {
        QCPGraph *graph = addGraph();
        if (!graph) {
            return false;
        }
        m_multilineGraphs.append(graph);
        graph->setPen(GetColor(m_multilineGraphs.count()));
    }

    m_multilineCount = count;
    m_curMulLineCount = 0;

    return true;
}

void MyCustomPlot::BuildHisSubPlotDatas(const QPointF &pos, QVector<double> &xDatas, QVector<double> &yDatas)
{
    bool baseLineMode = (m_baseLineIndex >= 0);
    double minDiff = 100000;
    int xIndex;
    bool get = false;
    double x = pos.x();
    int count = m_xDatas[0].count();
    auto &vec = m_xDatas[0];        // 因为x轴的数值都是固定的，所以只拿一个的就可以了
    for (int i = 0; i < count; ++i) {
        double diff = qAbs(vec[i] - x);
        if (diff <= minDiff/* && diff <= 1*/) {
            minDiff = diff;
            xIndex = i;
            get = true;
        }

        if (get && diff > minDiff) {
            break;
        }
    }

    if (!get || (m_yDatas.count() != m_times.count())) {
        std::cout << "error:没有找到子图相应点的数据" << std::endl;
        return;
    }

    for (int i = 0; i < m_yDatas.count(); ++i) {
        xDatas.push_back(m_times[i]);
        if (!baseLineMode) {
            yDatas.push_back(m_yDatas[i][xIndex]);
        } else {
            yDatas.push_back(m_yDatas[i][xIndex] + m_yBase[i]);
        }
    }
}

void MyCustomPlot::InterChoosePointMode()
{
    AddChoosePointsGraph();

    for (int i = 0; i < m_multilineCount; ++i) {
        m_points.push_back(new QVector<struct ChoosePoint*>());
    }
}

void MyCustomPlot::ExitChoosePointMode()
{
    RemoveChoosePointsGraph();

    while(!m_points.empty()) {
        auto vec = m_points.first();
        while (!vec->empty()) {
            auto p = vec->first();
            delete p;
            vec->pop_front();
        }

        delete vec;
        m_points.pop_front();
    }
}

void MyCustomPlot::ShowMenu()
{
    m_showMenu = true;
}

void MyCustomPlot::ExitMenu()
{
    m_showMenu = false;
}

void MyCustomPlot::SetBaiduMapCenter(const QString &centerName)
{
    m_baiduMapCenter = centerName;
}

QString MyCustomPlot::GetMeasureDataOfPos(double lon, double lat)
{
    QString ret;
    const double eps = 1e-7;
    for (int i = 0; i < m_lons.count(); ++i) {
        if (qAbs(m_lons[i] - lon) < eps && (m_lats[i] - lat) < eps) {
            if (m_xDatas.count() > 0 && m_xDatas[0].count() > i) {
                ret = QString(" \n 测量值：" + QString::number(m_xDatas[0][i]) + "," + QString::number(m_yDatas[0][i]));
                ret += QString(" \n 告警值：" + QString::number(m_warnings[i]));
            }
        }
    }

    return ret;
}

void MyCustomPlot::wheelEvent(QWheelEvent *event)
{
    m_isWheeled = true;
    QCustomPlot::wheelEvent(event);
}

void MyCustomPlot::SetAppPath(const QString &appPath)
{
    m_appPath = appPath;
}
