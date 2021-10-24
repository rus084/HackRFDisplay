#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void initHackRf();
	void closeHackRf();

	void LoadBitMap32Bpp(int Xsize, int Ysize, char* data);

	void HackRfAddFreq();
	void HackRfSubFreq();
	void HackRfAddSndFreq();
	void HackRfSubSndFreq();

#ifdef __cplusplus
}
#endif
