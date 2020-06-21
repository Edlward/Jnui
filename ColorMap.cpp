#pragma execution_character_set("utf-8")

#include "ColorMap.h"
#include "DataReceiver.h"
#include <QMenu>
#include <QAction>
#include <iostream>
#include "MyGraphicsScene.h"
#include "BaiduMap.h"
#include "MyGraphicsItem.h"

ColorMap::ColorMap(MyGraphicsScene *scene, QWidget *parent, PLOT_TYPE type)
    : QCustomPlot(parent), m_scene(scene), m_parentWidget(parent), m_plotType(type), m_xStart(0),
      m_xEnd(0), m_yStart(0), m_yEnd(0), m_preX(0), m_recordCount(100), m_item(nullptr),
      m_keySize(0), m_valueSize(0), m_baiduMap(nullptr),
      m_isBaiduMap(false), m_rubberBand(nullptr), m_pauseDataUpdate(false),
      m_choosePointMode(false), m_choosePointsGraph(nullptr), m_isShiftPressed(false),
      m_showMenu(false), /*m_isWheeled(false), */m_dragMode(false), m_dragPos(nullptr)
{
    qRegisterMetaType<QCPRange>("QCPRange");
    setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    axisRect()->setupFullAxesBox(true);
    m_colorMap = new QCPColorMap(xAxis, yAxis);
    m_colorScale = new QCPColorScale(this);
    m_colorScale->setDataRange(QCPRange(-120, 40));
    plotLayout()->addElement(0, 1, m_colorScale);
    m_colorScale->setType(QCPAxis::atRight);
    m_colorMap->setColorScale(m_colorScale);
    m_colorScale->axis()->setLabel("Color scale");

    m_colorMap->setGradient(QCPColorGradient::gpSpectrum);
    m_colorMap->rescaleDataRange();
    m_marginGroup = new QCPMarginGroup(this);
    axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginGroup);
    m_colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginGroup);
    rescaleAxes();
    resize(parent->width(), parent->height());

    m_warnGraph = addGraph();
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(6);
    m_warnGraph->setPen(pen);
    m_warnGraph->setLineStyle(QCPGraph::lsNone);
    m_warnGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssStar, 10));

    if (PLOT_TYPE::PLOT_3D_SUB_POS == type || PLOT_TYPE::PLOT_3D_HIS_POS == type) {
        double time = QDateTime::currentSecsSinceEpoch();
        yAxis->setRange(time, time+DEFALUT_AXIS_STEP);
        m_yStart = time;
        m_yEnd = time+DEFALUT_AXIS_STEP;

        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);//日期做Y轴
        dateTicker->setDateTimeFormat("yyyy-MM-dd hh:mm:ss");   //日期格式
        dateTicker->setTickCount(3);
        yAxis->setTicker(dateTicker);       // 设置y轴为时间轴
        yAxis->setTickLabelRotation(-90);   // 刻度值逆时针旋转90度
        m_yName = "Time(s)";
        yAxis->setLabel(m_yName);
    } else if (PLOT_TYPE::PLOT_3D_SUB_TIME == type || PLOT_TYPE::PLOT_3D_HIS_TIME == type) {
        double time = QDateTime::currentSecsSinceEpoch();
        xAxis->setRange(time, time+DEFALUT_AXIS_STEP);
        m_xStart = time;
        m_xEnd = time+DEFALUT_AXIS_STEP;

        QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);//日期做X轴
        dateTicker->setDateTimeFormat("yyyy-MM-dd hh:mm:ss");//日期格式(可参考QDateTime::fromString()函数)
        dateTicker->setTickCount(5);
        xAxis->setTicker(dateTicker);//设置X轴为时间轴
        m_xName = "Time(s)";
        xAxis->setLabel(m_xName);
    }

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
    m_label = nullptr;

    move(GRAPHICS_SCENE_WIDTH, 0);
    replot();
}

ColorMap::~ColorMap()
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

    while(!m_recordZ.empty()) {
        QVector<double> *tmp = m_recordZ.first();
        m_recordZ.pop_front();
        delete tmp;
    }

    delete m_marginGroup;
    delete m_colorScale;
}

void ColorMap::UpdateData(QCPColorMapData *mapData)
{
    if (!m_pauseDataUpdate) {
        m_colorMap->setData(mapData);
    }
}

