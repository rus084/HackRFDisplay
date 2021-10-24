#include "CyclicBuffer.h"
#include <Windows.h>

CyclicBuffer::CyclicBuffer() : CompleteCallback(nullptr), HalfCompleteCallback(nullptr)
{
}

void CyclicBuffer::setCompleteCallback(std::function<void()>* callback)
{
	CompleteCallback = callback;
}

void CyclicBuffer::setHalfCompleteCallback(std::function<void()>* callback)
{
	HalfCompleteCallback = callback;
}

void CyclicBuffer::bufferOverfloved()
{
	bufferPosition = 0;
	if (nextBufferLen > 0)
	{
		buffer = nextBuffer;
		nextBuffer = nullptr;
		bufferLen = nextBufferLen;
		nextBufferLen = 0;
	}
	if (CompleteCallback)
		(*CompleteCallback)();
}

void CyclicBuffer::halfCompleted()
{
	if (HalfCompleteCallback)
		(*HalfCompleteCallback)();
}

int CyclicBuffer::HackRFcallback(hackrf_transfer* transfer)
{
	size_t bytes_to_read;

	bytes_to_read = transfer->valid_length;

	if (buffer == nullptr)
	{
		memset(transfer->buffer, 0, bytes_to_read);
		bufferOverfloved();
	}
	else
	{
		int position = 0;
		while (bytes_to_read > 0)
		{
			int remainBuf = bufferLen - bufferPosition;
			int toRead = (int)min(bytes_to_read, remainBuf);
			memcpy(&transfer->buffer[position], &buffer[bufferPosition], toRead);
			bool firstHalf = (bufferPosition < (bufferLen >> 1));
			bytes_to_read -= toRead;
			position += toRead;
			bufferPosition += toRead;

			if (firstHalf && (bufferPosition >= (bufferLen >> 1)))
				halfCompleted();

			if (bufferPosition >= bufferLen)
				bufferOverfloved();
		}
	}
	return 0;
}

void CyclicBuffer::setNewBuffer(int len, char* buf)
{
	nextBuffer = buf;
	nextBufferLen = len;
}