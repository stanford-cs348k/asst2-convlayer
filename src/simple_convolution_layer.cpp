#include "simple_convolution_layer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

void SimpleConvolutionLayer::Init(Parameters params) {
}

void SimpleConvolutionLayer::Run(Parameters params, Data data) {
  auto input_index = [&params] (int i, int j, int c) {
    return c * (params.width + 2) * (params.height + 2) +
        j * (params.width + 2) + i;
  };
  auto tmp_index = [&params] (int i, int j, int c) {
    return c * params.width * params.height + j * params.width + i;
  };
  auto output_index = [&params] (int i, int j, int c) {
    return c * params.width * params.height + j * params.width + i;
  };
  auto depthwise_weights_index = [&params] (int i, int j, int c) {
    return c * params.k * params.k + j * params.k + i;
  };
  auto pointwise_weights_index = [&params] (int c1, int c2) {
    return c2 * params.channels + c1;
  };
  float* tmp = new float[params.width * params.height * params.channels];
  // (1) Depthwise convolution. Note that the spatial kernel is centered
  // around the output pixel, so we need an offset.
  const int kernel_start = -(params.k / 2);
  for (int i = 0; i < params.width; i++) {
    for (int j = 0; j < params.height; j++) {
      for (int c = 0; c < params.f; c++) {
        float val = 0.f;
        for (int ii = 0; ii < params.k; ii++) {
          for (int jj = 0; jj < params.k; jj++) {
            // Note that we have to add 1 to the i and j inputs to the input
            // index function due to padding.
            const int in_index = input_index(i + ii + kernel_start + 1,
                                             j + jj + kernel_start + 1,
                                             c);
            const int dw_index = depthwise_weights_index(ii, jj, c);
            val += data.depthwise_weights[dw_index] * data.input[in_index];
          }
        }
        tmp[tmp_index(i, j, c)] = val;
      }
    }
  }
  // (2) Batch norm.
  for (int c = 0; c < params.channels; c++) {
    const float average = data.depthwise_average[c];
    const float variance = data.depthwise_variance[c];
    const float beta = data.depthwise_beta[c];
    const float gamma = data.depthwise_gamma[c];
    const float scale = gamma / sqrt(variance + params.epsilon);
    for (int i = 0; i < params.width; i++) {
      for (int j = 0; j < params.height; j++) {
        const int index  = tmp_index(i, j, c);
        tmp[index] = scale * (tmp[index] - average) + beta;
      }
    }
  }
  // (3) ReLU.
  auto relu = [] (float* array, int count) {
    for (int i = 0; i < count; i++) array[i] = std::max(array[i], 0.f);
  };
  relu(tmp, params.width * params.height * params.channels);
  // (4) Pointwise convolution.
  for (int i = 0; i < params.width; i++) {
    for (int j = 0; j < params.height; j++) {
      for (int c = 0; c < params.f; c++) {
        float val = 0.f;
        for (int cc = 0; cc < params.channels; cc++) {
          const int tmp_ind = tmp_index(i, j, cc);
          const int pw_index = pointwise_weights_index(c, cc);
          val += data.pointwise_weights[pw_index] * tmp[tmp_ind];
        }
        data.output[output_index(i, j, c)] = val;
      }
    }
  }
  // (5) Batch norm.
  for (int c = 0; c < params.f; c++) {
    const float average = data.pointwise_average[c];
    const float variance = data.pointwise_variance[c];
    const float beta = data.pointwise_beta[c];
    const float gamma = data.pointwise_gamma[c];
    const float scale = gamma / sqrt(variance + params.epsilon);
    for (int i = 0; i < params.width; i++) {
      for (int j = 0; j < params.height; j++) {
        const int index  = output_index(i, j, c);
        data.output[index] = scale * (data.output[index] - average) + beta;
      }
    }
  }
  // (6) ReLU.
  relu(data.output, params.width * params.height * params.f);

  delete[] tmp;
}
