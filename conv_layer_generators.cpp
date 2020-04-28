#include "Halide.h"
#include <stdio.h>

using namespace Halide;

class DefaultConvLayerGenerator : public Halide::Generator<DefaultConvLayerGenerator> {
public:
    Input<Buffer<float>> in_func{"in_func", 4};
    Input<Buffer<float>> b{"biases", 1};
    Input<Buffer<float>> W{"Weights", 4};

    Output<Buffer<float>> forward{"forward", 4};

    Var x, y, z, n;

    void generate() {
      // Hardcoded in driver harness as well
      const int pad = 2;

      Halide::Func f_in_bound;
      f_in_bound = Halide::BoundaryConditions::repeat_edge(in_func, 0, in_func.width(), 0, in_func.height(), 0, in_func.channels(), 0, in_func.dim(3).extent());

      // Add these as generator parameters?
      Halide::RDom r(0, W.width(), 0, W.height(), 0, W.channels());

      forward(x, y, z, n) = b(z);
      forward(x, y, z, n) += W(r.x, r.y, r.z, z) *
        f_in_bound(x + r.x - pad, y + r.y - pad, r.z, n);

      std::cout << "Loop nests..." << std::endl;
      forward.print_loop_nest();
    }
};

HALIDE_REGISTER_GENERATOR(DefaultConvLayerGenerator, DefaultConvLayerGenerator)

class StudentConvLayerGenerator : public Halide::Generator<StudentConvLayerGenerator> {
public:
    Input<Buffer<float>> in_func{"in_func", 4};
    Input<Buffer<float>> b{"biases", 1};
    Input<Buffer<float>> W{"Weights", 4};

    Output<Buffer<float>> forward{"forward", 4};

    Var x, y, z, n;

    void generate() {
      // Hardcoded in driver harness as well
      const int pad = 2;

      Halide::Func f_in_bound;

      // set up boundary conditions. Halide will take care of boundary conditions on accesses to f_in_bound. 
      f_in_bound = Halide::BoundaryConditions::repeat_edge(in_func, 0, in_func.width(), 0, in_func.height(), 0, in_func.channels(), 0, in_func.dim(3).extent());

      // reduction domain.  
      Halide::RDom r(0, W.width(), 0, W.height(), 0, W.channels());

      // first initialize all outputs to the bias
      forward(x, y, z, n) = b(z);
      // now perform the convolutions on the inputs
      forward(x, y, z, n) += W(r.x, r.y, r.z, z) *
        f_in_bound(x + r.x - pad, y + r.y - pad, r.z, n);

      std::cout << "Loop nests..." << std::endl;
      forward.print_loop_nest();

      // BEGIN: CS348K STUDENTS MODIFY THIS CODE
      // Insert your conv layer schedule here
      
      // END: CS348K STUDENTS MODIFY THIS CODE

    }
};


HALIDE_REGISTER_GENERATOR(StudentConvLayerGenerator, StudentConvLayerGenerator)


class AutoConvLayerGenerator : public Halide::Generator<AutoConvLayerGenerator> {
public:
    Input<Buffer<float>> in_func{"in_func", 4};
    Input<Buffer<float>> b{"biases", 1};
    Input<Buffer<float>> W{"Weights", 4};

    Output<Buffer<float>> forward{"forward", 4};

    Var x, y, z, n;

    void generate() {
      // Hardcoded in driver harness as well
      const int pad = 2;

      Halide::Func f_in_bound;

      // set up boundary conditions. Halide will take care of boundary conditions on accesses to f_in_bound. 
      f_in_bound = Halide::BoundaryConditions::repeat_edge(in_func, 0, in_func.width(), 0, in_func.height(), 0, in_func.channels(), 0, in_func.dim(3).extent());

      // reduction domain.  
      Halide::RDom r(0, W.width(), 0, W.height(), 0, W.channels());

      // first initialize all outputs to the bias
      forward(x, y, z, n) = b(z);
      // now perform the convolutions on the inputs
      forward(x, y, z, n) += W(r.x, r.y, r.z, z) *
        f_in_bound(x + r.x - pad, y + r.y - pad, r.z, n);

      std::cout << "Loop nests..." << std::endl;
      forward.print_loop_nest();

      // Autoschedule
      if (auto_schedule) {
        in_func.set_estimates({{0, 28}, {0, 28}, {0, 3}, {0, 16}});
        b.set_estimates({{0, 3}});
        W.set_estimates({{0, 3}, {0, 3}, {0, 3}, {0, 256}});
        forward.set_estimates({{0, 28}, {0, 28}, {0, 3}, {0, 16}});
      }
    }
};


HALIDE_REGISTER_GENERATOR(AutoConvLayerGenerator, AutoConvLayerGenerator)
