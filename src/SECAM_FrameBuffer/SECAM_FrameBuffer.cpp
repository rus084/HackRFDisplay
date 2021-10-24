#include "SECAM_FrameBuffer.h"


constexpr auto BLACK_LEVEL = 80;
constexpr auto PITCH_LEVEL = 85;
constexpr auto SYNC_LEVEL = 100;
constexpr auto WHITE_LEVEL = 5;

SECAM_FrameBuffer::SECAM_FrameBuffer(int sample_rate, std::function<void()>* frameRequest)
{
	this->sample_rate = sample_rate;
	FrameDrawer = new CyclicBuffer();
	FrameDrawer->setCompleteCallback(frameRequest);
	FrameDrawer->setHalfCompleteCallback(frameRequest);

	// because we have I and Q part, which is byte
	int buffer_count = sample_rate / 25;
	rows = 625;
	visibleRows = 574;
	collums = buffer_count / rows;
	visibleCollums = collums - ((collums * 120) / 640);
	bufferSize = buffer_count * 2;
	rowLength = bufferSize / rows;
	buffer = new char[bufferSize];
	fillBuffer(buffer);
	FrameDrawer->setNewBuffer(bufferSize, buffer);
}

// https://helpiks.org/5-22006.html
void SECAM_FrameBuffer::fillBuffer(char* buf)
{
	memset(buf, BLACK_LEVEL, bufferSize);
	int RowPitchLen = (rowLength * 120) / 640;
	int rowSyncLen = (rowLength * 47) / 640;
	int rowSyncFromPitchOffset = (rowLength * 15) / 640;
	// https://studfile.net/preview/1666872/page:31/
	// гасящие импульсы
	{ 
		int f1PinchBeg = getRowIndex(rows - 2) + (rowLength / 2);
		int f1OverlapLen = bufferSize - f1PinchBeg;
		memset(&buf[f1PinchBeg], PITCH_LEVEL, f1OverlapLen);
		int f1PitchEnd = getRowIndex(23) + (rowLength / 2);
		memset(&buf[0], PITCH_LEVEL, f1PitchEnd);
	}


	{
		int f2PinchBeg = getRowIndex(rows / 2 - 1);
		int f2PitchEnd = getRowIndex(rows / 2 + 24);
		int f2PitchLen = f2PitchEnd - f2PinchBeg;
		memset(&buf[f2PinchBeg], PITCH_LEVEL, f2PitchLen);
	}

	{
		int frameSyncLen = rowLength * 5 / 2;
		memset(&buf[0], SYNC_LEVEL, frameSyncLen);
		int f2SyncBeg = getRowIndex(rows / 2 + 1) + (rowLength / 2) - (rowLength / 4);
		memset(&buf[f2SyncBeg], SYNC_LEVEL, frameSyncLen);
	}

	for (int i = 0; i < rows; i++)
	{
		int indexBegin = getRowIndex(i);
		int PitchBeg = indexBegin + (rowLength - RowPitchLen);
		memset(&buf[PitchBeg], PITCH_LEVEL, RowPitchLen);
		memset(&buf[PitchBeg + rowSyncFromPitchOffset], SYNC_LEVEL, rowSyncLen);
	}
}

int SECAM_FrameBuffer::getRowIndex(int row)
{
	return row * rowLength;
}


int SECAM_FrameBuffer::getFBRowIndex(int row)
{
	if (row > visibleRows)
		return 0;

	int tempRow = row / 2;
	if (row % 2)
	{
		tempRow += 23;
		return tempRow * rowLength;
	}
	else
	{
		tempRow += rows / 2 + 24;
		return tempRow * rowLength;
	}
}


static char Color32BPPTobyte(unsigned long int* data)
{
	unsigned long int temp = *data;
	unsigned char r = (unsigned char)(temp >> 0) & 0xFF;
	unsigned char g = (unsigned char)(temp >> 8) & 0xFF;
	unsigned char b = (unsigned char)(temp >> 16) & 0xFF;
	// https://ru.wikipedia.org/wiki/YCbCr

	//float y = (0.299 * r + 0.587 * g + 0.114 * b) / 256.0;
	float y = (0.2627f * r + 0.678f * g + 0.0593f * b) / 256.0f;

	float out = y * (WHITE_LEVEL - BLACK_LEVEL) + BLACK_LEVEL;
	char ret = (char)out;
	return ret;
}

void SECAM_FrameBuffer::LoadBitMap32Bpp(int Xsize, int Ysize, int offsetx, int offsety, char* data)
{
	int rowsToDraw = std::min(Ysize, visibleRows);
	int collumsToDraw = std::min(Xsize, visibleCollums);

	for (int i = 0; i < rowsToDraw; i++)
	{
		unsigned long int* inPtr = (unsigned long int*) &data[(Xsize * i) * 4];
		char* outPtr = (char*) &buffer[getFBRowIndex(i + offsety)] + offsetx;

		for (int j = 0; j < collumsToDraw; j++)
		{
			char tmp = Color32BPPTobyte(inPtr++);
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
	int rowsToDraw = std::min(Ysize, visibleRows);
	int collumsToDraw = std::min(Xsize, visibleCollums);

	for (int i = 0; i < rowsToDraw; i++)
	{
		unsigned long int* inPtr = (unsigned long int*) & data[(Xsize * i) * 4];
		char* outPtr = (char*)&buffer[getFBRowIndex(rowsToDraw - 1 - i)];

		for (int j = 0; j < collumsToDraw; j++)
		{
			char tmp = Color32BPPTobyte(inPtr++);
			*(outPtr++) = tmp;
			*(outPtr++) = tmp;
		}
	}
}


