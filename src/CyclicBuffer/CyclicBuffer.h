#pragma once
#include <functional>
#include "hackrf/HackRFdevice.h"
class CyclicBuffer
{
public:
	CyclicBuffer();
	int HackRFcallback(hackrf_transfer* transfer);
	void setNewBuffer(int len, char* buffer);
	void setCompleteCallback(std::function<void()>* callback);
	void setHalfCompleteCallback(std::function<void()>* callback);
private:
	int bufferPosition = 0;
	char* buffer = nullptr;
	int bufferLen = 0;
	char* nextBuffer = nullptr;
	int nextBufferLen = 0;
	std::function<void()>* CompleteCallback;
	std::function<void()>* HalfCompleteCallback;
	void bufferOverfloved();
	void halfCompleted();
};

