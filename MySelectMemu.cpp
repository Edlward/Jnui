#include "MySelectMemu.h"
#include <QAction>
#include <QMessageBox>
#include <QMenu>
#include <QLineSeries>
#include <QtCharts/QChart>
#include "MyCustomPlot.h"
#include "Common.h"
#include <iostream>

#pragma execution_character_set("utf-8")

MySelectMemu::MySelectMemu(MyCustomPlot *plot, const QPoint &pos, int lineCount)
    : QTreeWidget(plot->parentWidget()), m_plot(plot), m_pos(pos),
      m_lineCount(lineCount), m_subMenu(nullptr)
{
    QMenu menu(this);
    QAction *action = nullptr;
    if (!m_plot->m_pauseDataUpdate) {
        action = new QAction(tr("新增纵线"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this, [=] {
            CreateCrosswiseLine();
        });

        action = new QAction(tr("新增横线"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this, [=] {
            CreateLengthwiseLine();
        });

        action = new QAction(tr("删除所有线"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this, [=] {
            ClearLines();
        });

        if (m_plot->IsMainView()) {
            action = new QAction(tr("生成子图"), this);
            menu.addAction(action);
            connect(action, &QAction::triggered, this, [=] {
                NewSubView();
            });
        }

        if (m_plot->m_plotType == PLOT_TYPE::PLOT_2D_MAIN) {
            action = new QAction(this);
            if (!m_plot->m_multilineMode) {
                action->setText("进入余晖模式");
            } else {
                action->setText("退出余晖模式");
            }
            menu.addAction(action);
            connect(action, &QAction::triggered, this, [=] {
                if (!m_plot->m_multilineMode) {
                    InterMultilineMode();
                } else {
                    ExitMultilineMode();
                }
            });
        }

        if ((m_plot->m_plotType == PLOT_TYPE::PLOT_2D_MAIN ||
             m_plot->m_plotType == PLOT_TYPE::PLOT_2D_HIS_MAIN) &&
             m_plot->CanEnterBaiduMap()) {
            action = new QAction(tr("进入地图模式"), this);
            menu.addAction(action);
            connect(action, &QAction::triggered, this, [=] {
                InterMapMode();
            });
        }

        if (m_plot->IsMainView()) {
            action = new QAction(tr("设置X轴范围"), this);
            menu.addAction(action);
            connect(action, &QAction::triggered, this, [=] {
                SetXRange();
            });
        }

        action = new QAction(tr("设置Y轴范围"), this);
        menu.addAction(action);
        connect(action, &QAction::triggered, this, [=] {
            SetYRange();
        });
    }

    if (m_plot->m_plotType != PLOT_TYPE::PLOT_2D_HIS_MAIN &&
        m_plot->m_plotType != PLOT_TYPE::PLOT_2D_HIS_SUB) {
        action = new QAction(this);
        if (m_plot->m_pauseDataUpdate) {
            action->setText(QString("恢复数据显示更新"));
        } else {
            action->setText(QString("暂停数据显示更新"));
        }
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_plot->m_pauseDataUpdate = !m_plot->m_pauseDataUpdate;
            //std::cout << "数据更新:" << m_plot->m_pauseDataUpdate << " " << m_plot->m_choosePointMode << std::endl;
            if (!m_plot->m_pauseDataUpdate && m_plot->m_choosePointMode) { // 恢复数据更新时，如果在取点模式下直接退出取点模式
                //std::cout << "退出取点，恢复数据更新" << std::endl;
                m_plot->ClearAllChoosedPoints();
                m_plot->m_choosePointMode = false;
                m_plot->ExitChoosePointMode();
            }
        });
    }

    action = new QAction(tr("还原"), this);
    menu.addAction(action);
    connect(action, &QAction::triggered, this, [=] {
        ZoomReset();
    });

    if (!m_plot->m_choosePointMode) {
        action = new QAction(this);
        if (!m_plot->m_dragMode) {
            action->setText(QString("进入拖动模式"));
        } else {
            action->setText(QString("退出拖动模式"));
        }
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_plot->m_dragMode = !m_plot->m_dragMode;
            if (m_plot->m_dragMode) {
                m_plot->setCursor(QCursor(Qt::PointingHandCursor));
                m_plot->axisRect()->setRangeDragAxes(nullptr, m_plot->yAxis2);
                m_plot->axisRect()->setRangeDrag(Qt::Vertical);
            } else {
                m_plot->unsetCursor();
                ZoomReset();
            }
        });
    }

    if (!m_plot->m_dragMode && m_plot->IsMainView() &&
            (m_plot->m_pauseDataUpdate || m_plot->m_plotType == PLOT_TYPE::PLOT_2D_HIS_MAIN ||
             m_plot->m_plotType == PLOT_TYPE::PLOT_2D_HIS_SUB)) {
        action = new QAction(this);
        if (!m_plot->m_choosePointMode) {
            action->setText(QString("进入取点模式"));
        } else {
            action->setText(QString("退出取点模式"));
        }
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_plot->ClearAllChoosedPoints();
            m_plot->m_choosePointMode = !m_plot->m_choosePointMode;
            if (m_plot->m_choosePointMode) {
                m_plot->InterChoosePointMode();
            } else {
                m_plot->ExitChoosePointMode();
            }
        });
    }

    if (m_plot->m_choosePointMode) {
        action = new QAction(this);
        action->setText(QString("清除所有取点"));
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_plot->ClearAllChoosedPoints();
        });
    }

    if (m_plot->m_choosePointMode && m_plot->m_highLightPoint) {
        action = new QAction("删除此取点", this);
        m_plot->ShowMenu();
        menu.addAction(action);
        connect(action, &QAction::triggered, this, [&] {
            if (m_plot->m_highLightPoint) {
                m_plot->m_points[m_plot->m_highLightPoint->lineIndex]->removeOne(m_plot->m_highLightPoint);
                delete m_plot->m_highLightPoint;
                m_plot->m_highLightPoint = nullptr;
                m_plot->UpdateChoosePointsGraph();
            }
        });
    }

    if (m_plot->m_baseLineIndex < 0 && m_plot->IsMainView() && m_plot->CanChooseBaseLine()) {
        m_subMenu = new QMenu(this);
        m_subMenu->setTitle("选定基线:");
        for (int i = 0; i < m_lineCount; ++i) {
            QLabel *text = new QLabel(QString("Line " + QString::number(i+1)), this);
            text->setStyleSheet("color: " + GetColorStyle(i+1));
            QWidgetAction *action = new QWidgetAction(this);
            action->setText(QString::number(i+1));  // 基线的index
            action->setDefaultWidget(text);
            m_subMenu->addAction(action);
            connect(action, &QAction::triggered, this, &MySelectMemu::ChooseBaseLine);
        }

        menu.addMenu(m_subMenu);
    }

    if (m_plot->m_baseLineIndex >= 0 && m_plot->IsMainView()) {
        action = new QAction(this);
        action->setText(QString("删除基线"));
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            m_plot->RemoveBaseLine();
        });
    }

    if (((m_plot->m_highLightLineIndex >= 0) || (!m_plot->m_multilineMode))
            && m_plot->m_baseLineIndex < 0 && m_plot->IsMainView()) {
        action = new QAction(this);
        m_plot->ShowMenu();
        action->setText(QString("设为基线"));
        menu.addAction(action);
        connect(action, &QAction::triggered, this,
            [&](){
            if (m_plot->m_highLightLineIndex >= 0) {
                m_plot->SetBaseLine(m_plot->m_highLightLineIndex + 1);
            } else if (!m_plot->m_multilineMode) {
                m_plot->SetBaseLine(1);
            }
        });
    }

    menu.exec(QCursor::pos());
    menu.clear();
    m_plot->ExitMenu();
}

