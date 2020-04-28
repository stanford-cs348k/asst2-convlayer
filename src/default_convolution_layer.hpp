#ifndef HALIDE_CONVOLUTION_LAYER_HPP_
#define HALIDE_CONVOLUTION_LAYER_HPP_

#include "convolution_layer.hpp"

// A simple Halide implementation of a MobileNet convolution layer. This
// implementation does not provide an optimized schedule, and instead relies on
// a default Halide schedule. This implementation should be used as an example
// for getting Halide syntax right.
class HalideConvolutionLayer : public ConvolutionLayer {
 public:
  HalideConvolutionLayer() = default;

  void Init(Parameters params) override;
  void Run(Parameters params, Data data) override;
};

#endif  // HALIDE_CONVOLUTION_LAYER_HPP_
