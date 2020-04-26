#include <algorithm>
#include <chrono>
#include <cmath>		
#include <cstdlib> 
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>
#include "convolution_layer.hpp"
#include "halide_convolution_layer.hpp"
#include "fast_convolution_layer.hpp"

using namespace std;

float* FillZero(const int size) {
  auto buf = new float[size];
  for (int i = 0; i < size; i++) {
    buf[i] = 0;
  }
  return buf;
}

void CompareBuffers(float* ref, float* data, float eps, int size) {
  for (int i = 0; i < size; i++) {
    float diff = abs(ref[i] - data[i]);
    assert(diff < eps);
  }
}

float* FillRandom(const int size) {
  auto buf = new float[size];
  for (int i = 0; i < size; i++) {
    buf[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  }
  return buf;
}

int main(int argc, char** argv) {
  ConvolutionLayer::Parameters params;
  params.num_f = 64;
  params.f_w = 8;
  params.f_h = 14;
  params.channels = 64;
  params.width = 47;
  params.height = 32;
  params.n = 7;
  params.pad = 2;

  ConvolutionLayer::Data data;
  data.biases = FillRandom(params.num_f);
  data.weights = FillRandom(params.f_h*params.f_w*params.channels*params.num_f);
  data.input = FillRandom(params.width*params.height*params.channels*params.n);
  data.output = FillZero(params.width*params.height*params.n*params.num_f);

  ConvolutionLayer::Data fast_data;
  fast_data.biases = data.biases;
  fast_data.weights = data.weights;
  fast_data.input = data.input;
  fast_data.output = FillZero(params.width*params.height*params.n*params.num_f);

  {
    std::unique_ptr<ConvolutionLayer> reference_conv_layer(new HalideConvolutionLayer);
    reference_conv_layer->Init(params);

    double total_elapsed = 0.;
    auto start = std::chrono::system_clock::now();
    reference_conv_layer->Run(params, data);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_elapsed += elapsed.count();
    std::cout << "Reference Convolution layer took " << elapsed.count() << " secconds" << std::endl;
  }


  {
    std::unique_ptr<ConvolutionLayer> fast_conv_layer(new FastConvolutionLayer);
    fast_conv_layer->Init(params);

    double total_elapsed = 0.;
    auto start = std::chrono::system_clock::now();
    fast_conv_layer->Run(params, fast_data);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_elapsed += elapsed.count();
    std::cout << "Fast Convolution layer took " << elapsed.count() << " secconds" << std::endl;
  }

  int output_size =
    params.width*params.height*params.n*params.num_f;
  float eps = 0.00001;
  CompareBuffers(data.output, fast_data.output, eps, output_size);

  std::cout << "Fast result matches reference" << endl;

  delete data.input;
  delete data.output;
  delete data.biases;
  delete data.weights;

  return 0;
}
