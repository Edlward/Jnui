#ifndef BAIDUMAP_H
#define BAIDUMAP_H

#include <QObject>
#include <QWidget>
#include <QVariantList>
#include "CustomPlotInterface.h"

#pragma execution_character_set("utf-8")

class QWebEngineView;
class QWebChannel;
class QCustomPlot;
class BaiduMap;

struct LonLat
{
    double lon;
    double lat;
};

class JsBridge : public QObject
{
Q_OBJECT
public:
    JsBridge(BaiduMap *baiduMap, const QString &centerName);
    ~JsBridge();

    void SetData(const QVector<double> &lons, const QVector<double> &lats);
    void UpdateWarnings(const QVector<double> &warnings);

signals:
    void sigUpdateData();
public slots:
    QVariantList GetData();
    QVariantList GetWarnings();
    void SwitchDataView();
    QString GetCenterName();
    QString GetMeasureDataOfPos(QString lon, QString lat);

private:
    QVariantList m_data;
    QVariantList m_warnings;
    BaiduMap *m_baiduMap;
    QVector<double> m_lons;
    QVector<double> m_lats;
    QString m_baiduMapCenter;
};

class BaiduMap : public QWidget
{
public:
    BaiduMap(QWidget *parent, CustomPlotInterface *plot, const QString &centerName, const QString &appPath);
    ~BaiduMap();
    void resize(qreal wigth, qreal height);
    void UpdateData(const QVector<double> &lons, const QVector<double> &lats);
    void UpdateWarnings(const QVector<double> &warnings);
    void SwitchDataView();
    void show();
    void hide();
    QString GetMeasureDataOfPos(double lon, double lat);

private:
    JsBridge *m_bridge;
    QWebChannel *m_channel;
    QWebEngineView *m_webView;
    CustomPlotInterface *m_plot;
    bool m_dataUpdated;
};

#endif // BAIDUMAP_H
