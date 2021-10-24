#pragma once
#include <functional>
#include "CyclicBuffer/CyclicBuffer.h"


class SECAM_FrameBuffer
{
public:
	SECAM_FrameBuffer(int sample_rate, std::function<void()>* frameRequest);
	CyclicBuffer* FrameDrawer = nullptr;

	void LoadBitMap32Bpp(int Xsize, int Ysize, char* data);
	void LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data);
	void LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data);

	char* buffer;

	int rows;
	int collums;
	int bufferSize;
	int rowLength;
	int visibleCollums;
	int visibleRows;
	int sample_rate;
private:
	void drawCompleteCallback();
	void fillBuffer(char* buffer);
	int getRowIndex(int row);

	int getFBRowIndex(int row);
};

