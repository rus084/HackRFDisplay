#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "../circular_buffer/circular_buffer.h"


class FrameBufferBase
	: public CircularBuffer::IListener
{
public:
	class IListener {
	public:
		virtual void onFrameRequest() = 0;
	};

public:
	void setListener(std::weak_ptr<IListener> listener);

	virtual void LoadBitMap32Bpp(int Xsize, int Ysize, char* data) = 0;
	virtual void LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data) = 0;
	virtual void LoadBitmapGray8(int Xsize, int Ysize, char* data) = 0;

protected:
	std::weak_ptr<IListener> listener_;
};

