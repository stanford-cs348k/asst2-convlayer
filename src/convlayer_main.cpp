#include <algorithm>
#include <chrono>
#include <cassert>
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
    if (!(diff < eps)) {
      cout << "Buffers differ by " << diff << " at " << i << ", ref[" << i << "] = " << ref[i] << ", data[" << i << "] = " << data[i] << endl;
    }
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
  if (argc != 9) {
    cout << "Error: Given " << argc << " arguments, expected 9" << endl;
    cout << "usage: ./bin/convlayer <scheduling algorithm> <width> <height> <channels> <batch size> <num filters> <filter width> <filter height>" << endl;
    return 1;
  }

  string schedule = argv[1];
  
  int width = stoi(argv[2]);
  int height = stoi(argv[3]);
  int channels = stoi(argv[4]);
  int n = stoi(argv[5]);
  
  int num_f = stoi(argv[6]);
  int f_w = stoi(argv[7]);
  int f_h = stoi(argv[8]);

  cout << "Schedule: " << schedule << endl;
  cout << "width   : " << width << endl;
  cout << "height  : " << height << endl;
  cout << "channels : " << channels << endl;
  cout << "batch size: " << n << endl;
  cout << "num filters: " << num_f << endl;
  cout << "filter width: " << f_w << endl;
  cout << "filter height: " << f_h << endl << endl;

  ConvolutionLayer::Parameters params;
  params.width = width;
  params.height = height;
  params.channels = channels;

  params.num_f = num_f;
  params.f_w = f_w;
  params.f_h = f_h;

  params.n = n;

  // Hardcoded in generator cpp as well
  params.pad = 2;

  ConvolutionLayer::Data data;
  data.biases = FillRandom(params.num_f);
  data.weights = FillRandom(params.f_h*params.f_w*params.channels*params.num_f);
  data.input = FillRandom(params.width*params.height*params.channels*params.n);
  data.output = FillZero(params.width*params.height*params.n*params.num_f);

  if (schedule == "default") {
    std::unique_ptr<ConvolutionLayer> reference_conv_layer(new HalideConvolutionLayer);
    reference_conv_layer->Init(params);

    double min_time = 1e10;

    for (int i = 0; i < 3; i++) {
      double total_elapsed = 0.;
      auto start = std::chrono::system_clock::now();
      reference_conv_layer->Run(params, data);
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = end - start;
      total_elapsed += elapsed.count();
      if (total_elapsed < min_time) {
        min_time = total_elapsed;
      }
    }
    std::cout << "Reference Convolution layer took " << min_time << " secconds" << std::endl;
  } else if (schedule == "student") {
    std::unique_ptr<ConvolutionLayer> fast_conv_layer(new FastConvolutionLayer);
    fast_conv_layer->Init(params);

    double min_time = 1e10;

    for (int i = 0; i < 3; i++) {
      double total_elapsed = 0.;
      auto start = std::chrono::system_clock::now();
      fast_conv_layer->Run(params, data);
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = end - start;
      total_elapsed += elapsed.count();
      if (total_elapsed < min_time) {
        min_time = total_elapsed;
      }
    }

    std::cout << "Fast Convolution layer took " << min_time << " secconds" << std::endl;
  } else if (schedule == "auto") {
    cout << "Autoscheduled conv not yet implemented" << endl;
    assert(false);
  } else {
    cout << "Error: Unsupported schedule \"" << schedule << "\", options are \"default\", \"auto\", \"student\"" << endl;
    assert(false);
  }


  delete data.input;
  delete data.output;
  delete data.biases;
  delete data.weights;

  return 0;
}