void ColorMap::UpdateData(QVector<double> &zData)
{
    if (m_pauseDataUpdate) {
        return;
    }

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

        if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS || m_plotType == PLOT_TYPE::PLOT_3D_SUB_TIME) {
            m_xStart += DEFALUT_AXIS_STEP;    //tmp
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
        if (m_plotType == PLOT_TYPE::PLOT_3D_SUB_POS  && time >= m_yEnd) {
            m_yEnd = time + DEFALUT_AXIS_STEP;
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

    SetRange(m_xStart, m_xEnd, m_yStart, m_yEnd);
    m_colorMap->setData(data);
}

void ColorMap::UpdateData(const QVector<double> &xData, const QVector<double> &yData,
                          const QVector<double> &zData, QCPColorMapData *mapData, qint64 time)
{
    if (!m_pauseDataUpdate) {
        if (time == 0) { // 数据采集模式
            m_colorMap->setData(mapData);
            m_xDatas.clear();
            m_xDatas.push_back(xData);
            m_yDatas.clear();
            m_yDatas.push_back(yData);
            m_zDatas.clear();
            m_zDatas.push_back(zData);
        } else {        // 观察历史数据模式
            if (m_xDatas.count() == 0) {
                m_colorMap->setData(mapData);
            } else {
                delete mapData;
            }

            m_times.push_back(time);
        }

        m_xDatas.push_back(xData);
        m_yDatas.push_back(yData);
        m_zDatas.push_back(zData);
    }
}

void ColorMap::SetRange(double xStart, double xEnd, double yStart, double yEnd)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
        m_xStart = xStart;
        m_xEnd = xEnd;
        m_yStart = yStart;
        m_yEnd = yEnd;
        xAxis->setRange(xStart, xEnd);
        yAxis->setRange(yStart, yEnd);
        break;
    case PLOT_TYPE::PLOT_3D_SUB_POS:
        m_xStart = xStart;
        m_xEnd = xEnd;
        xAxis->setRange(xStart, xEnd);
        break;
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
        m_yStart = yStart;
        m_yEnd = yEnd;
        yAxis->setRange(yStart, yEnd);
        break;
    default:
        break;
    }
}

void ColorMap::SetXYLable(const QString &xName, const QString &yName)
{
    switch (m_plotType) {
    case PLOT_TYPE::PLOT_3D_MAIN:
    case PLOT_TYPE::PLOT_3D_HIS_MAIN:
        m_xName = xName;
        m_yName = yName;
        xAxis->setLabel(xName);
        yAxis->setLabel(yName);
        break;
    case PLOT_TYPE::PLOT_3D_SUB_POS:
    case PLOT_TYPE::PLOT_3D_HIS_POS:
        m_xName = xName;
        xAxis->setLabel(xName);
        break;
    case PLOT_TYPE::PLOT_3D_SUB_TIME:
    case PLOT_TYPE::PLOT_3D_HIS_TIME:
        m_yName = yName;
        yAxis->setLabel(yName);
        break;
    default:
        break;
    }
}

void ColorMap::SetXLabel(const QString &xName)
{
    if (PLOT_TYPE::PLOT_3D_SUB_POS == m_plotType ||
        PLOT_TYPE::PLOT_3D_HIS_POS == m_plotType) {
        m_xName = xName;
        xAxis->setLabel(xName);
    }
}

void ColorMap::SetYLabel(const QString &yName)
{
    if (PLOT_TYPE::PLOT_2D_SUB == m_plotType ||
        PLOT_TYPE::PLOT_2D_HIS_SUB == m_plotType) {
        m_yName = yName;
        yAxis->setLabel(yName);
    }

    if (PLOT_TYPE::PLOT_3D_SUB_TIME == m_plotType ||
        PLOT_TYPE::PLOT_3D_HIS_TIME == m_plotType) {
        m_yName = yName;
        yAxis->setLabel(yName);
    }
}

bool ColorMap::SetSubPlotInfo(const QPointF &point, int recordCount)
{
    if (recordCount > MAX_3D_RECORD_COUNT) {
        return false;
    }

    m_startPoint = point;
    m_recordCount = recordCount;
    return true;
}

void ColorMap::SetRelativeItem(MyGraphicsItem *item)
{
    m_item = item;
}

void ColorMap::show()
{
    m_hide = false;
    //replot();
    if (!m_isBaiduMap) {
        QCustomPlot::show();
    } else {
        m_baiduMap->show();
    }
}

