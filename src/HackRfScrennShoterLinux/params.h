#include <stdio.h>
#include <stdlib.h>

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