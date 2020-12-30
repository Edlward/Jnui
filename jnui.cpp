#include "jnui.h"
#include "MyGraphicsScene.h"
#include "DataReceiver.h"
#include <iostream>

Jnui::Jnui(QWidget *parent, int type, const QString &appPath)
    : m_appPath(appPath)
{
    // 新建自定义场景
    m_scene = new MyGraphicsScene(parent, this, (PLOT_TYPE)type);
}

Jnui::~Jnui()
{
    delete m_scene;
}

void Jnui::UpdateData(struct DBusData *data)//更新数据
{
    m_mutex.lock();
    if (m_receivers.empty()) //为空则解锁返回
    {
        m_mutex.unlock();
        return;
    }

    for (auto r : m_receivers) {
        DBusData *tmp = new DBusData(*data);
        r->DataIn(tmp);//将新数据放入数据队列
    }
    m_mutex.unlock();
}

void Jnui::AddDataReceiver(DataReceiver *receiver)
{
    m_mutex.lock();                            //互斥锁上锁
    m_receivers.append(receiver);
    receiver->SetAppPath(m_appPath);
    m_mutex.unlock();                          //互斥锁解锁
}

void Jnui::RemoveDataReceiver(DataReceiver *receiver)
{
    m_mutex.lock();                            //互斥锁上锁
    m_receivers.removeOne(receiver);
    m_mutex.unlock();                          //互斥锁解锁
}

void Jnui::Resize(qreal width, qreal height)
{
    m_scene->Resize(width, height);
}
