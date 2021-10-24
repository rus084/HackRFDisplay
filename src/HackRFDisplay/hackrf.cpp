#include "hackrf.h"
#include "hackrf/HackRFdevice.h"
#include "SECAM_FrameBuffer/SECAM_FrameBuffer.h"
#include "SoundProcessor/SoundProcessor.h"

int defaultFreq = 770000000;

static void ScreenShotReqest();

SECAM_FrameBuffer* fb;
HackRFdevice* hackrf;
std::function<int(hackrf_transfer*)> hackRfCallbackFunc;
SoundProcessor* sound;
std::function<void()> screenshotRequestFunc = std::bind(ScreenShotReqest);

int HackRFcallback(hackrf_transfer* transfer)
{
    fb->FrameDrawer->HackRFcallback(transfer);
    sound->HackRFcallback(transfer);
    return 0;
}

static void ScreenShotReqest()
{
    //PostMessage(hWnd, WM_USER_MAKE_SCREENSHOT, 0, 0);
}

void initHackRf()
{
    int sampleRate = 15000000;

    hackrf = new HackRFdevice();
    hackrf->OpenAny();
    hackrf->set_freq(defaultFreq);
    hackrf->set_sample_rate(sampleRate);
    hackrf->amp_enable(true);
    hackrf->set_txvga_gain(17);


    fb = new SECAM_FrameBuffer(sampleRate, &screenshotRequestFunc);
    sound = new SoundProcessor(sampleRate);
    hackRfCallbackFunc = std::bind(HackRFcallback, std::placeholders::_1);
    hackrf->startTransmit(&hackRfCallbackFunc);
}

void closeHackRf()
{
    hackrf->transferAbort();
    hackrf->Close();
    delete sound;
    delete fb;
}

// чтобы кадр был в границах видимого растра, нужно его смещать
int offsetx = 130;
int offsety = 40;

void LoadBitMap32Bpp(int Xsize, int Ysize, char* data)
{
    fb->LoadBitMap32Bpp(Xsize, Ysize, offsetx, offsety, data);
}

void HackRfAddFreq()
{
    defaultFreq += 100000;
    hackrf->set_freq(defaultFreq);
}

void HackRfSubFreq()
{
    defaultFreq -= 100000;
    hackrf->set_freq(defaultFreq);
}


void HackRfAddSndFreq()
{
    sound->setSoundFreqOffset(sound->getSoundFreqOffset() + 10000);
}

void HackRfSubSndFreq()
{
    sound->setSoundFreqOffset(sound->getSoundFreqOffset() - 10000);
}