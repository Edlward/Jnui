#ifndef PCMPLAYER_H
#define PCMPLAYER_H

#pragma execution_character_set("utf-8")

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
