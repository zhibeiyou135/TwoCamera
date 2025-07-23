#ifndef INFER_SINGLE_DVS_CONSTRUCT_MODEL_H
#define INFER_SINGLE_DVS_CONSTRUCT_MODEL_H
#ifdef TORCH_ENABLE
// #include <torch/script.h>
#endif
#include "ConstructModelBase.h"
#include "SingleDVSConstructModelPolicy.h"
#include <QString>
#ifdef NVIDIA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#endif
namespace construct {
class SingleDVSConstructModel : public ConstructModelBase {
#ifdef NVIDIA
private:
  SingleDVSConstructModelPolicyBase *policy;
  void *deviceData[2]{nullptr, nullptr};
  nvinfer1::ICudaEngine *engine = nullptr;
  nvinfer1::IExecutionContext *context = nullptr;
  nvinfer1::ICudaEngine *loadEngine();
  int inferCount = 0;
  QString modelPath;

public:
  SingleDVSConstructModel(const QString &modelPath,
                          SingleDVSConstructModelPolicyBase *p);
  void *infer(void *) override;
#else
public:
  SingleDVSConstructModel(const QString &modelPath,
                          SingleDVSConstructModelPolicyBase *p) {}
  void *infer(void *p) override { return nullptr; }
#endif
};
} // namespace construct

#endif