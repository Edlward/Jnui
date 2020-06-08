#ifndef JNUI_H
#define JNUI_H

#include "jnui_global.h"
#include <QMutex>
#include <QVector>
#include <QString>

class MyGraphicsScene;
class DataReceiver;
struct DBusData;
class QWidget;

class JNUI_EXPORT Jnui
{
public:
    // parent为主窗体的this指针，type为PLOT_TYPE类型，appPath为html文件位置
    Jnui(QWidget *parent, int type, const QString &appPath);
    ~Jnui();
    void UpdateData(struct DBusData *data);          // 更新数据
    void Resize(qreal width, qreal height);          // 主窗口改变大小时的事件

    friend MyGraphicsScene;
private:
    void AddDataReceiver(DataReceiver *receiver);
    void RemoveDataReceiver(DataReceiver *receiver);

    MyGraphicsScene *m_scene;
    QVector<DataReceiver*> m_receivers;
    QString m_appPath;
    QMutex m_mutex;
};

#endif // JNUI_H
