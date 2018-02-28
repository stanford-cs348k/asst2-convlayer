#ifndef CONVOLUTION_LAYER_HPP_
#define CONVOLUTION_LAYER_HPP_

// Interface for designing a convolutional layer of the MobileNet deep network.
class ConvolutionLayer {
 public:
  // Container for all the data involved in running a single MobileNet
  // layer. See struct Parameters below to see "hyperparameters" which are
  // referred to in this struct.
  struct Data {
    // Input activiations. Size (width + 2) * (height + 2) * channels. The '+ 2'
    // in each dimension comes from the fact that the input activations are
    // padded to make the depthwise convolution easier. Accessing input(0, 0, c)
    // is really accessing the pixel logically at (-1, -1, c).
    float* input = nullptr;
    // Output activations. Size width * height * f.
    float* output = nullptr;
    // Depthwise convolution kernel weights. Size k * k * channels.
    float* depthwise_weights = nullptr;
    // Pointwise convolution kernel weights. Size f * channels.
    float* pointwise_weights = nullptr;
    // Depthwise batch norm learned parameters. All are size channels.
    float* depthwise_average = nullptr;
    float* depthwise_variance = nullptr;
    float* depthwise_beta = nullptr;
    float* depthwise_gamma = nullptr;
    // Pointwise batch norm learned parameters. All are size f.
    float* pointwise_average = nullptr;
    float* pointwise_variance = nullptr;
    float* pointwise_beta = nullptr;
    float* pointwise_gamma = nullptr;
  };
  // Hyperparameters for a MobileNet convolution layer. Most fields refer to
  // sizes of various objects in the layer.
  struct Parameters {
    // Width, height, and channels determine the size of the input activations,
    // and consequently dimensions of the convolution kernels.
    int width = 0;
    int height = 0;
    int channels = 0;
    // Support of the depthwise convolution kernel (note that it is square,
    // i.e. k * k, in the spatial dimensions).
    int k = 0;
    // Support of the pointwise convolution kernel. This determines the output
    // size.
    int f = 0;
    float epsilon = .00001f;  // used in batch norm division.
  };

  virtual void Init(Parameters params) = 0;
  virtual void Run(Parameters params, Data data) = 0;
};

#endif  // CONVOLUTION_LAYER_HPP_
