#ifndef MYSELECTMEMU_H
#define MYSELECTMEMU_H

/**@file xx.h-文件名
* @brief      数据处理模块头文件
* @details    主要包含DAS解调算法、BOTDR解调算法、语音信号处理、共享内存读写等函数接口，用于对光纤线路中采集到的原始采样数据进行处理
* @mainpage   工程概览
* @author     lzy
* @email      jnuerlzy@gmail.com
* @version    V1.0
* @date       2020-12-07
* @copyright  Copyright (c) 2020-2022 广州凌远技术有限公司
************************************************************
* @attention
* 硬件平台：xx   ;
* 软件版本： Qt5.12.3 msvc2017 64bit
* @par 修改日志
* <table>
* <tr><th>Date        <th>Version  <th>Author  <th>Description
* <tr><td>2020/11/30  <td>1.0      <td>lzy     <td>初始版本
* </table>
*
************************************************************
*/

#pragma execution_character_set("utf-8")///<指示char的执行字符集是UTF-8编码。

#include <QTreeWidget>

class MyCustomPlot;
class QCPAxis;

class MySelectMemu : public QTreeWidget
{
public:
    MySelectMemu(MyCustomPlot *plot, const QPoint &pos, int lineCount);
    virtual ~MySelectMemu();
public slots:
    void CreateCrosswiseLine();
    void CreateLengthwiseLine();
    void NewSubView();
    void ClearLines();
//    void ZoomIn();
//    void ZoomOut();
    void ZoomReset();
    void InterMultilineMode();
    void ExitMultilineMode();
    void InterMapMode();
    void ChooseBaseLine();
    void SetXRange();
    void SetYRange();

private:
    MyCustomPlot *m_plot;
    QPoint m_pos;
    int m_lineCount;
    QMenu *m_subMenu;
};

#endif // MYSELECTMEMU_H
