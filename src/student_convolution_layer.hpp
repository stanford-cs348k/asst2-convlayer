#ifndef STUDENT_CONVOLUTION_LAYER_HPP_
#define STUDENT_CONVOLUTION_LAYER_HPP_

#include "convolution_layer.hpp"

// Your implementation of a MobileNet convolution layer. The functions Init()
// and Run() are left empty for you to implement.
class StudentConvolutionLayer : public ConvolutionLayer {
 public:
  StudentConvolutionLayer() = default;

  // An auxiliary function called once, before Run() is ever called (however,
  // Run() may be called multiple times, but Init() will only be called
  // once). In particular, you can allocate any temporary memory in this
  // function, so that it doesn't count towards your running time.
  void Init(Parameters params) override;
  // The core convolution layer code goes in Run(). See convolution_layer.hpp
  // for details on the Parameters and Data classes.
  void Run(Parameters params, Data data) override;

 private:
  // BEGIN: CS348K STUDENTS MODIFY THIS CODE
  // You are free to add any internal functions or members to help your
  // implementation. In particular, you are allowed to store any temporary
  // variables you need in Run() as private members.
  // END: CS348K STUDENTS MODIFY THIS CODE
};

#endif  // STUDENT_CONVOLUTION_LAYER_HPP_
