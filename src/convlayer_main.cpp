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
#include "auto_convolution_layer.hpp"

#include "args.hxx"


using namespace args;
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
  string schedule = "default";
  int width = 28;
  int height = 28;
  int channels = 3;
  int n = 16;

  int num_f = 256;
  int f_w = 3;
  int f_h = 3;

  args::ArgumentParser parser("Runs the scheduled convolution layer.", "");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

  args::ValueFlag<std::string> schedule_algorithm(parser, "schedule algorithm", "The algorithm used to schedule the convolution. Legal values are auto, student, and default", {'s', "schedule"});

  args::ValueFlag<int> width_arg(parser, "width", "The input image width.", {'w', "width"});
  args::ValueFlag<int> height_arg(parser, "height", "The input image height.", {'h', "height"});
  args::ValueFlag<int> channels_arg(parser, "channels", "The number of channels in the input image.", {'c', "channels"});
  args::ValueFlag<int> batch_size_arg(parser, "batch size", "The number of images processed in a batch.", {'b', "batch-size"});

  args::ValueFlag<int> num_filters_arg(parser, "num filters", "The number of filters applied to each image.", {"num-filters"});
  args::ValueFlag<int> filter_width_arg(parser, "filter width", "The filter width.", {"filter-width"});
  args::ValueFlag<int> filter_height_arg(parser, "filter height", "The filter height.", {"filter-height"});

  try
  {
    parser.ParseCLI(argc, argv);
  }
  catch (args::Help)
  {
    std::cout << parser;
    return 0;
  }
  catch (args::ParseError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  catch (args::ValidationError e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  if (schedule_algorithm) {
    schedule = args::get(schedule_algorithm);
  }

  if (width_arg) {
    width = get(width_arg);
  }

  if (height_arg) {
    height = get(height_arg);
  }
  
  if (channels_arg) {
    channels = get(channels_arg);
  }

  if (batch_size_arg) {
    n = get(batch_size_arg);
  }

  if (num_filters_arg) {
    num_f = get(num_filters_arg);
  }

  if (filter_width_arg) {
    f_w = get(filter_width_arg);
  }

  if (filter_height_arg) {
    f_h = get(filter_height_arg);
  }

  cout << "Schedule     : " << schedule << endl;
  cout << "width        : " << width << endl;
  cout << "height       : " << height << endl;
  cout << "channels     : " << channels << endl;
  cout << "batch size   : " << n << endl;
  cout << "num filters  : " << num_f << endl;
  cout << "filter width : " << f_w << endl;
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
    std::cout << "Default schedule convolution layer: " << min_time << " seconds" << std::endl;
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

    std::cout << "Student schedule convolution layer: " << min_time << " seconds" << std::endl;
  } else if (schedule == "auto") {
    std::unique_ptr<ConvolutionLayer> auto_conv_layer(new AutoConvolutionLayer);
    auto_conv_layer->Init(params);

    double min_time = 1e10;

    for (int i = 0; i < 3; i++) {
      double total_elapsed = 0.;
      auto start = std::chrono::system_clock::now();
      auto_conv_layer->Run(params, data);
      auto end = std::chrono::system_clock::now();
      std::chrono::duration<double> elapsed = end - start;
      total_elapsed += elapsed.count();
      if (total_elapsed < min_time) {
        min_time = total_elapsed;
      }
    }

    std::cout << "Auto-scheduler convolution layer:" << min_time << " seconds" << std::endl;
  } else {
    cout << "Error: Unsupported schedule \"" << schedule << "\", options are \"default\", \"auto\", \"student\"" << endl;
    assert(false);
  }


  delete[] data.input;
  delete[] data.output;
  delete[] data.biases;
  delete[] data.weights;

  return 0;
}
