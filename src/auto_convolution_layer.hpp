#ifndef AUTO_CONVOLUTION_LAYER_HPP_
#define AUTO_CONVOLUTION_LAYER_HPP_

#include "convolution_layer.hpp"

class AutoConvolutionLayer : public ConvolutionLayer {
 public:
  AutoConvolutionLayer() = default;

  // An auxiliary function called once, before Run() is ever called (however,
  // Run() may be called multiple times, but Init() will only be called
  // once). In particular, you can allocate any temporary memory in this
  // function, so that it doesn't count towards your running time.
  void Init(Parameters params) override;
  // The core convolution layer code goes in Run(). See convolution_layer.hpp
  // for details on the Parameters and Data classes.
  void Run(Parameters params, Data data) override;

 private:
};

#endif  // AUTO_CONVOLUTION_LAYER_HPP_
