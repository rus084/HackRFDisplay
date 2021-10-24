#include "SoundProcessor.h"
#include <cmath>
#include <array>
#define M_PI           3.14159265358979323846  /* pi */


#include <iostream>


#define IPADDRESS "239.255.77.77"
#define PORT 4010

static std::array<int8_t, 2048> calcSinTable()
{
    std::array<int8_t, 2048> result = std::array<int8_t, 2048>();

    for (int i = 0; i < 2048; i++)
    {
        double phase = (((double)i) / 2048.0) * 2.0 * M_PI;

        result[i] = (int8_t)(20 * std::sin(phase));
    }

    return result;
}

static std::array<int8_t, 2048> sinTable = calcSinTable();

DWORD WINAPI SoundProcessor::SoundProcessorThreadEntry(LPVOID lpParam)
{
    ((SoundProcessor*)lpParam)->threadStart();

    return 0;
}


SoundProcessor::SoundProcessor(int sampleRate)
{
    this->sampleRate = sampleRate;
    freqDeviationCoef = (uint32_t) ((1ULL << 32) * (uint64_t)maxFreqDeviation / (uint64_t)sampleRate / 32768);
    setSoundFreqOffset(6500000);

    Socket = new UDPSocket(IPADDRESS, PORT);

    DWORD threadId;

    threadHandle = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        SoundProcessorThreadEntry,       // thread function name
        this,          // argument to thread function 
        0,                      // use default creation flags 
        &threadId);   // returns the thread identifier 

}


SoundProcessor::~SoundProcessor()
{
    std::cout << "wait for sound thread exit" << std::endl;
    needExit = true;

    if (WaitForSingleObject(
        threadHandle,
        100
    ) == WAIT_TIMEOUT)
    {
        // поток может "зависнуть" на чтении из сокета если звук не передается
        std::cout << "force kill thread" << std::endl;
        TerminateThread(threadHandle, 0);
    }

    

    delete Socket;

    std::cout << "Closing socket." << std::endl;
}

int SoundProcessor::getSoundFreqOffset()
{
    return soundFreq;
}

void SoundProcessor::setSoundFreqOffset(int value)
{
    soundFreq = value;
    defaultPhaseShift = (1ULL << 32) * (uint64_t)soundFreq / (uint64_t)sampleRate;
}

int SoundProcessor::getBufferUsed()
{
    int writePos = writeAudioPos;
    int readPos = readAudioPos;

    if (readPos > writePos)
        readPos -= (sizeof(audioBuf) / sizeof(audioBuf[0]));

    return writePos - readPos;
}

int SoundProcessor::HackRFcallback(hackrf_transfer* transfer)
{
    int bytes_to_read = transfer->valid_length;
    int bufferUsed = getBufferUsed();

    for (int i = 0; i < bytes_to_read; i += 2)
    {
        signalPhase += defaultPhaseShift + (audioBuf[readAudioPos] * freqDeviationCoef);

        readAudioPosFrac++;
        if (readAudioPosFrac > readAudioDivider)
        {
            readAudioPosFrac = 0;
            if (bufferUsed-- > 0)
            {
                readAudioPos++;
                readAudioPos &= 8191;
            }
        }  

        // не нужно проверять переполнение signalPhase, так как оно обрабатывается "как-бы аппаратно" переполнением 32 битной переменной
        int sinPhase = signalPhase >> 21;
        transfer->buffer[i] += (uint8_t)(sinTable[sinPhase]);
        sinPhase -= 512;  // смещаем на 90 градусов
        sinPhase &= 2047; // так как количество элементов таблицы - степень двойки, делать заворот можно просто обнуляя старшие биты
        transfer->buffer[i+1] += (uint8_t)(sinTable[sinPhase]);
    }

    if (bufferUsed < 1900)
        readAudioDivider = 312;

    if (bufferUsed > 2000)
        readAudioDivider = 311;

    return 0;
}


void SoundProcessor::threadStart()
{
    char recvBuf[4096];
    while (!needExit)
    {
        int bytes_read;
        bytes_read = Socket->ReadSocket(recvBuf, sizeof(recvBuf));


        if (bytes_read < 0)
        {
            delete Socket;
            Socket = new UDPSocket(IPADDRESS, PORT);
            continue;
        }

        int writePos = writeAudioPos;
        int readPos = readAudioPos;

        if (readPos > writePos)
            readPos -= (sizeof(audioBuf) / sizeof(audioBuf[0]));

        int bufferUsed = writePos - readPos;

        for (int i = 5; i < bytes_read; i += 4)
        {
            if (bufferUsed++ >= (sizeof(audioBuf) / sizeof(audioBuf[0])))
                return;

            audioBuf[writeAudioPos++] = *((short*)(&recvBuf[i]));

            if (writeAudioPos >= (sizeof(audioBuf) / sizeof(audioBuf[0])))
                writeAudioPos = 0;
        }
    }
}