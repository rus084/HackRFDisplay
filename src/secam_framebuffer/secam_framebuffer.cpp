#include "secam_framebuffer.h"
#include <memory>
#include <vector>

#include <string.h>

constexpr auto BLACK_LEVEL = 80;
constexpr auto PITCH_LEVEL = 85;
constexpr auto SYNC_LEVEL = 100;
constexpr auto WHITE_LEVEL = 5;

SECAM_FrameBuffer::SECAM_FrameBuffer()
{
	int buffer_count = SAMPLE_RATE / 25;
	// because we have I and Q part
	bufferSize_ = buffer_count * 2;

	collums_ = buffer_count / rows_;
	visibleCollums_ = collums_ - ((collums_ * 120) / 640);
	rowLength_ = bufferSize_ / rows_;

	interOffset = (rowLength_ / 2);

	buffer_ = makeBuffer();
	//shadowBuffer_ = makeBuffer();
}

std::shared_ptr<std::vector<int8_t> > SECAM_FrameBuffer::onSentCallback() {
	if (auto listener = listener_.lock()) {
		listener->onFrameRequest();
	}

	return buffer_;
}

std::shared_ptr<std::vector<int8_t> > SECAM_FrameBuffer::onHalfSentCallback() {
	if (auto listener = listener_.lock()) {
		listener->onFrameRequest();
	}

	return nullptr;
}

// https://helpiks.org/5-22006.html
std::shared_ptr<std::vector<int8_t> > SECAM_FrameBuffer::makeBuffer()
{
	auto buffer = std::make_shared<std::vector<int8_t> >(bufferSize_, BLACK_LEVEL);

	auto buf = buffer->data();

	int RowPitchLen = (rowLength_ * 120) / 640;
	int rowSyncLen = (rowLength_ * 47) / 640;
	int rowSyncFromPitchOffset = (rowLength_ * 15) / 640;

	// https://studfile.net/preview/1666872/page:31/
	{ 
		int f1PinchBeg = getRowIndex(rows_ - 2) + (rowLength_ / 2);
		int f1OverlapLen = bufferSize_ - f1PinchBeg;
		memset(&buf[f1PinchBeg], PITCH_LEVEL, f1OverlapLen);
		int f1PitchEnd = getRowIndex(23) + (rowLength_ / 2);
		memset(&buf[0], PITCH_LEVEL, f1PitchEnd);
	}

	{
		int f2PinchBeg = getRowIndex(rows_ / 2 - 1);
		int f2PitchEnd = getRowIndex(rows_ / 2 + 24);
		int f2PitchLen = f2PitchEnd - f2PinchBeg;
		memset(&buf[f2PinchBeg], PITCH_LEVEL, f2PitchLen);
	}

	{
		int frameSyncLen = rowLength_ * 5 / 2;
		memset(&buf[0], SYNC_LEVEL, frameSyncLen);
		int f2SyncBeg = getRowIndex(rows_ / 2 + 1) + interOffset;
		memset(&buf[f2SyncBeg], SYNC_LEVEL, frameSyncLen);
	}

	for (int i = 0; i < rows_; i++)
	{
		int indexBegin = getRowIndex(i);
		int PitchBeg = indexBegin + (rowLength_ - RowPitchLen);
		memset(&buf[PitchBeg], PITCH_LEVEL, RowPitchLen);
		memset(&buf[PitchBeg + rowSyncFromPitchOffset], SYNC_LEVEL, rowSyncLen);
	}

	return buffer;
}

int SECAM_FrameBuffer::getRowIndex(int row)
{
	return row * rowLength_;
}


int SECAM_FrameBuffer::getFBRowIndex(int row)
{
	if (row > visibleRows_)
		return 0;

	int tempRow = row / 2;
	if ((row % 2) == 0)
	{
		tempRow += 23;
		return tempRow * rowLength_;
	}
	else
	{
		tempRow += rows_ / 2 + 24;
		return tempRow * rowLength_;
	}
}


static int8_t Color32BPPTobyte(uint32_t* data)
{
	unsigned long int temp = *data;
	unsigned char r = (unsigned char)(temp >> 0) & 0xFF;
	unsigned char g = (unsigned char)(temp >> 8) & 0xFF;
	unsigned char b = (unsigned char)(temp >> 16) & 0xFF;
	// https://ru.wikipedia.org/wiki/YCbCr

	//float y = (0.299 * r + 0.587 * g + 0.114 * b) / 256.0;
	float y = (0.2627f * r + 0.678f * g + 0.0593f * b) / 256.0f;

	char out = (char)(y * (WHITE_LEVEL - BLACK_LEVEL) + BLACK_LEVEL);

	return out;
}

void SECAM_FrameBuffer::LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data)
{
	const auto bufGuard = buffer_;

	int rowsToDraw = std::min(Ysize, visibleRows_);
	int collumsToDraw = std::min(Xsize, visibleCollums_);

	auto buffer = bufGuard->data();

	for (int i = 0; i < rowsToDraw; i++)
	{
		uint32_t* inPtr = (uint32_t*) &data[(Xsize * i) * 4];
		int8_t* outPtr = &buffer[getFBRowIndex(i + offsety)] + offsetx;

		for (int j = 0; j < collumsToDraw; j++)
		{
			int8_t tmp = Color32BPPTobyte(inPtr++);
			*(outPtr++) = tmp;
			*(outPtr++) = tmp;
		}
	}
}

void SECAM_FrameBuffer::LoadBitMap32Bpp(int Xsize, int Ysize, char* data) {
	LoadBitMap32Bpp(Xsize, Ysize, 0, 0, data);
}


void SECAM_FrameBuffer::LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data)
{
	const auto bufGuard = buffer_;

	int rowsToDraw = std::min(Ysize, visibleRows_);
	int collumsToDraw = std::min(Xsize, visibleCollums_);

	auto buffer = bufGuard->data();

	for (int i = 0; i < rowsToDraw; i++)
	{
		uint32_t* inPtr = (uint32_t*) & data[(Xsize * i) * 4];
		int8_t* outPtr = &buffer[getFBRowIndex(rowsToDraw - 1 - i)];

		for (int j = 0; j < collumsToDraw; j++)
		{
			int8_t tmp = Color32BPPTobyte(inPtr++);
			*(outPtr++) = tmp;
			*(outPtr++) = tmp;
		}
	}
}

void SECAM_FrameBuffer::LoadBitmapGray8(int Xsize, int Ysize, char* data) {
	const auto bufGuard = buffer_;

	int rowsToDraw = std::min(Ysize, visibleRows_);
	int collumsToDraw = std::min(Xsize, visibleCollums_);

	auto buffer = bufGuard->data();

	for (int i = 0; i < rowsToDraw; i++)
	{
		uint8_t* inPtr = (uint8_t*) & data[(Xsize * i)];
		int8_t* outPtr = &buffer[getFBRowIndex(rowsToDraw - 1 - i)];

		for (int j = 0; j < collumsToDraw; j++)
		{
			uint8_t pix = *inPtr;
			inPtr++;

			int8_t tmp = (pix * (WHITE_LEVEL - BLACK_LEVEL) / 255) + BLACK_LEVEL;

			*(outPtr++) = tmp;
			*(outPtr++) = tmp;
		}
	}
}
