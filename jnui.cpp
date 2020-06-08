#include "jnui.h"
#include "MyGraphicsScene.h"
#include "DataReceiver.h"
#include <iostream>

Jnui::Jnui(QWidget *parent, int type, const QString &appPath)
    : m_appPath(appPath)
{
    m_scene = new MyGraphicsScene(parent, this, (PLOT_TYPE)type);
}

Jnui::~Jnui()
{
    delete m_scene;
}

void Jnui::UpdateData(struct DBusData *data)
{
    m_mutex.lock();
    if (m_receivers.empty()) {
        m_mutex.unlock();
        return;
    }

    for (auto r : m_receivers) {
        DBusData *tmp = new DBusData(*data);
        r->DataIn(tmp);
    }
    m_mutex.unlock();
}

void Jnui::AddDataReceiver(DataReceiver *receiver)
{
    m_mutex.lock();
    m_receivers.append(receiver);
    receiver->SetAppPath(m_appPath);
    m_mutex.unlock();
}

void Jnui::RemoveDataReceiver(DataReceiver *receiver)
{
    m_mutex.lock();
    m_receivers.removeOne(receiver);
    m_mutex.unlock();
}

void Jnui::Resize(qreal width, qreal height)
{
    m_scene->Resize(width, height);
}
