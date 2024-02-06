#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "framebuffer_base.h"


class SECAM_FrameBuffer 
	: public FrameBufferBase
{
public:
	static constexpr auto SAMPLE_RATE = 15000000;

public:
	SECAM_FrameBuffer();

	void LoadBitMap32Bpp(int Xsize, int Ysize, char* data) override;
	void LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data) override;
	virtual void LoadBitmapGray8(int Xsize, int Ysize, char* data) override;
	void LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data);

	std::shared_ptr<std::vector<int8_t> > onSentCallback() override;
	std::shared_ptr<std::vector<int8_t> > onHalfSentCallback() override;

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

