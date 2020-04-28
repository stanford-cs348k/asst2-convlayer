#include "default_convolution_layer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

#include "Halide.h"

#include "DefaultConvLayerGenerator.h"

void DefaultConvolutionLayer::Init(Parameters params) {
}

void DefaultConvolutionLayer::Run(Parameters params, Data data) {

  Halide::Runtime::Buffer<float> in_func(data.input,
                              params.width,
                              params.height,
                              params.channels,
                              params.n);
  Halide::Runtime::Buffer<float> b(data.biases, params.num_f);
  Halide::Runtime::Buffer<float> W(data.weights, params.f_w, params.f_h, params.channels, params.num_f);
  Halide::Runtime::Buffer<float> output_buffer(params.width, params.height, params.num_f, params.n);
 
  DefaultConvLayerGenerator(in_func, b, W, output_buffer);

  std::cout << "[DEBUG] output buffer size = "
            << output_buffer.number_of_elements() << std::endl;
            //<< output_buffer.get()->number_of_elements() << std::endl;
  // Copy data from Default buffer to raw buffer in data.output.
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
