#include <algorithm>
#include <chrono>
#include <cmath>		
#include <cstdlib> 
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>
#include "convolution_layer.hpp"
#include "fast_convolution_layer.hpp"

bool ReadData(std::string activations_filename,
              std::string weights_filename,
              ConvolutionLayer::Parameters* params,
              ConvolutionLayer::Data* data,
              std::string* error) {
  // Read in activations.
  {
    FILE* f = fopen(activations_filename.c_str(), "rb");
    if (not f) {
      error->assign("Could not open file " + activations_filename);
      return false;
    }
    fread(&(params->width), sizeof(params->width), 1, f);
    fread(&(params->height), sizeof(params->height), 1, f);
    fread(&(params->channels), sizeof(params->channels), 1, f);
    // The width and height given in the file are the nominal size of the
    // input. However, the actual data is padded by 1 on all sides.
    const int size =
        (params->width + 2) * (params->height + 2) * params->channels;
    data->input = new float[size];
    if (fread(data->input, sizeof(float), size, f) != size) {
      error->assign("Incorrect number of elements in activations file");
      return false;
    }
    std::cout << "Read activations: "
              << params->width << " x "
              << params->height << " x "
              << params->channels << std::endl;
    fclose(f);
  }
  // Read in data (weights).
  {
    FILE* f = fopen(weights_filename.c_str(), "rb");
    if (not f) {
      error->assign("Could not open file " + weights_filename);
      return false;
    }
    // Read in depthwise weights.
    {
      int channels;
      fread(&(params->k), sizeof(params->k), 1, f);
      fread(&channels, sizeof(channels), 1, f);
      if (channels != params->channels) {
        error->assign("Depthwise filter has wrong number of channels: " +
                      std::to_string(channels));
        return false;
      }
      const int size = params->k * params->k * params->channels;
      data->depthwise_weights = new float[size];
      if (fread(data->depthwise_weights, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in depthwise weights");
        return false;
      }
      std::cout << "Read depthwise weights: "
                << params->k << " x "
                << params->k << " x "
                << params->channels << std::endl;
    }
    // Read in pointwise weights.
    {
      int channels;
      fread(&(params->f), sizeof(params->f), 1, f);
      fread(&channels, sizeof(channels), 1, f);
      if (channels != params->channels) {
        error->assign("Pointwise filter has wrong number of channels: " +
                      std::to_string(channels));
        return false;
      }
      const int size = params->f * params->channels;
      data->pointwise_weights = new float[size];
      if (fread(data->pointwise_weights, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in pointwise weights");
        return false;
      }
      std::cout << "Read pointwise weights: "
                << params->f << " x "
                << params->channels << std::endl;
    }
    // Read in depthwise batch norm params.
    {
      const int size = params->channels;
      data->depthwise_average = new float[size];
      if (fread(data->depthwise_average, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in depthwise average");
        return false;
      }
      data->depthwise_variance = new float[size];
      if (fread(data->depthwise_variance, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in depthwise variance");
        return false;
      }
      data->depthwise_beta = new float[size];
      if (fread(data->depthwise_beta, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in depthwise beta");
        return false;
      }
      data->depthwise_gamma = new float[size];
      if (fread(data->depthwise_gamma, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in depthwise gamma");
        return false;
      }
    }
    // Read in pointwise batch norm params.
    {
      const int size = params->f;
      data->pointwise_average = new float[size];
      if (fread(data->pointwise_average, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in pointwise average");
        return false;
      }
      data->pointwise_variance = new float[size];
      if (fread(data->pointwise_variance, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in pointwise variance");
        return false;
      }
      data->pointwise_beta = new float[size];
      if (fread(data->pointwise_beta, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in pointwise beta");
        return false;
      }
      data->pointwise_gamma = new float[size];
      if (fread(data->pointwise_gamma, sizeof(float), size, f) != size) {
        error->assign("Incorrect number of elements in pointwise gamma");
        return false;
      }
    }
    fclose(f);
  }
  // Allocate output buffer.
  {
    const int size = params->width * params->height * params->f;
    data->output = new float[size];
  }
  return true;
}

float GetMaxAbsDifference(const float* a, const float* b, int size) {
  double max_diff = -1.;
  for (int i = 0; i < size; i++) {
    max_diff = std::max(max_diff, (double)(std::fabs(a[i] - b[i])));
  }
  return max_diff;
}

void Verify(std::string golden_filename, const float* output, int size) {
  FILE* f = fopen(golden_filename.c_str(), "rb");
  if (not f) {
    std::cout << "Could not open golden file " << golden_filename << std::endl;
    return;
  }
  std::vector<float> golden(size);
  if (fread(&(golden[0]), sizeof(float), size, f) != size) {
    std::cout << "Golden file is of the wrong size" << std::endl;
    return;
  }
  const auto max_diff = GetMaxAbsDifference(&(golden[0]), output, size);
  std::cout << "Maximum absolute difference from golden: "
            << max_diff << std::endl;
  fclose(f);
}

int main(int argc, char** argv) {
  // Parse command line arguments.
  if (argc < 5) {
    std::cout << "usage: " << argv[0]
              << " activations weights golden num_runs" << std::endl;
    return 0;
  }
  const std::string activations_filename = argv[1];
  const std::string weights_filename = argv[2];
  const std::string golden_filename = argv[3];
  const int num_runs = atoi(argv[4]);

  ConvolutionLayer::Parameters params;
  ConvolutionLayer::Data data;
  std::string err;
  if (not ReadData(
          activations_filename, weights_filename, &params, &data, &err)) {
    std::cout << "Error reading in data: " << err << std::endl;
    return 0;
  }
  std::unique_ptr<ConvolutionLayer> conv_layer(new FastConvolutionLayer);
  conv_layer->Init(params);

  // Run convolution layer implementation for num_runs which is specified on the
  // command line.
  double total_elapsed = 0.;
  for (int run = 0; run < num_runs; run++) {
    auto start = std::chrono::system_clock::now();
    conv_layer->Run(params, data);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_elapsed += elapsed.count();
    std::cout << "Convolution layer took " << elapsed.count()
              << " secconds" << std::endl;
    // Verify against golden.
    const int output_size = params.width * params.height * params.f;
    Verify(golden_filename, data.output, output_size);
  }
  const double average_elapsed = total_elapsed / (double)num_runs;
  std::cout << "Average time: " << average_elapsed << "s" << std::endl;


  return 0;
}
