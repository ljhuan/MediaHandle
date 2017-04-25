#ifndef MEDIA_TYPE_H_
#define MEDIA_TYPE_H_
#include "media_element.h"

class Yuv420 : public BaseMediaBuffer {
public:
    Yuv420(uint32_t width, uint32_t height) : BaseMediaBuffer(width * height * 3 / 2) {
        width_ = width;
        height_ = height;
    }

    uint8_t * getY() {
        return data_;
    }

    uint8_t * getU() {
        return &data_[width_ * height_];
    }

    uint8_t * getV() {
        return &data_[width_ * height_ * 5 / 4];
    }

    uint32_t getWidth() {
        return width_;
    }

    uint32_t getHeight() {
        return height_;
    }
private:
    uint32_t width_ = 0;
    uint32_t height_ = 0;
};

#endif  // MEDIA_TYPE_H_
