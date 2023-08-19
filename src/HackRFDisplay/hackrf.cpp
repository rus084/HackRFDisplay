#include "hackrf.h"
#include <libhackrfpp/hackrf_device.h>
#include <secam_framebuffer/secam_framebuffer.h>
#include <circular_buffer/circular_buffer.h>
#include "SoundProcessor/SoundProcessor.h"

int defaultFreq = 770000000;

static void ScreenShotReqest();

std::shared_ptr<SECAM_FrameBuffer> fb;
std::shared_ptr<HackRFdevice> hackrf;
std::shared_ptr<CircularBuffer> circularBuf;
std::shared_ptr<SoundProcessor> sound;
std::shared_ptr<HackRFdevice::ICallbackListener> hackRfListener;

class HackRfListenerProxy : public HackRFdevice::ICallbackListener {
    int onHackRfCallback(hackrf_transfer* transfer) override {
        circularBuf->onHackRfCallback(transfer);
        sound->HackRFcallback(transfer);
        return 0;
    }
};


void initHackRf()
{
    int sampleRate = 15000000;

    fb = std::make_shared<SECAM_FrameBuffer>();
    hackrf = std::make_shared<HackRFdevice>();
    circularBuf = std::make_shared<CircularBuffer>();
    sound = std::make_shared <SoundProcessor>(sampleRate);
    hackRfListener = std::make_shared<HackRfListenerProxy>();

    hackrf->openAny();
    hackrf->set_freq(defaultFreq);
    hackrf->set_sample_rate(SECAM_FrameBuffer::SAMPLE_RATE);
    hackrf->amp_enable(true);
    hackrf->set_txvga_gain(17);

    hackrf->setCallbackListener(hackRfListener);
    circularBuf->setListener(fb);


    hackrf->startTransmit();
}

void closeHackRf()
{
    hackrf->transferAbort();
    hackrf->close();

    sound.reset();
    hackrf.reset();
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