void ColorMap::hide()
{
    m_hide = true;
    if (!m_isBaiduMap) {
        QCustomPlot::hide();
    } else {
        m_baiduMap->hide();
    }
}

void ColorMap::ExitBaiduMap()
{
    m_isBaiduMap = false;
    QCustomPlot::show();
}

void ColorMap::SetAxisRangeByUser(const QString &axisName, int axisIndex)
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

void ColorMap::ShowMenu(QMouseEvent *event)
{
    // 显示右键下拉框
    QMenu menu(this);
    QAction *action;
    if (!m_pauseDataUpdate && IsMainView()) {
        action = new QAction(tr("生成时间子图"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            double x = xAxis->pixelToCoord(event->localPos().x());
            double y = yAxis->pixelToCoord(event->localPos().y());
            QPointF real_pos(x, y);
            if (m_plotType == PLOT_TYPE::PLOT_3D_MAIN) {
                MyGraphicsItem *item = m_scene->NewSubView(real_pos, PLOT_TYPE::PLOT_3D_SUB_TIME);
                item->SetYLabel(m_yName);
            } else {
                MyGraphicsItem *item = m_scene->NewSubView(real_pos, PLOT_TYPE::PLOT_3D_HIS_TIME, m_times.count());
                if (!item) {
                    return;
                }

                item->SetXYRange(m_times.first(), m_times.last(), m_yStart, m_yEnd);
                item->SetYLabel(m_yName);
                BuildTimeSubViewDatas(event->localPos(), item);
                item->replot();
            }
        });

        action = new QAction(tr("生成位置子图"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            double x = xAxis->pixelToCoord(event->localPos().x());
            double y = yAxis->pixelToCoord(event->localPos().y());
            QPointF real_pos(x, y);
            if (m_plotType == PLOT_TYPE::PLOT_3D_MAIN) {
                MyGraphicsItem *item = m_scene->NewSubView(real_pos, PLOT_TYPE::PLOT_3D_SUB_POS);
                if (item) {
                    item->SetXLabel(m_xName);
                }
            } else {
                MyGraphicsItem *item = m_scene->NewSubView(real_pos, PLOT_TYPE::PLOT_3D_HIS_POS, m_times.count());
                if (!item) {
                    return;
                }

                item->SetXYRange(m_xStart, m_xEnd, m_times.first(), m_times.last());
                item->SetXLabel(m_xName);
                BuildPosSubViewDatas(event->localPos(), item);
                item->replot();
            }
        });

        if (CanEnterBaiduMap()) {
            action = new QAction(tr("进入地图模式"), this);
            menu.addAction(action);
            connect(action, &QAction::triggered, this,
                [&](){
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
            });
        }
    }

    if (!m_pauseDataUpdate) {
        action = new QAction(tr("新增纵线"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            if (m_xLines[0] != nullptr) {
                return;
            }
            if (m_yLines[0] == nullptr || m_yLines[1] == nullptr) {
                double x = xAxis->pixelToCoord(event->localPos().x());
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
                    m_point1.setX(event->localPos().x());
                    m_point1.setY(event->localPos().y());
                } else {
                    m_yLines[1] = new QPair<double, QCPGraph*>(x, graph);
                    m_point2.setX(event->localPos().x());
                    m_point2.setY(event->localPos().y());
                    CreateTextDisplayer(true);
                }

                if (!m_hide) {
                    replot();
                }
            }
        });

        action = new QAction(tr("新增横线"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            if (m_yLines[0] != nullptr) {
                return;
            }
            if (m_xLines[0] == nullptr || m_xLines[1] == nullptr) {
                double y = yAxis->pixelToCoord(event->localPos().y());
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
                    m_point1.setX(event->localPos().x());
                    m_point1.setY(event->localPos().y());
                } else {
                    m_xLines[1] = new QPair<double, QCPGraph*>(y, graph);
                    m_point2.setX(event->localPos().x());
                    m_point2.setY(event->localPos().y());
                    CreateTextDisplayer(false);
                }

                if (!m_hide) {
                    replot();
                }
            }
        });

        action = new QAction(tr("删除所有线"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
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
        });

        if (m_plotType != PLOT_TYPE::PLOT_3D_SUB_TIME && m_plotType != PLOT_TYPE::PLOT_3D_HIS_TIME) {
            action = new QAction(tr("设置X轴范围"), this);
            menu.addAction(action);
            connect(action, &QAction::triggered, this,
                [&](){
                SetAxisRangeByUser(QString("请输入X轴范围: "), 1);
            });
        }

        if (m_plotType != PLOT_TYPE::PLOT_3D_SUB_POS && m_plotType != PLOT_TYPE::PLOT_3D_HIS_POS) {
            action = new QAction(tr("设置Y轴范围"), this);
            menu.addAction(action);
            connect(action, &QAction::triggered, this,
                [&](){
                SetAxisRangeByUser(QString("请输入Y轴范围: "), 2);
            });
        }
    }

    action = new QAction(tr("还原"), this);
    menu.addAction(action);
    connect(action, &QAction::triggered, this,
        [&](){
        // m_isWheeled = false;
        xAxis->setRange(m_xStart, m_xEnd);
        yAxis->setRange(m_yStart, m_yEnd);
        replot();
    });

    if (m_plotType != PLOT_TYPE::PLOT_3D_HIS_MAIN &&
        m_plotType != PLOT_TYPE::PLOT_3D_HIS_POS &&
        m_plotType != PLOT_TYPE::PLOT_3D_HIS_TIME) {
        action = new QAction(this);
        if (m_pauseDataUpdate) {
            action->setText(QString("恢复数据显示更新"));
        } else {
            action->setText(QString("暂停数据显示更新"));
        }
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_pauseDataUpdate = !m_pauseDataUpdate;
            if (!m_pauseDataUpdate) {
                ClearAllChoosedPoints();
            }
            if (!m_pauseDataUpdate && m_choosePointMode) { // 恢复数据更新时，如果在取点模式下直接退出取点模式
                ClearAllChoosedPoints();
                m_choosePointMode = false;
                RemoveChoosePointsGraph();
            }
        });
    }

    if (m_pauseDataUpdate || IsHisView()) {
        action = new QAction(this);
        if (!m_choosePointMode) {
            action->setText(QString("进入取点模式"));
        } else {
            action->setText(QString("退出取点模式"));
        }
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_choosePointMode = !m_choosePointMode;
            ClearAllChoosedPoints();
            if (m_choosePointMode) {
                AddChoosePointsGraph();
            } else {
                RemoveChoosePointsGraph();
            }
        });
    }

    if (!m_choosePointMode) {
        action = new QAction(this);
        if (!m_dragMode) {
            action->setText(QString("进入拖动模式"));
        } else {
            action->setText(QString("退出拖动模式"));
        }
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_dragMode = !m_dragMode;
            if (m_dragMode) {
                setCursor(QCursor(Qt::PointingHandCursor));
            } else {
                unsetCursor();
                xAxis->setRange(m_xStart, m_xEnd);
                yAxis->setRange(m_yStart, m_yEnd);
            }
        });
    }

    if ((m_pauseDataUpdate || IsHisView()) && m_choosePointMode) {
        action = new QAction(this);
        action->setText(QString("清除所有取点"));
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            ClearAllChoosedPoints();
        });
    }

    if ((m_pauseDataUpdate || IsHisView()) && m_choosePointMode && m_highLightPoint) {
        action = new QAction("删除此取点", this);
        m_showMenu = true;
        menu.addAction(action);
        connect(action, &QAction::triggered, this, [&] {
            if (m_highLightPoint) {
                m_points.removeOne(m_highLightPoint);
                delete m_highLightPoint;
                m_highLightPoint = nullptr;
                UpdateChoosePointsGraph();
            }
        });
    }

    menu.exec(QCursor::pos());
    m_showMenu = false;
}

