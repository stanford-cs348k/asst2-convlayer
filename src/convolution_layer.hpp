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
   
    // input to the conv layer is: width x height x channels x n
    int width = 0;     
    int height = 0;
    int channels = 0;   // number of channels per image (e.g., 3 for an rgb image)
    int n = 0;          // number of images in the batch

    // output of the conv layer is: width x height x num_f
    int num_f = 0;      // number of filters
    int f_h = 0;        // height of filter kernel (e.g., f_w=3 and f_h=3 for a 3x3 conv layer)
    int f_w = 0;        // width of filter kernel

    int pad = 0;
  };

  virtual void Init(Parameters params) = 0;
  virtual void Run(Parameters params, Data data) = 0;
  virtual ~ConvolutionLayer() {}
};

#endif  // CONVOLUTION_LAYER_HPP_
