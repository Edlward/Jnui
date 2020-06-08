#ifndef CUSTOMPLOTINTERFACE_H
#define CUSTOMPLOTINTERFACE_H

#include <QString>
#include <QVector>

class CustomPlotInterface
{
public:
    virtual void ExitBaiduMap() = 0;
    virtual QString GetMeasureDataOfPos(double lon, double lat) = 0;
    virtual void SetAppPath(const QString &appPath) = 0;

protected:
    QVector<double> m_warnings;
    QString m_appPath;
};

#endif // CUSTOMPLOTINTERFACE_H