void ColorMap::mousePressEvent(QMouseEvent *event)
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
    }

    if (event->button() == Qt::RightButton) {
        ShowMenu(event);
    } else if (event->button() == Qt::LeftButton) {
        bool moveLine = false;
        if (m_xLines[0] != nullptr) {
            double y = yAxis->pixelToCoord(event->pos().y());
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
            double x = xAxis->pixelToCoord(event->pos().x());
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

void ColorMap::mouseMoveEvent(QMouseEvent *event)
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
            double y = yAxis->pixelToCoord(event->pos().y());
            if (m_xLines[0]->first == m_preX) {
                m_preX = m_xLines[0]->first;
                QVector<double> xVec;
                xVec.append(m_xStart);
                xVec.append(m_xEnd);
                QVector<double> yVec;
                yVec.append(y);
                yVec.append(y);
                m_xLines[0]->second->setData(xVec, yVec);
                m_point1.setX(event->pos().x());
                m_point1.setY(event->pos().y());
            } else if (m_xLines[1] != nullptr && m_xLines[1]->first  == m_preX) {
                m_preX = m_xLines[1]->first;
                QVector<double> xVec;
                xVec.append(m_xStart);
                xVec.append(m_xEnd);
                QVector<double> yVec;
                yVec.append(y);
                yVec.append(y);
                m_xLines[1]->second->setData(xVec, yVec);
                m_point2.setX(event->pos().x());
                m_point2.setY(event->pos().y());
            }
        } else if (m_yLines[0] != nullptr) {
            double x = xAxis->pixelToCoord(event->pos().x());
            if (m_yLines[0]->first  == m_preX) {
                m_preX = m_yLines[0]->first;
                QVector<double> xVec;
                xVec.append(x);
                xVec.append(x);
                QVector<double> yVec;
                yVec.append(m_yStart);
                yVec.append(m_yEnd);
                m_yLines[0]->second->setData(xVec, yVec);
                m_point1.setX(event->pos().x());
                m_point1.setY(event->pos().y());
            } else if (m_yLines[1] != nullptr && m_yLines[1]->first  == m_preX) {
                m_preX = m_yLines[1]->first;
                QVector<double> xVec;
                xVec.append(x);
                xVec.append(x);
                QVector<double> yVec;
                yVec.append(m_yStart);
                yVec.append(m_yEnd);
                m_yLines[1]->second->setData(xVec, yVec);
                m_point2.setX(event->pos().x());
                m_point2.setY(event->pos().y());
            }
        }

        AdjustTextDisplayerPos();
        replot();
    } else {
        ShowPressedChoosePoint(event->localPos());
    }
}

void ColorMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_dragMode && m_dragPos) {
        int x = xAxis->pixelToCoord(m_dragPos->x());
        int y = yAxis->pixelToCoord(m_dragPos->y()) - yAxis->pixelToCoord(event->pos().y());
        x -= xAxis->pixelToCoord(event->pos().x());
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

        replot();

        delete m_dragPos;
        m_dragPos = nullptr;
        return;
    }

    if (m_rubberBand) {
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
            double y = yAxis->pixelToCoord(event->pos().y());
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
                m_point1.setX(event->pos().x());
                m_point1.setY(event->pos().y());
                AdjustTextDisplayerPos();
                //std::cout<< "show:" << m_preX << std::endl;
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
                m_point2.setX(event->pos().x());
                m_point2.setY(event->pos().y());
                AdjustTextDisplayerPos();
                replot();
            }
        } else if (m_yLines[0] != nullptr) {
            double x = xAxis->pixelToCoord(event->pos().x());
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
                m_point1.setX(event->pos().x());
                m_point1.setY(event->pos().y());
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
                m_point2.setX(event->pos().x());
                m_point2.setY(event->pos().y());
                AdjustTextDisplayerPos();
                replot();
            }
        }

        m_preX = 0;
    }
}

void ColorMap::SetAxisRange(QCPRange)
{
    xAxis->setRangeLower(m_xStart);
    yAxis->setRangeLower(m_yStart);
}

void ColorMap::SetLonLats(const QVector<double> &lons, const QVector<double> &lats)
{
    if (m_lons.count() == 0) {
        m_lons = lons;
        m_lats = lats;
    }

    if (m_isBaiduMap) {
        m_baiduMap->UpdateData(m_lons, m_lats);
    }
}

void ColorMap::UpdateWarnings(const QVector<double> &warnings)
{
    if (m_pauseDataUpdate) {
        return;
    }

    if (m_plotType == PLOT_TYPE::PLOT_3D_MAIN ||  // 3D主图
        (m_plotType == PLOT_TYPE::PLOT_3D_HIS_MAIN &&  m_xDatas.count() == 0)) { // 3D历史主图只显示第一组数据
        QVector<double> xData, yData;
        if (!m_warnings.empty()) {
            m_warnings.clear();
        }

        for (int i = 0; i < warnings.count(); ++i) {
            if (warnings[i] > 0) {
                xData.push_back(m_xDatas[0][i]);
                yData.push_back(0);             // 显示在x轴上
            }

            m_warnings.append(warnings[i]);
        }

        m_warnGraph->setData(xData, yData);

        if (m_isBaiduMap) {
            m_baiduMap->UpdateWarnings(warnings);
        }
    }
}

