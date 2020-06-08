#pragma execution_character_set("utf-8")

#include "BaiduMap.h"
#include <QtWebChannel/QWebChannel>
#include <QWebEngineView>
#include "Common.h"
#include "qcustomplot.h"
#include <iostream>

JsBridge::JsBridge(BaiduMap *baiduMap, const QString &centerName)
    : m_baiduMap(baiduMap), m_baiduMapCenter(centerName)
{

}

JsBridge::~JsBridge()
{

}

QVariantList JsBridge::GetData()
{
    return m_data;
}

QString JsBridge::GetCenterName()
{
    return m_baiduMapCenter;
}

QVariantList JsBridge::GetWarnings()
{
    //std::cout<< "get warnings:" << m_warnings.count() << std::endl;
    return m_warnings;
}

void JsBridge::SwitchDataView()
{
    m_baiduMap->SwitchDataView();
}

void JsBridge::SetData(const QVector<double> &lons, const QVector<double> &lats)
{
    if (lons.count() != lats.count() || lons.count() == 0 || m_data.count() > 0) {
        return;
    }

    for (int i = 0; i < lons.count(); ++i) {
        QString tmp = QString::number(lons[i], 'f', 7);
        tmp += ",";
        tmp += QString::number(lats[i], 'f', 7);
        m_data.append(tmp);
        m_lons.append(lons[i]);
        m_lats.append(lats[i]);
        // std::cout << tmp.toStdString() << std::endl;
    }

    emit sigUpdateData();
}

void JsBridge::UpdateWarnings(const QVector<double> &warnings)
{
    if (!m_warnings.empty()) {
        m_warnings.clear();
    }

    for (int i = 0; i < warnings.count(); ++i) {
        if (warnings[i] > 0) {
            QString tmp = QString::number(m_lons[i], 'f', 7);
            tmp += ",";
            tmp += QString::number(m_lats[i], 'f', 7);
            //m_warnings.append(QString("%1,%2").arg(m_lons[i]).arg(m_lats[i]));
            m_warnings.append(tmp);
            //std::cout << tmp.toStdString() <<std::endl;
        }
    }

    emit sigUpdateData();
}

QString JsBridge::GetMeasureDataOfPos(QString lon, QString lat)
{
    std::cout << "GetMeasureDataOfPos lon:" << lon.toStdString() << ",lat=" << lat.toStdString() << std::endl;
    return m_baiduMap->GetMeasureDataOfPos(lon.toDouble(), lat.toDouble());
}

BaiduMap::BaiduMap(QWidget *parent, CustomPlotInterface *plot, const QString &centerName, const QString &appPath)
    : QWidget(parent), m_plot(plot), m_dataUpdated(false)
{
    m_bridge = new JsBridge(this, centerName);
    if (!m_bridge) {
        return;
    }

    m_channel = new QWebChannel(this);
    if (!m_channel) {
        return;
    }

    m_channel->registerObject("bridge", (QObject*)m_bridge);
    m_webView = new QWebEngineView(parent);
    if (!m_webView) {
        return;
    }

    m_webView->page()->setWebChannel(m_channel);
    //QString url = "E:/test/build-test_chart-Desktop_Qt_5_14_1_MSVC2017_64bit-Debug/debug/index.html";
    QString url = appPath + "/index.html";
    std::cout << url.toStdString() << std::endl;
    m_webView->page()->load(QUrl(url));
    m_webView->move(GRAPHICS_SCENE_WIDTH, 0);
    m_webView->resize(parent->width(), parent->height());
    m_webView->show();
}

BaiduMap::~BaiduMap()
{
    delete m_webView;
    delete m_channel;
    delete m_bridge;
}

void BaiduMap::resize(qreal wigth, qreal height)
{
    m_webView->resize(wigth, height);
}

void BaiduMap::UpdateData(const QVector<double> &lons, const QVector<double> &lats)
{
    if (!m_dataUpdated && lons.count() > 0) {
        m_bridge->SetData(lons, lats);
        m_dataUpdated = true;
    }
}

void BaiduMap::UpdateWarnings(const QVector<double> &warnings)
{
    m_bridge->UpdateWarnings(warnings);
}

void BaiduMap::SwitchDataView()
{
    QWidget::hide();
    m_webView->hide();
    m_plot->ExitBaiduMap();
}

void BaiduMap::show()
{
    QWidget::show();
    m_webView->show();
}

void BaiduMap::hide()
{
    QWidget::hide();
    m_webView->hide();
}

QString BaiduMap::GetMeasureDataOfPos(double lon, double lat)
{
    return m_plot->GetMeasureDataOfPos(lon, lat);
}
