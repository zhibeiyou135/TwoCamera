#ifndef INFER_CONSTRUCT_MODEL_BASE_H
#define INFER_CONSTRUCT_MODEL_BASE_H
namespace construct {
class ConstructModelBase {
public:
  virtual void *infer(void *) = 0;
  virtual ~ConstructModelBase() {}
};
} // namespace construct

#endif