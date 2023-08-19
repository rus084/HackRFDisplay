#include "progressive_fb.h"
#include <memory>
#include <vector>

constexpr auto BLACK_LEVEL = 80;
constexpr auto PITCH_LEVEL = 85;
constexpr auto SYNC_LEVEL = 100;
constexpr auto WHITE_LEVEL = 5;

ProgressiveFB::ProgressiveFB()
{
	int buffer_count = SAMPLE_RATE / frameRate;
	int collums = buffer_count / rows_;

	// fix rounding 
	buffer_count = collums * rows_;

	collums_ = buffer_count / rows_;

	// because we have I and Q part
	bufferSize_ = buffer_count * 2;

	
	visibleCollums_ = collums_ - ((collums_ * 120) / 640);

	buffer_ = makeBuffer();
	//shadowBuffer_ = makeBuffer();
}

std::shared_ptr<std::vector<int8_t> > ProgressiveFB::onSentCallback() {
	if (auto listener = listener_.lock()) {
		listener->onFrameRequest();
	}

	return buffer_;
}

std::shared_ptr<std::vector<int8_t> > ProgressiveFB::onHalfSentCallback() {
	//if (auto listener = listener_.lock()) {
	//	listener->onFrameRequest();
	//}

	return nullptr;
}

// https://helpiks.org/5-22006.html
std::shared_ptr<std::vector<int8_t> > ProgressiveFB::makeBuffer()
{
	auto buffer = std::make_shared<std::vector<int8_t> >(bufferSize_, BLACK_LEVEL);
	auto buf = buffer->data();

	// https://studfile.net/preview/1666872/page:31/
	{ 
		int f1PinchBeg = getRowIndex(rows_ - 2);
		int f1OverlapLen = bufferSize_ - f1PinchBeg;
		int f1PitchEnd = getRowIndex(23);

		memset(&buf[f1PinchBeg], PITCH_LEVEL, f1OverlapLen);
		memset(&buf[0], PITCH_LEVEL, f1PitchEnd);
	}


	int rowLength = collums_ * 2;
	{
		int frameSyncLen = rowLength * 2;
		memset(&buf[0], SYNC_LEVEL, frameSyncLen);
	}


	int RowPitchLen = (rowLength * 120) / 640;
	int rowSyncLen = (rowLength * 47) / 640;
	int rowSyncFromPitchOffset = (rowLength * 15) / 640;

	for (int i = 0; i < rows_; i++)
	{
		int indexBegin = getRowIndex(i);
		int PitchBeg = indexBegin + (rowLength - RowPitchLen);
		memset(&buf[PitchBeg], PITCH_LEVEL, RowPitchLen);
		memset(&buf[PitchBeg + rowSyncFromPitchOffset], SYNC_LEVEL, rowSyncLen);
	}

	return buffer;
}

int ProgressiveFB::getRowIndex(int row)
{
	int rowLength = collums_ * 2;

	return row * rowLength;
}


int ProgressiveFB::getFBRowIndex(int row)
{
	if (row > visibleRows_)
		return 0;

	int tempRow = row;

	tempRow += 23;
	int rowLength = collums_ * 2;

	return tempRow * rowLength;
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

void ProgressiveFB::LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data)
{
	const auto bufGuard = buffer_;

	int rowsToDraw = std::min(Ysize, visibleRows_);
	int collumsToDraw = std::min(Xsize, visibleCollums_);

	auto buffer = buffer_->data();

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

void ProgressiveFB::LoadBitMap32Bpp(int Xsize, int Ysize, char* data) {
	LoadBitMap32Bpp(Xsize, Ysize, 0, 0, data);
}

void ProgressiveFB::LoadBitMap32BppMirrorV(int Xsize, int Ysize, char* data)
{
	const auto bufGuard = buffer_;

	int rowsToDraw = std::min(Ysize, visibleRows_);
	int collumsToDraw = std::min(Xsize, visibleCollums_);

	auto buffer = buffer_->data();

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


