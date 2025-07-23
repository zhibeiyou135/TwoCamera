#ifndef TWOCAMERA_CROPCONFIG_H
#define TWOCAMERA_CROPCONFIG_H

#include <atomic>
#include <mutex>

class CropConfig {
public:
    static CropConfig* getInstance() {
        static CropConfig instance;
        return &instance;
    }

    bool getEnableCrop() const { return enableCrop.load(); }
    void setEnableCrop(bool enable) { enableCrop.store(enable); }

    int getX() const { return x.load(); }
    void setX(int value) { x.store(value); }

    int getY() const { return y.load(); }
    void setY(int value) { y.store(value); }

    int getWidth() const { return width.load(); }
    void setWidth(int value) { width.store(value); }

    int getHeight() const { return height.load(); }
    void setHeight(int value) { height.store(value); }

private:
    CropConfig() : enableCrop(true), x(1737), y(1052), width(2375), height(1447) {}
    std::atomic<bool> enableCrop;
    std::atomic<int> x;       // 裁剪起始x坐标
    std::atomic<int> y;       // 裁剪起始y坐标
    std::atomic<int> width;   // 裁剪宽度
    std::atomic<int> height;  // 裁剪高度
};

#endif // TWOCAMERA_CROPCONFIG_H 