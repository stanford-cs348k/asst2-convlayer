#include "halide_convolution_layer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

#ifdef USE_HALIDE
#include "Halide.h"
#endif  // USE_HALIDE

void HalideConvolutionLayer::Init(Parameters params) {
}

void HalideConvolutionLayer::Run(Parameters params, Data data) {
#ifdef USE_HALIDE
  Halide::Buffer<float> input(data.input,
                              params.width + 2,
                              params.height + 2,
                              params.channels);
  Halide::Func tmp("tmp");
  Halide::Var x("x");
  Halide::Var y("y");
  Halide::Var c("c");
  // (1) Depthwise convolution. Note that the spatial kernel is centered
  // around the output pixel, so we need an offset.
  {
    Halide::Buffer<float> w(
        data.depthwise_weights, params.k, params.k, params.channels);
    const int offset = params.k / 2;
    Halide::RDom r(0, params.k, 0, params.k, "depthwise_rdom");
    tmp(x, y, c) = 0.f;
    tmp(x, y, c) +=
        input(x + r.x - offset, y + r.y - offset, c) * w(r.x, r.y, c);
  }
  // (2) Batch norm.
  {
    // TODO(raj): Populate batch norm parameters from @data.
    Halide::Buffer<float> average(data.depthwise_average, params.channels);
    Halide::Buffer<float> variance(data.depthwise_variance, params.channels);
    Halide::Buffer<float> beta(data.depthwise_beta, params.channels);
    Halide::Buffer<float> gamma(data.depthwise_gamma, params.channels);
    Halide::Expr v = tmp(x, y, c);
    v = (v - average(c)) / Halide::sqrt(variance(c) + params.epsilon);
    tmp(x, y, c) = gamma(c) * v + beta(c);
  }
  // (3) ReLU.
  tmp(x, y, c) = Halide::max(tmp(x, y, c), 0.f);
  // (4) Pointwise convolution.
  Halide::Func output("output");
  {
    Halide::Buffer<float> w(data.pointwise_weights, params.f, params.channels);
    Halide::RDom r(0, params.f, "pointwise_rdom");
    output(x, y, c) = 0.f;
    output(x, y, c) += tmp(x, y, r.x) * w(c, r.x);
  }
  // (5) Batch norm.
  {
    // TODO(raj): Populate batch norm parameters from @data.
    Halide::Buffer<float> average(data.pointwise_average, params.channels);
    Halide::Buffer<float> variance(data.pointwise_variance, params.channels);
    Halide::Buffer<float> beta(data.pointwise_beta, params.channels);
    Halide::Buffer<float> gamma(data.pointwise_gamma, params.channels);
    Halide::Expr v = output(x, y, c);
    v = (v - average(c)) / Halide::sqrt(variance(c) + params.epsilon);
    output(x, y, c) = gamma(c) * v + beta(c);
  }
  // (6) ReLU.
  output(x, y, c) = Halide::max(output(x, y, c), 0.f);

  // Print the description of the Halide schedule (basically pseudo-code for the
  // loop ordering).
  printf("Pseudo-code for the schedule:\n");
  output.print_loop_nest();
  printf("\n");

  // Realize output buffer and copy to data.output pointer.
  Halide::Buffer<float> output_buffer(params.width, params.height, params.f);
  output_buffer.set_min(1, 1);
  output.realize(output_buffer);
  std::cout << "[DEBUG] output buffer size = "
            << output_buffer.get()->number_of_elements() << std::endl;
  // Copy data from Halide buffer to raw buffer in data.output.
  {
    int index = 0;
    for (int c = 0; c < params.f; c++) {
      for (int j = 0; j < params.height; j++) {
        for (int i = 0; i < params.width; i++) {
          data.output[index++] = output_buffer(i + 1, j + 1, c);
        }
      }
    }
  }
#endif  // USE_HALIDE
}
