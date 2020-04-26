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

float* FillRandom(const int size) {
  auto buf = new float[size];
  for (int i = 0; i < size; i++) {
    buf[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  }
  return buf;
}

int main(int argc, char** argv) {
  ConvolutionLayer::Parameters params;
  params.num_f = 32;
  params.f_w = 8;
  params.f_h = 14;
  params.channels = 64;
  params.width = 32;
  params.height = 32;
  params.n = 7;
  params.pad = 2;

  ConvolutionLayer::Data data;

  data.biases = FillRandom(params.num_f);
  data.weights = FillRandom(params.f_h*params.f_w*params.channels*params.num_f);
  data.input = FillRandom(params.width*params.height*params.channels*params.n);
  data.output = FillZero(params.width*params.height*params.n*params.num_f);

  //std::unique_ptr<ConvolutionLayer> conv_layer(new FastConvolutionLayer);
  std::unique_ptr<ConvolutionLayer> reference_conv_layer(new HalideConvolutionLayer);
  reference_conv_layer->Init(params);

  double total_elapsed = 0.;
  auto start = std::chrono::system_clock::now();
  reference_conv_layer->Run(params, data);
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  total_elapsed += elapsed.count();
  std::cout << "Reference Convolution layer took " << elapsed.count() << " secconds" << std::endl;
  cout << "Output..." << endl;
  int out_size = params.width*params.height*params.num_f*params.n;
  for (int i = 0; i < out_size; i++) {
    cout << "  " << data.output[i] << endl;
  }

  delete data.input;
  delete data.output;
  delete data.biases;
  delete data.weights;

  return 0;
}
