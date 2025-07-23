#ifndef TWOCAMERA_DVDISPLAYCONFIG_H
#define TWOCAMERA_DVDISPLAYCONFIG_H

#include <QString>
#include <atomic>
#include <mutex>

class DVDisplayConfig {
public:
    static DVDisplayConfig* getInstance() {
        static DVDisplayConfig instance;
        return &instance;
    }

    // Display mode getter/setter
    std::string getDisplayMode() const { return displayMode; }
    void setDisplayMode(const std::string& mode) { 
        displayMode = mode; 
    }

    // Crop settings
    bool getCropEnabled() const { return cropEnabled.load(); }
    void setCropEnabled(bool enabled) { cropEnabled.store(enabled); }

    int getCropX() const { return cropX.load(); }
    void setCropX(int value) { cropX.store(value); }

    int getCropY() const { return cropY.load(); }
    void setCropY(int value) { cropY.store(value); }

    int getCropWidth() const { return cropWidth.load(); }
    void setCropWidth(int value) { cropWidth.store(value); }

    int getCropHeight() const { return cropHeight.load(); }
    void setCropHeight(int value) { cropHeight.store(value); }

    // Save settings
    bool getSaveEnabled() const { return saveEnabled.load(); }
    void setSaveEnabled(bool enabled) { saveEnabled.store(enabled); }

    std::string getSavePath() const { return savePath; }
    void setSavePath(const std::string& path) { savePath = path; }

    // Rotation settings
    bool getRotationEnabled() const { return rotationEnabled.load(); }
    void setRotationEnabled(bool enabled) { rotationEnabled.store(enabled); }

    double getRotationAngle() const { return rotationAngle.load(); }
    void setRotationAngle(double angle) { rotationAngle.store(angle); }

    std::string getRotationBorderMode() const { return rotationBorderMode; }
    void setRotationBorderMode(const std::string& mode) { rotationBorderMode = mode; }

    std::string getRotationInterpolation() const { return rotationInterpolation; }
    void setRotationInterpolation(const std::string& interp) { rotationInterpolation = interp; }

private:
    DVDisplayConfig() : 
        displayMode("original"),
        cropEnabled(false),
        cropX(100),
        cropY(50),
        cropWidth(640),
        cropHeight(480),
        saveEnabled(false),
        savePath("saved_crops"),
        rotationEnabled(false),
        rotationAngle(0.0),
        rotationBorderMode(""),
        rotationInterpolation("") {}

    std::string displayMode;   // "original" or "cropped"
    std::atomic<bool> cropEnabled;
    std::atomic<int> cropX;
    std::atomic<int> cropY;
    std::atomic<int> cropWidth;
    std::atomic<int> cropHeight;
    std::atomic<bool> saveEnabled;
    std::string savePath;
    std::atomic<bool> rotationEnabled;
    std::atomic<double> rotationAngle;
    std::string rotationBorderMode;
    std::string rotationInterpolation;
};

#endif // TWOCAMERA_DVDISPLAYCONFIG_H 