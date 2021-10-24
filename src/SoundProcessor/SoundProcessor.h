#pragma once
#include <stdint.h>
#include "hackrf/HackRFdevice.h"
#include "UDPSocket.h"

class SoundProcessor
{
public:
	int HackRFcallback(hackrf_transfer* transfer);
	SoundProcessor(int sampleRate);
	~SoundProcessor();

	int getSoundFreqOffset();
	void setSoundFreqOffset(int value);

private:
	const int maxFreqDeviation = 50000;

	int sampleRate;
	int soundFreq;

	UDPSocket* Socket;
	HANDLE threadHandle;
	bool needExit = false;

	uint32_t signalPhase = 0; // format is fixed point, 11 bits int part + 21 bits frac 
	uint32_t defaultPhaseShift; // на сколько смещать фазу каждый семпл при нулевом уровне аудиосигнала
	uint32_t freqDeviationCoef;
	
	short audioBuf[8192];
	int readAudioPos = 0;
	int writeAudioPos = 0;

	int readAudioPosFrac = 0;
	int readAudioDivider = 312;

	int logCount = 0;

	static DWORD WINAPI SoundProcessorThreadEntry(LPVOID lpParam);
	void threadStart();
	int getBufferUsed();
};
