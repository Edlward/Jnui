#pragma execution_character_set("utf-8")

#include "PCMPlayer.h"
#include <iostream>

using namespace std;

#pragma comment (lib, "winmm.lib")

PCMPlayer::PCMPlayer(unsigned long nSamplesPerSec, unsigned short wBitsPerSample)
    : m_stop(false)
{
    for (int i = 0; i < PCM_PLAYER_BUFFER_COUNT; ++i) {
        BufferStripe *stripe = new BufferStripe();
        if (!stripe) {
            return;
        }

        m_emptyQ.push_back(stripe);
    }

    m_waveFormat.wFormatTag = WAVE_FORMAT_PCM; // 设置波形声音的格式
    m_waveFormat.nChannels = 1; // 设置单通道
    m_waveFormat.nSamplesPerSec = nSamplesPerSec;   // 采样率
    m_waveFormat.nAvgBytesPerSec = nSamplesPerSec/2;
    m_waveFormat.nBlockAlign = 2;
    m_waveFormat.wBitsPerSample = wBitsPerSample;
    m_waveFormat.cbSize = 0; // 额外信息的大小

    MMRESULT ret = waveOutOpen(&m_waveOut, WAVE_MAPPER, &m_waveFormat,
                               reinterpret_cast<DWORD_PTR>(PCMPlayer::WaveCallback),
                               reinterpret_cast<DWORD_PTR>(this),
                               CALLBACK_FUNCTION);
    if (ret != MMSYSERR_NOERROR) {
        cout << "open wave out failed,err=" << ret << endl;
        m_ready = false;
    }

    m_ready = true;
}


void PCMPlayer::WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dw1, DWORD_PTR dw2)
{
    Q_UNUSED(hWave)
    Q_UNUSED(dw1)
    Q_UNUSED(dw2)

    PCMPlayer *player = reinterpret_cast<PCMPlayer*>(dwInstance);
    if (!player) {
        cout << "player is null" << endl;
        return;
    }

    switch (uMsg) {
    case WOM_DONE:// 上次缓存播放完成,触发该事件
    {
        cout << "play over stripe:" << player->m_curPlayStripe << endl;
        // 播放完的数据块push到空闲队列
        player->m_curPlayStripe->Reset();
        player->PushEmptyQ(player->m_curPlayStripe);
        //player->DoPlay(player->GetFullBufferStripe());
        player->NotifyWaveOut();
        break;
    }
    case WOM_CLOSE:
        waveOutUnprepareHeader(player->m_waveOut, &player->m_waveHeader, sizeof(WAVEHDR));
        player->NotifyWaveOut();
        break;
    case WOM_OPEN:
        cout << "wave out opened!" << endl;
        break;
    }
}

PCMPlayer::~PCMPlayer()
{
    waveOutClose(m_waveOut);

    BufferStripe *stripe = NULL;
    while(!m_emptyQ.empty()) {
        stripe = m_emptyQ.first();
        m_emptyQ.pop_front();
        delete stripe;
    }

    while(!m_fullQ.empty()) {
        stripe = m_fullQ.first();
        m_fullQ.pop_front();
        delete stripe;
    }
}

void PCMPlayer::Play(char *buffer, int size)
{
    if (!m_ready) {
        return;
    }

    int cur_size = 0;
    while (size > cur_size) {
        BufferStripe *stripe = GetEmptyBufferStripe();
        cout << "play stripe:" << stripe << endl;
        if (size - cur_size >= PCM_PLAYER_BUFFER_STRIPE_SIZE) {
            memcpy(stripe->m_buffer, buffer, PCM_PLAYER_BUFFER_STRIPE_SIZE);
            stripe->m_curSize = PCM_PLAYER_BUFFER_STRIPE_SIZE;
            cur_size += PCM_PLAYER_BUFFER_STRIPE_SIZE;
            buffer += PCM_PLAYER_BUFFER_STRIPE_SIZE;
            PushFullQ(stripe);
        } else {
            memcpy(stripe->m_buffer, buffer, static_cast<size_t>(size - cur_size));
            stripe->m_curSize = (size - cur_size);
            cur_size += (size - cur_size);
            buffer += (size - cur_size);
            PushFullQ(stripe);
            break;
        }
    }
}

PCMPlayer::BufferStripe* PCMPlayer::GetEmptyBufferStripe()
{
    BufferStripe* stripe = NULL;
    m_emptyMutex.lock();
    while (m_emptyQ.empty() && !m_stop) {
        m_emptyCond.wait(&m_emptyMutex);
    }
    if (!m_stop) {
        stripe = m_emptyQ.first();
        m_emptyQ.pop_front();

    }
    m_emptyMutex.unlock();

    return stripe;
}

PCMPlayer::BufferStripe* PCMPlayer::GetFullBufferStripe()
{
    BufferStripe* stripe = NULL;
    m_fullMutex.lock();
    while (m_fullQ.empty() && !m_stop) {
        cout << "start wait full" << endl;
        m_fullCond.wait(&m_fullMutex);
        cout << "end wait full" << endl;
    }
    if (!m_stop) {
        stripe = m_fullQ.first();
        m_fullQ.pop_front();
    }
    m_fullMutex.unlock();

    return stripe;
}

void PCMPlayer::PushFullQ(BufferStripe *stripe)
{
    m_fullMutex.lock();
    m_fullQ.push_back(stripe);
    m_fullCond.wakeOne();
    m_fullMutex.unlock();
    cout << "full stripe count:" << m_fullQ.count() << endl;
}

void PCMPlayer::PushEmptyQ(BufferStripe *stripe)
{
    m_emptyMutex.lock();
    m_emptyQ.push_back(stripe);
    m_emptyCond.wakeOne();
    m_emptyMutex.unlock();
    cout << "empty stripe count:" << m_emptyQ.count() << endl;
}

void PCMPlayer::run()
{
    while(!m_stop) {
        bool ret = DoPlay(GetFullBufferStripe());
        if (ret) {
            cout << "start wait out" << endl;
            WaitWaveOut();
            cout << "end wait out" << endl;
        }
    }
    m_fullMutex.unlock();
}

void PCMPlayer::Stop()
{
    m_fullMutex.lock();
    m_stop = true;
    m_fullMutex.unlock();
}

bool PCMPlayer::DoPlay(BufferStripe *stripe)
{
    if (!stripe) {
        return false;
    }

    cout << "do play buffer len=" << stripe->m_curSize << " strip:" << stripe << endl;
    m_curPlayStripe = stripe;
    ZeroMemory(&m_waveHeader, sizeof(WAVEHDR));
    m_waveHeader.dwBufferLength = static_cast<DWORD>(stripe->m_curSize);
    m_waveHeader.lpData = stripe->m_buffer;
    waveOutPrepareHeader(m_waveOut, &m_waveHeader, sizeof(WAVEHDR));
    MMRESULT ret = waveOutWrite(m_waveOut, &m_waveHeader, sizeof(WAVEHDR));
    if (ret == MMSYSERR_NOERROR) {
        return true;
    } else {
        cout << "play buffer error! err=" << ret << endl;
    }

    return false;
}

void PCMPlayer::WaitWaveOut()
{
    m_playMutex.lock();
    m_playCond.wait(&m_playMutex);
    m_playMutex.unlock();
}

void PCMPlayer::NotifyWaveOut()
{
    m_playMutex.lock();
    m_playCond.wakeOne();
    m_playMutex.unlock();
}