void ColorMap::CreateTextDisplayer(bool xDirection)
{
    m_label = new QLabel(this);
    if (!m_label) {
        return;
    }

    double x, y, diff;
    QString str("两条线见的距离：%1");
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

void ColorMap::AdjustTextDisplayerPos()
{
    if (!m_label) {
        return;
    }

    double x, y, diff;
    QString str("两条线见的距离：%1");
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

void ColorMap::AddChoosePointsGraph()
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

void ColorMap::RemoveChoosePointsGraph()
{
    if (m_choosePointsGraph) {
        removeGraph(m_choosePointsGraph);
        m_choosePointsGraph = nullptr;
    }
}

void ColorMap::GetChoosePointsData(QVector<double> &x, QVector<double> &y)
{
    for (auto p : m_points) {
        x.push_back(p->point.x());
        y.push_back(p->point.y());
    }
}

bool ColorMap::ShowPressedChoosePoint(const QPointF &point)
{
    double x = xAxis->pixelToCoord(point.x());
    double y = yAxis->pixelToCoord(point.y());
    for (auto p : m_points) {
        //std::cout << "show pressed x1:" << x << " x2:" << p.x() << " y1:" << y << " y2:" << p.y() << std::endl;
        if ((qAbs(p->point.x() - x) <= 1) && (qAbs(p->point.y() - y) < 1)) {
            m_highLightPoint = p;
            // std::cout << "show pressed" << std::endl;
            // show tip
            QString str("%1,%2,%3");
            str = str.arg(p->point.x()).arg(p->point.y()).arg(p->zData);
            QToolTip::showText(QPoint(point.x() + GRAPHICS_SCENE_WIDTH, point.y()), str, this);
            return true;
        }
    }

    m_highLightPoint = nullptr;
    return false;
}

void ColorMap::UpdateChoosePointsGraph()
{
    if (!m_choosePointsGraph) {
        return;
    }

    QVector<double> xData, yData;
    GetChoosePointsData(xData, yData);
    m_choosePointsGraph->setData(xData, yData);
    replot();
}

void ColorMap::keyPressEvent(QKeyEvent *event)
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
            for (auto p : m_points) {
                if (p == m_highLightPoint) {
                    m_points.removeOne(p);
                    delete m_highLightPoint;
                    m_highLightPoint = nullptr;
                    UpdateChoosePointsGraph();
                    break;
                }
            }
        }
    }

    if (event->key() == Qt::Key_Shift) {
        m_isShiftPressed = true;
    }
}

void ColorMap::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift) {
        m_isShiftPressed = false;
    }
}

bool ColorMap::ChoosePoints(const QPointF &pos)
{
    double minDiff = 1000;
    int xIndex, yIndex;
    bool get = false;
    double x = xAxis->pixelToCoord(pos.x());
    double y = yAxis->pixelToCoord(pos.y());
    int count = m_xDatas[0].count();
    for (int j = 0; j < count; ++j) {
        double diff = qAbs(m_xDatas[0][j] - x);
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
        std::cout<< "ChoosePoint not get x" << std::endl;
        return false;
    }

    get = false;
    minDiff = 1000;
    for (int i = 0; i < m_yDatas[0].count(); ++i) {
        double diff = qAbs(m_yDatas[0][i] - y);
        if (minDiff >= diff) {
            yIndex = i;
            minDiff = diff;
        }
    }

    if (minDiff > 0.5) {
        return false;
    }

    double z = m_zDatas[0][(xIndex+1) * (yIndex+1)];

    if (m_isShiftPressed) {
        m_points.push_back(new struct ChoosePoint(yIndex, xIndex,
                                                  QPointF(m_xDatas[0][xIndex],
                                                          m_yDatas[0][yIndex]), z));
    } else {
        if (m_points.empty()) {
            m_points.push_back(new struct ChoosePoint(yIndex, xIndex,
                                                      QPointF(m_xDatas[0][xIndex],
                                                              m_yDatas[0][yIndex]), z));
        } else {
            while (m_points.count() > 1) {
                auto p = m_points.last();
                m_points.pop_back();
                delete p;
            }

            m_points.first()->lineIndex = yIndex;
            m_points.first()->pointIndex = xIndex;
            m_points.first()->point.setX(m_xDatas[0][xIndex]);
            m_points.first()->point.setY(m_yDatas[0][yIndex]);
        }
    }

    if (m_choosePointsGraph) {
        UpdateChoosePointsGraph();
    }

    return false;
}

