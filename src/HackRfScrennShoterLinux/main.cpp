#include <functional>
#include <libhackrfpp/hackrf_device.h>
#include <secam_framebuffer/secam_framebuffer.h>
#include <secam_framebuffer/progressive_fb.h>
#include <circular_buffer/circular_buffer.h>

#include <memory>
#include <unistd.h>
#include <signal.h>

class FbListener : public FrameBufferBase::IListener {
public:
    void onFrameRequest() override {

    }
};

void CaptureImage() {
//    frameBuffer->LoadBitMap32BppMirrorV(bmpScreen.bmWidth, bmpScreen.bmHeight, lpbitmap);
}

std::shared_ptr<FrameBufferBase> fb;
std::shared_ptr<HackRFdevice> hackrf;
std::shared_ptr<CircularBuffer> circularBuf;
std::shared_ptr<FbListener> fbListener;

template <typename T>
void setFB() {
    hackrf->set_sample_rate(T::SAMPLE_RATE);

    fb = std::make_shared<T>();

    circularBuf->setListener(fb);

    fb->setListener(fbListener);
}

static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argv, char** argc) {
    signal(SIGINT, intHandler);

    hackrf = std::make_shared<HackRFdevice>();
    hackrf->openAny();
    hackrf->set_freq(770000000);
    hackrf->amp_enable(false);
    hackrf->set_txvga_gain(10);

    circularBuf = std::make_shared<CircularBuffer>();
    fbListener = std::make_shared<FbListener>();

    setFB<SECAM_FrameBuffer>();

    hackrf->setCallbackListener(circularBuf);
    hackrf->startTransmit();

    printf("started\n");

    while (keepRunning) {
        sleep(1);
    }

    printf("finishing\n");

    hackrf->transferAbort();
    hackrf->close();
}

// g++ -I../ -I/usr/include/libhackrf/ main.cpp ../libhackrfpp/hackrf_device.cpp ../circular_buffer/circular_buffer.cpp ../secam_framebuffer/framebuffer_base.cpp ../secam_framebuffer/progressive_fb.cpp ../secam_framebuffer/secam_framebuffer.cpp   -lhackrf
