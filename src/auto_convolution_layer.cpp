#include "auto_convolution_layer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

#include "Halide.h"

#include "AutoConvLayerGenerator.h"

void AutoConvolutionLayer::Init(Parameters params) {
}

void AutoConvolutionLayer::Run(Parameters params, Data data) {
  // see description of Parameters struct in fast_convolution_layer.hpp
  
  // input: input activations for the conv layer
  Halide::Runtime::Buffer<float> in_func(data.input,
                              params.width,
                              params.height,
                              params.channels,
                              params.n);

  // input: weights and biases for all filters
  Halide::Runtime::Buffer<float> b(data.biases, params.num_f);
  Halide::Runtime::Buffer<float> W(data.weights, params.f_w, params.f_h, params.channels, params.num_f);
  
  // output: width x height x num_f tensor of activations
  Halide::Runtime::Buffer<float> output_buffer(params.width, params.height, params.num_f, params.n); 

  AutoConvLayerGenerator(in_func, b, W, output_buffer);
  
  // Copy data from the Halide output buffer to a C buffer in data.output.
  {
    int index = 0;
    for (int n = 0; n < params.n; n++) {
      for (int c = 0; c < params.num_f; c++) {
        for (int j = 0; j < params.height; j++) {
          for (int i = 0; i < params.width; i++) {
            data.output[index++] = output_buffer(i, j, c, n);
          }
        }
      }
    }
  }
}
