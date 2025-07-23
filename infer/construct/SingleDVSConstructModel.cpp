#include "SingleDVSConstructModel.h"
#ifdef NVIDIA
#include "infer/logger.h"
#include "infer/logging.h"
#endif
#include <QDebug>
#include <QFile>
#include <iostream>
#ifdef TORCH_ENABLE
#undef slots
#include <torch/torch.h>
#define slots Q_SLOTS
#endif

using namespace construct;
#ifdef NVIDIA
#ifdef TORCHE_ENABLE
using namespace torch;
#endif
SingleDVSConstructModel::SingleDVSConstructModel(
    const QString &modelPath, SingleDVSConstructModelPolicyBase *p) {
  policy = p;
  QFile file(modelPath);
  if (!file.open(QFile::ReadOnly)) {
    qDebug() << "open model " << modelPath << " failed";
    exit(-1);
  }
  auto data = file.readAll();
  using namespace std;
  using namespace cv;

  if (deviceData[0] != nullptr) {
    cudaFree(deviceData[0]);
    deviceData[0] = nullptr;
  }
  if (deviceData[1] != nullptr) {
    cudaFree(deviceData[1]);
    deviceData[1] = nullptr;
  }

  auto res = cudaMalloc(&(deviceData[0]), policy->getInputSize());
  if (res != cudaSuccess) {
    qDebug() << "malloc image space failed " << policy->getInputSize();
    exit(-1);
  }
  res = cudaMalloc(&(deviceData[1]), policy->getOutputSize());
  if (res != cudaSuccess) {
    qDebug() << "malloc result space failed" << endl;
    exit(-1);
  }
  qDebug() << "cuda malloc finished";
  if (engine != nullptr) {
    engine->destroy();
    engine = nullptr;
  }
  auto runtime = nvinfer1::createInferRuntime(sample::gLogger.getTRTLogger());
  engine = runtime->deserializeCudaEngine(data.data(), data.size());
  qDebug() << "engine bingdings " << engine->getNbBindings();
  for (int i = 0; i < engine->getNbBindings(); i++) {
    auto d = engine->getBindingDimensions(i);
    qDebug() << "bingding " << i << " dims " << d.nbDims;
    for (int j = 0; j < d.nbDims; j++) {
      qDebug() << d.d[j];
    }
  }
  if (context != nullptr) {
    context->destroy();
    context = nullptr;
  }
  context = engine->createExecutionContext();
  nvinfer1::Dims dim;
  dim.nbDims = 4;
  dim.d[0] = 1;
  dim.d[1] = 1;
  dim.d[2] = 1280;
  dim.d[3] = 720;
  qDebug() << "init construct engine finish";
}

void *SingleDVSConstructModel::infer(void *data) {
  if (cudaMemcpy(deviceData[0], data, policy->getInputSize(),
                 cudaMemcpyHostToDevice) != cudaSuccess) {
    qDebug() << "cuda copy failed from host " << (uint64_t)data << " to device "
             << (uint64_t)deviceData[0];
  }
  qDebug() << "cuda copy from host " << (uint64_t)data << " to device "
           << (uint64_t)deviceData[0];
  // context->setBindingDimensions(0, nvinfer1::Dims4{1, 1, 720, 1280});
  if (!this->context->execute(1, deviceData)) {
    qDebug() << "infer construct execute model failed";
  } else {
    qDebug() << "infer construct";
  }
  uchar *res = new uchar[policy->getOutputSize()];
  cudaMemcpy(res, deviceData[1], policy->getOutputSize(),
             cudaMemcpyDeviceToHost);
  return res;
}
#endif