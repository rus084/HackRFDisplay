#pragma once
#include <functional>
#include <memory>
#include <vector>
#include "framebuffer_base.h"


class ProgressiveFB 
	: public FrameBufferBase
{
public:
	static constexpr auto SAMPLE_RATE = 7500000;
	static constexpr auto frameRate = 50;
	static constexpr int rows_ = 312;
	static constexpr int visibleRows_ = 288;

public:
	ProgressiveFB();

	void LoadBitMap32Bpp(int Xsize, int Ysize, char* data) override;
	void LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data) override;
	void LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data);

	std::shared_ptr<std::vector<int8_t> > onSentCallback() override;
	std::shared_ptr<std::vector<int8_t> > onHalfSentCallback() override;

	std::shared_ptr<std::vector<int8_t> > buffer_;

	// todo: fix data races with double buffering
	// std::shared_ptr<std::vector<int8_t> > shadowBuffer_;

	int collums_;
	int visibleCollums_;

	int bufferSize_;

//private:
	std::shared_ptr<std::vector<int8_t> > makeBuffer();
	int getRowIndex(int row);

	int getFBRowIndex(int row);
};