bool ColorMap::GetPrevPointOnLine()
{
    if (!m_highLightPoint) {
        return false;
    }

    if (m_highLightPoint->pointIndex > 0) {
        m_highLightPoint->pointIndex--;
        m_highLightPoint->point.setX(m_xDatas[0][m_highLightPoint->pointIndex]);
        m_highLightPoint->point.setY(m_yDatas[0][m_highLightPoint->pointIndex]);
        return true;
    }

    return false;
}

bool ColorMap::GetNextPointOnLine()
{
    if (!m_highLightPoint) {
        return false;
    }

    if (m_xDatas.count() >= m_highLightPoint->pointIndex) {
        m_highLightPoint->pointIndex++;
        m_highLightPoint->point.setX(m_xDatas[0][m_highLightPoint->pointIndex]);
        m_highLightPoint->point.setY(m_yDatas[0][m_highLightPoint->pointIndex]);
        return true;
    }

    return false;
}

void ColorMap::ClearAllChoosedPoints()
{
    if (!m_points.empty()) {
        while(!m_points.empty()) {
            auto p = m_points.first();
            delete p;
            m_points.pop_front();
        }
    }

    UpdateChoosePointsGraph();
}

void ColorMap::BuildTimeSubViewDatas(const QPointF &pos, MyGraphicsItem *item)
{
    if (m_xDatas.empty()) {
        return;
    }

    double minDiff = 1000;
    int xIndex;
    bool get = false;
    double x = xAxis->pixelToCoord(pos.x());
    int count = m_xDatas[0].count();
    for (int j = 0; j < count; ++j) {
        double diff = qAbs(m_xDatas[0][j] - x);
        if (diff <= minDiff) {
            minDiff = diff;
            xIndex = j;
            get = true;
        }

        if (get && diff > minDiff) {    // 距离是从小到大排序的，后面的无需再遍历
            break;
        }
    }

    int z = 0;
    for (int i = 0; i < m_times.count(); ++i) {
        QVector<double> tmp;
        for (int j = 0; j < m_yDatas[i].count(); ++j) {
            tmp.append(m_zDatas[i][xIndex*j+z++]);
        }
        item->UpdateData(tmp);
        item->SetValueSize(m_yDatas[0].count());
        tmp.clear();
        z = 0;
    }
}

void ColorMap::BuildPosSubViewDatas(const QPointF &pos, MyGraphicsItem *item)
{
    if (m_xDatas.empty()) {
        return;
    }

    double minDiff = 1000; // 随便一个大值
    int yIndex;
    bool get = false;
    double y = yAxis->pixelToCoord(pos.y());
    int count = m_yDatas[0].count();
    for (int j = 0; j < count; ++j) {
        double diff = qAbs(m_yDatas[0][j] - y);
        if (diff <= minDiff) {
            minDiff = diff;
            yIndex = j;
            get = true;
        }
    }

    int z = 0;
    for (int i = 0; i < m_times.count(); ++i) {
        QVector<double> tmp;
        for (int j = 0; j < m_xDatas[i].count(); ++j) {
            tmp.append(m_zDatas[i][yIndex*j + z++]);
        }
        item->UpdateData(tmp);
        item->SetKeySize(m_xDatas[0].count());
        tmp.clear();
        z = 0;
    }
}

void ColorMap::SetBaiduMapCenter(const QString &centerName)
{
    m_baiduMapCenter = centerName;
}

QString ColorMap::GetMeasureDataOfPos(double lon, double lat)
{
    QString ret;
    const double eps = 1e-7;
    for (int i = 0; i < m_lons.count(); ++i) {
        if ((qAbs(m_lons[i] - lon) < eps) && ((m_lats[i] - lat) < eps)) {
            if (m_xDatas.count() > 0 && m_xDatas[0].count() > i) {
                ret = QString(" \n 测量值：" + QString::number(m_xDatas[0][i]) + "," + QString::number(m_yDatas[0][i]));
                ret += QString(" \n 告警值：" + QString::number(m_warnings[i]));
                //std::cout << "GetMeasureDataOfPos:" << ret.toStdString() << std::endl;
            }
        }
    }

    return ret;
}

void ColorMap::SetAppPath(const QString &appPath)
{
    m_appPath = appPath;
}

bool ColorMap::CanEnterBaiduMap()
{
    return (!m_lons.empty() && m_lons.count() > 0);
}
