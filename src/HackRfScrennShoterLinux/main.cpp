#include <functional>
#include <libhackrfpp/hackrf_device.h>
#include <secam_framebuffer/secam_framebuffer.h>
#include <secam_framebuffer/progressive_fb.h>
#include <circular_buffer/circular_buffer.h>

#include <memory>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

class FbListener : public FrameBufferBase::IListener {
public:
    void onFrameRequest() override {
        std::unique_lock lk(lock);
        frameRequested_ = true;
        cv.notify_one();
    }

    bool waitForFrameRequest() {
        std::unique_lock lk(lock);
        cv.wait(lk, [this]{ return frameRequested_ || !keepWaiting_; });

        if (frameRequested_) {
            frameRequested_ = false;
            return true;
        }

        if (!keepWaiting_) {
            return false;
        }

        return false;
    }

    void stopWaiting() {
        std::unique_lock lk(lock);
        keepWaiting_ = false;
        cv.notify_one();
    }
private:
    std::condition_variable cv;
    std::mutex lock;
    bool keepWaiting_ = true;
    bool frameRequested_ = false;
};

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

struct Params {
    void printUsage() {
        printf("Usage: programm fifo width height\n");
    }

    bool initFromArgs(int argv, char** argc) {
        if (argv < 4) {
            printUsage();
            return false;
        }

        path = argc[1];
        width = atoi(argc[2]);
        height = atoi(argc[3]);

        if (width == 0 || height == 0) {
            printUsage();
            return false;
        }

        return true;
    }

    char* path;
    int width;
    int height;
};

class FifoReader {
public:
    FifoReader(int fd, int width, int height, std::shared_ptr<FbListener> fbListener, std::shared_ptr<FrameBufferBase> fb)
        : fd_(fd), width_(width), height_(height), fbListener_(fbListener), fb_(fb)
    {
        frameSize = width * height;
        readBuf.resize(frameSize);

        readThread_ = std::thread([this](){readLoop();});
        writeThread_ = std::thread([this](){pushLoop();});
    }

private:
    void pushLoop() {
        while (fbListener_->waitForFrameRequest()) {
            fb_->LoadBitmapGray8(width_, height_, readBuf.data());
        }

        keepRunning_ = false;
    }

    void readLoop() {
        while (keepRunning_) {
            int toRead = frameSize - readPos;
            ssize_t readed = read(fd_, &readBuf[readPos], toRead);

            if (readed < 0) {
                return;
            }

            readPos += readed;

            if (readPos >= frameSize) {
                //printf("frame read, tryReaded %d, last chunk: %d\n", toRead, (int)readed);
                readPos = 0;
            }
        }
    }

private:
    int fd_;
    int width_;
    int height_;

    std::atomic<bool> keepRunning_ = true;

    std::shared_ptr<FbListener> fbListener_;
    std::shared_ptr<FrameBufferBase> fb_;

    int frameSize;
    std::vector<char> readBuf;
    int readPos = 0;

    std::thread readThread_;
    std::thread writeThread_;
};


static volatile int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argv, char** argc) {
    Params params;
    if (!params.initFromArgs(argv, argc)) {
        printf("cannot parse args\n");
        return 0;
    }

    int fifoFd = open(params.path, O_RDONLY);
    if (fifoFd < 0) {
        printf("cannot open input file\n");
        return 0;
    }


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

    FifoReader fifoReader(fifoFd, params.width, params.height, fbListener, fb);

    printf("started\n");

    while (keepRunning) {
        sleep(1);
    }

    printf("finishing\n");
    fbListener->stopWaiting();

    hackrf->transferAbort();
    hackrf->close();

    close(fifoFd);
}

// g++ -I../ -I/usr/include/libhackrf/ main.cpp ../libhackrfpp/hackrf_device.cpp ../circular_buffer/circular_buffer.cpp ../secam_framebuffer/framebuffer_base.cpp ../secam_framebuffer/progressive_fb.cpp ../secam_framebuffer/secam_framebuffer.cpp   -lhackrf
