#include "infer/classification/NiguangModelPolicyPlus.h"
#include <QDebug>
using namespace std;
const QString &NiguangModelPolicyPlus::processOutput(uchar *out) {
  qDebug() << "call NiguangPlus";
  float *data = (float *)out;
  qDebug() << "infer result " << ls[argmax<float>(data, CLASSES)];
  auto currentIndex = argmax<float>(data, CLASSES);
  labelIndexCount[queue[queueIndex]] -=
      ((labelIndexCount[queue[queueIndex]] == 0) ? 0 : 1);
  queueIndex = (queueIndex + 1) % QUEUE_SIZE;
  queue[queueIndex] = currentIndex;
  labelIndexCount[queue[queueIndex]] += 1;
  return ls[argmax<int>(labelIndexCount, CLASSES)];
}