#include "fast_convolution_layer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

#include "Halide.h"

void FastConvolutionLayer::Init(Parameters params) {
  // BEGIN: CS348K STUDENTS MODIFY THIS CODE
  // END: CS348K STUDENTS MODIFY THIS CODE
}

void FastConvolutionLayer::Run(Parameters params, Data data) {
  // BEGIN: CS348K STUDENTS MODIFY THIS CODE
  // see description of Parameters struct in fast_convolution_layer.hpp
  
  // input: input activations for the conv layer
  Halide::Buffer<float> in_func(data.input,
                              params.width,
                              params.height,
                              params.channels,
                              params.n);

  // input: weights and biases for all filters
  Halide::Buffer<float> b(data.biases, params.num_f);
  Halide::Buffer<float> W(data.weights, params.f_w, params.f_h, params.channels, params.num_f);
  
  // output: width x height x num_f tensor of activations
  Halide::Buffer<float> output_buffer(params.width, params.height, params.num_f, params.n); 
  
  Halide::Func forward("conv");
  Halide::Var x("x"), y("y"), z("z"), n("n");

  // set up boundary conditions. Halide will take care of boundary conditions on accesses to f_in_bound. 
  Halide::Func f_in_bound;
  f_in_bound = Halide::BoundaryConditions::repeat_edge(in_func, 0, params.width, 0, params.height, 0, params.channels, 0, params.n);

  // reduction domain.  
  Halide::RDom r(0, params.f_w, 0, params.f_h, 0, params.channels);

  // first initialize all outputs to the bias
  forward(x, y, z, n) = b(z);
  // now perform the convolutions on the inputs
  forward(x, y, z, n) += W(r.x, r.y, r.z, z) *
    f_in_bound(x + r.x - params.pad, y + r.y - params.pad, r.z, n);
  
  // realize() is the point where Halide performs the computation, storing the output in output_buffer 
  forward.realize(output_buffer);
  
  std::cout << "[DEBUG] output buffer size = "
            << output_buffer.get()->number_of_elements() << std::endl;
  
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
  // END: CS348K STUDENTS MODIFY THIS CODE
}
