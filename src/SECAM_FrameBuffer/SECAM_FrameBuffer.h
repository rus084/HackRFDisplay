#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "../circular_buffer/circular_buffer.h"


class SECAM_FrameBuffer 
	: public CircularBuffer::IListener
{
public:
	class IListener {
	public:
		virtual void onFrameRequest() = 0;
	};

public:
	SECAM_FrameBuffer(int sampleRate);

	void setListener(std::weak_ptr<IListener> listener);

	void LoadBitMap32Bpp(int Xsize, int Ysize, char* data);
	void LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data);
	void LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data);

	std::shared_ptr<std::vector<int8_t> > onSentCallback() override;
	std::shared_ptr<std::vector<int8_t> > onHalfSentCallback() override;

	std::weak_ptr<IListener> listener_;

	std::shared_ptr<std::vector<int8_t> > buffer_;

	// todo: fix data races with double buffering
	// std::shared_ptr<std::vector<int8_t> > shadowBuffer_;

	static constexpr int rows_ = 625;
	static constexpr int visibleRows_ = 574;

	int collums_;
	int bufferSize_;
	int rowLength_;
	int visibleCollums_;


	int interOffset;
//private:
	void drawCompleteCallback();
	std::shared_ptr<std::vector<int8_t> > makeBuffer();
	int getRowIndex(int row);

	int getFBRowIndex(int row);
};

