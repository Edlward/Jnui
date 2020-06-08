#ifndef DATARECEIVER_H
#define DATARECEIVER_H

#pragma execution_character_set("utf-8")

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QMetaType>
#include "Common.h"
#include <QQueue>

class MyGraphicsItem;
//class DBusClient;
struct DBusData;
class ColorMap;
// 数据处理接收和处理线程类
class DataReceiver : public QThread
{
Q_OBJECT
public:
    DataReceiver();
    virtual ~DataReceiver();
    void DataIn(DBusData *data);
    void SetDataAccepter(MyGraphicsItem *item);
    void Stop();
    void SetDataType(PLOT_TYPE type) {
        m_type = type;
    }

    void SetAppPath(const QString &appPath)
    {
        m_path = appPath;
    }

signals:
    void DataReady2();

protected:
    void run() override;

private:
    void GetDataFromDbus(DBusData *data);
    void GetHisDatas(DBusData *data);
    void ProcessMeasure(DBusData *m_data);
    void Process3DMeasure(DBusData *m_data);
    void ProcessHis(DBusData *m_data);
    void Process3DHis(DBusData *m_data);

    QMutex m_mutex;
    QWaitCondition m_cond;
    bool m_dataIn;
    //DBusClient *m_dbusClient;
    MyGraphicsItem *m_item;
    bool m_setXY;
    bool m_stop;
    PLOT_TYPE m_type;
    ColorMap *m_colorMap;
    bool m_seted;
    QQueue<DBusData*> m_datas;
    QString m_path;
    int m_dataCount;
    int m_curDataCount;
};

#endif // DATARECEIVER_H
