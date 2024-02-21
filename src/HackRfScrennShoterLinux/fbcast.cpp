#include "params.h"

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

typedef struct {
    int fd;
    void* ptr;
    struct fb_fix_screeninfo fixInfo;
    struct fb_var_screeninfo varInfo;
} FrameBuffer;

int FrameBuffer_init(FrameBuffer* self, char* file) {
    int iFrameBufferSize;

    self->fd = open(file, O_RDWR);
    
    /* Open the framebuffer device in read write */
    if (self->fd < 0) {
        printf("Cannot open framebuffer file\n");
        return -1;
    }

    /* Do Ioctl. Retrieve fixed screen info. */
    if (ioctl(self->fd, FBIOGET_FSCREENINFO, &self->fixInfo) < 0) {
        printf("get fixed screen info failed: %s\n",
              strerror(errno));
        close(self->fd);
        return -1;
    }
    /* Do Ioctl. Get the variable screen info. */
    if (ioctl(self->fd, FBIOGET_VSCREENINFO, &self->varInfo) < 0) {
        printf("Unable to retrieve variable screen info: %s\n",
              strerror(errno));
        close(self->fd);
        return -1;
    }

    /* Calculate the size to mmap */
    iFrameBufferSize = self->fixInfo.line_length * self->varInfo.yres;
    printf("Line length %d\n", self->fixInfo.line_length);
    printf("bits_per_pixel %d\n", self->varInfo.bits_per_pixel);
    printf("xres %d\n", self->varInfo.xres);
    printf("yres %d\n", self->varInfo.yres);
    printf("smem_len %d\n", self->fixInfo.smem_len);

    if (self->varInfo.bits_per_pixel != 32) {
        printf("bits_per_pixel is not 32\n");
        close(self->fd);
        return -1;
    }

    /* Now mmap the framebuffer. */
    self->ptr = mmap(NULL, iFrameBufferSize, PROT_READ | PROT_WRITE,
                     MAP_SHARED, self->fd,0);
    if (self->ptr == NULL) {
        printf("mmap failed:\n");
        close(self->fd);
        return -1;
    }
    return 0;
}

void FrameBuffer_close(FrameBuffer* self)
{
    munmap(self->ptr,0);
    close(self->fd);
}

void draw(FrameBuffer* self)
{
    unsigned char* fbp = (unsigned char*)self->ptr;

    // Figure out where in memory to put the pixel
    for (int y = 100; y < 300; y++)
        for (int x = 100; x < 300; x++) {

            

            int location = (x+self->varInfo.xoffset) * (self->varInfo.bits_per_pixel/8) +
                       (y+self->varInfo.yoffset) * self->fixInfo.line_length;

            if (self->varInfo.bits_per_pixel == 32) {
                *(fbp + location) = 100;        // Some blue
                *(fbp + location + 1) = 15+(x-100)/2;     // A little green
                *(fbp + location + 2) = 200-(y-100)/5;    // A lot of red
                *(fbp + location + 3) = 0;      // No transparency
        //location += 4;
            } else  { //assume 16bpp
                int b = 10;
                int g = (x-100)/6;     // A little green
                int r = 31-(y-100)/16;    // A lot of red
                unsigned short int t = r<<11 | g << 5 | b;
                *((unsigned short int*)(fbp + location)) = t;
            }

        }
}

class Grabber {
public:
    Grabber(const FrameBuffer& fb, int width, int height, int destFd) 
        : fb_(fb), width_(width), height_(height), destFd_(destFd)
        , tempBuf(new unsigned char[width_ * height_])
    {
    }

    ~Grabber() {
        delete[] tempBuf;
    }

    static unsigned char Color32BPPTobyte(uint32_t* data)
    {
        unsigned long int temp = *data;
        unsigned char r = (unsigned char)(temp >> 0) & 0xFF;
        unsigned char g = (unsigned char)(temp >> 8) & 0xFF;
        unsigned char b = (unsigned char)(temp >> 16) & 0xFF;
        // https://ru.wikipedia.org/wiki/YCbCr
        float y = (0.2627f * r + 0.678f * g + 0.0593f * b);

        if (y >= 255) {
            return 255;
        }

        return y;
    }

    void grab() {
        int xFbOffset = fb_.varInfo.xoffset;
        int bytesPerPixel = fb_.varInfo.bits_per_pixel/8;
        int yFbOffset = fb_.varInfo.yoffset;
        int rowLength = fb_.fixInfo.line_length;

        unsigned char* fbp = (unsigned char*)fb_.ptr;

        int writeSize = height_*width_;

        unsigned char* wptr = tempBuf;

        for (int y = 0; y < height_; y++) {
            int baseOffset = (y + yFbOffset) * rowLength;
            for (int x = 0; x < width_; x++) {
                *wptr = Color32BPPTobyte((uint32_t*)&fbp[baseOffset]);
                wptr++;
                baseOffset+=4;
            }
        }

        int result = write(destFd_, tempBuf, writeSize);
        if (result != writeSize) {
            printf("write fb size mismatch\n");
        }
    }

private:
    const FrameBuffer& fb_;
    const int width_;
    const int height_;
    const int destFd_;

    unsigned char* tempBuf;
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

    int fifoFd = open(params.path, O_WRONLY);
    if (fifoFd < 0) {
        printf("cannot open output file\n");
        return 0;
    }

    FrameBuffer fb = {0};
    if (FrameBuffer_init(&fb, "/dev/fb0") < 0) {
        return -1;
    }

    signal(SIGINT, intHandler);

    Grabber grabber(fb, params.width, params.height, fifoFd);

    while (keepRunning) {
        usleep(40000);
        grabber.grab();
    }

    FrameBuffer_close(&fb);
}