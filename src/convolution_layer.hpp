#ifndef CONVOLUTION_LAYER_HPP_
#define CONVOLUTION_LAYER_HPP_

class ConvolutionLayer {
 public:

  // Container for all the data involved in running a single convolutional
  // layer. See struct Parameters below to see "hyperparameters" which are
  // referred to in this struct.
  struct Data {
    float* biases = nullptr;
    float* weights = nullptr;
    float* input = nullptr;
    float* output = nullptr;
  };

  // Hyperparameters for a convolution layer
  struct Parameters {
    int width = 0;
    int height = 0;
    int channels = 0;
    int n = 0;

    int num_f = 0;
    int f_h = 0;
    int f_w = 0;

    int in_w = 0;
    int in_h = 0;

    int pad = 0;
  };

  virtual void Init(Parameters params) = 0;
  virtual void Run(Parameters params, Data data) = 0;
  virtual ~ConvolutionLayer() {}
};

#endif  // CONVOLUTION_LAYER_HPP_
