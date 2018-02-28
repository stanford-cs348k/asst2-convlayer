#ifndef SIMPLE_CONVOLUTION_LAYER_HPP_
#define SIMPLE_CONVOLUTION_LAYER_HPP_

#include "convolution_layer.hpp"

// A simple, native C++ implementation of a MobileNet convolution layer. This
// implementation is not optimized in anyway, and should be used as an example
// for getting the semantics of the network right.
class SimpleConvolutionLayer : public ConvolutionLayer {
 public:
  SimpleConvolutionLayer() = default;

  void Init(Parameters params) override;
  void Run(Parameters params, Data data) override;
};

#endif  // SIMPLE_CONVOLUTION_LAYER_HPP_
