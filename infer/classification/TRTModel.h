#ifndef TRT_MODEL_H
#define TRT_MODEL_H
#include "ClassificationModelBase.h"
#include <QByteArray>
#ifdef NVIDIA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#endif

class TRTModel : public ClassificationModelBase {
private:
#ifdef NVIDIA
  void *deviceData[2]{nullptr, nullptr};
  nvinfer1::ICudaEngine *engine = nullptr;
  nvinfer1::IExecutionContext *context = nullptr;
  nvinfer1::ICudaEngine *loadEngine();
  int inferCount = 0;
  QString modelPath;

public:
  void init() override;
  uchar *infer(const uchar *data) override;
  static TRTModel *newModel(QString path);
  TRTModel(const QString &path);
  ~TRTModel();
#else
public:
  TRTModel(const QString &path) {}
  uchar *infer(const uchar *data) override { return nullptr; }
  void init() override {}
#endif
};
#endif