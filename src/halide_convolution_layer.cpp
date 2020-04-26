#include "halide_convolution_layer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

#include "Halide.h"

void HalideConvolutionLayer::Init(Parameters params) {
}

void HalideConvolutionLayer::Run(Parameters params, Data data) {

  Halide::Buffer<float> in_func(data.input,
                              params.width,
                              params.height,
                              params.channels,
                              params.n);

  Halide::Buffer<float> b(data.biases, params.num_f);
  Halide::Buffer<float> W(data.weights, params.f_w, params.f_h, params.channels, params.num_f);

  Halide::Func forward("conv");
  Halide::Var x("x"), y("y"), z("z"), n("n");

  Halide::Func f_in_bound;
  f_in_bound = Halide::BoundaryConditions::repeat_edge(in_func, 0, params.width, 0, params.height, 0, params.channels, 0, params.n);

  Halide::RDom r(0, params.f_w, 0, params.f_h, 0, params.channels);

  forward(x, y, z, n) = b(z);
  forward(x, y, z, n) += W(r.x, r.y, r.z, z) *
    f_in_bound(x + r.x - params.pad, y + r.y - params.pad, r.z, n);

  std::cout << "Loop nests..." << std::endl;
  forward.print_loop_nest();

  // Realize output buffer and copy to data.output pointer.
  Halide::Buffer<float> output_buffer(params.width, params.height, params.num_f, params.n);
  forward.realize(output_buffer);
  std::cout << "[DEBUG] output buffer size = "
            << output_buffer.get()->number_of_elements() << std::endl;
  // Copy data from Halide buffer to raw buffer in data.output.
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