MySelectMemu::~MySelectMemu()
{
    delete m_subMenu;
}

void MySelectMemu::CreateCrosswiseLine()
{
    m_plot->CreateCrosswiseLine(m_pos);
}

void MySelectMemu::CreateLengthwiseLine()
{
    m_plot->CreateLengthwiseLine(m_pos);
}

void MySelectMemu::ClearLines()
{
    m_plot->ClearLines();
}

void MySelectMemu::NewSubView()
{
    m_plot->NewSubView(m_pos);
}

void MySelectMemu::ZoomReset()
{
    m_plot->ZoomReset();
}

void MySelectMemu::InterMultilineMode()
{
    m_plot->InterMultilineMode();
}

void MySelectMemu::ExitMultilineMode()
{
    m_plot->ExitMultilineMode();
}

void MySelectMemu::InterMapMode()
{
    m_plot->InterMapMode();
}

void MySelectMemu::ChooseBaseLine()
{
    QAction *pChosenAction = dynamic_cast<QAction*>(sender());
    m_plot->SetBaseLine(pChosenAction->text().toInt());
}

void MySelectMemu::SetXRange()
{
    m_plot->SetAxisRangeByUser(QString("设置X轴范围"), 1);
}

void MySelectMemu::SetYRange()
{
    m_plot->SetAxisRangeByUser(QString("设置Y轴范围"), 2);
}
