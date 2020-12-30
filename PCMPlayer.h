#ifndef PCMPLAYER_H
#define PCMPLAYER_H

/**@file MySelectMemu.h-文件名
* @brief      脉码调制
* @details
* @mainpage   工程概览
* @author     lzy
* @email      jnuerlzy@gmail.com
* @version    V1.0
* @date       2020-12-22
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

#include <QThread>
#include "Common.h"
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <Windows.h>

class PCMPlayer : public QThread
{
public:
    PCMPlayer(unsigned long nSamplesPerSec, unsigned short wBitsPerSample);
    virtual ~PCMPlayer();

    void Play(char *buffer, int size);
    void Stop();

    static void WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dw1, DWORD_PTR dw2);

protected:
    void run() override;

private:
    struct BufferStripe;
    BufferStripe *GetEmptyBufferStripe();
    BufferStripe *GetFullBufferStripe();
    void PushFullQ(BufferStripe *stripe);
    void PushEmptyQ(BufferStripe *stripe);
    bool DoPlay(BufferStripe *stripe);
    void WaitWaveOut();
    void NotifyWaveOut();

private:
    struct BufferStripe
    {
        BufferStripe()
            : m_curSize(0)
        {}

        void Reset()
        {
            m_curSize = 0;
        }

        char m_buffer[PCM_PLAYER_BUFFER_STRIPE_SIZE];
        int m_curSize;
    };

    QQueue<BufferStripe*> m_emptyQ;
    QQueue<BufferStripe*> m_fullQ;
    QMutex m_emptyMutex;
    QWaitCondition m_emptyCond;
    QMutex m_fullMutex;
    QWaitCondition m_fullCond;
    bool m_stop;
    HWAVEOUT m_waveOut;
    WAVEFORMATEX m_waveFormat;
    WAVEHDR m_waveHeader;
    BufferStripe *m_curPlayStripe;
    QMutex m_playMutex;
    QWaitCondition m_playCond;
    bool m_ready;
};

#endif // PCMPLAYER_H
