#include "TRTModel.h"
#include "infer/classification/ClassificationModelPolicy.h"
#include "infer/logger.h"
#include "infer/logging.h"
#include <QDebug>
#ifdef NVIDIA
TRTModel::TRTModel(const QString &path) {
  modelPath = path;
  init();
}
uchar *TRTModel::infer(const uchar *data) {
  cudaMemcpy(deviceData[0], data, policy->getInputSize(),
             cudaMemcpyHostToDevice);
  if (!this->context->execute(1, deviceData)) {
    qDebug() << "infer classify failed";
  }
  uchar *res = new uchar[policy->getOutputSize()];
  cudaMemcpy(res, deviceData[1], policy->getOutputSize(),
             cudaMemcpyDeviceToHost);
  return res;
}
TRTModel::~TRTModel() {
  if (deviceData[0] != nullptr) {
    cudaFree(deviceData[0]);
    deviceData[0] = nullptr;
  }
  if (deviceData[1] != nullptr) {
    cudaFree(deviceData[1]);
    deviceData[1] = nullptr;
  }
}
TRTModel *TRTModel::newModel(QString path) { return new TRTModel(path); }

void TRTModel::init() {
  if (policy == nullptr) {
    qDebug() << "init without model policy";
    return;
  }
  auto buffer = readNet(modelPath);
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
    cout << "malloc image space failed" << endl;
    exit(-1);
  }
  res = cudaMalloc(&(deviceData[1]), policy->getOutputSize());
  if (res != cudaSuccess) {
    cout << "malloc result space failed" << endl;
    exit(-1);
  }

  if (engine != nullptr) {
    engine->destroy();
    engine = nullptr;
  }
  auto runtime = nvinfer1::createInferRuntime(sample::gLogger.getTRTLogger());
  engine = runtime->deserializeCudaEngine(buffer.data(), buffer.size());
  if (context != nullptr) {
    context->destroy();
    context = nullptr;
  }
  context = engine->createExecutionContext();
  qDebug() << "init engine finish";
}
#endif