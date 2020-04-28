# CS348K (Mini-) Assignment 2: <br/> Optimizing a Convolutional Layer in Halide #

In this assignment you will implement optimize the performance of code to evaluate a convolutional layer in a DNN. 
Implementing a conv layer [is easy](http://cs348k.stanford.edu/spring20/lecture/dnneval/slide_024) (we give you the algorithm in Halide in the starter code). The challenge is implementing the layer efficiently using the techniques described in class, such as SIMD vector processing, multi-core execution, and efficient blocking for cache locality. 

**You are allowed to use the reference Halide algorithm provided in the codebase verbatim** (see `fast_convolution_layer.cpp`). However, to improve the performance you will need to write an efficient Halide schedule. The starter code uses a naive/default Halide schedule, which corresponds to an evaluate order equivalent to code with loops that look like:

```
  // Initialization
  for n:
    for z:
      for y:
        for x:
          conv(...) = ...
  // Updates
  for n:
    for z:
      for y:
        for x:
          for r0:
            for r1:
              for r2:
                conv(...) = ...
```

Your job is to implement a custom Halide schedule that performs better than the default. (See [`Halide::Func::print_loop_nest()`](http://halide-lang.org/docs/class_halide_1_1_func.html#a365488c2eaf769c61635120773e541e1) to inspect and debug your schedule like this.)

# Resources and documentation #
* [Halide tutorials](http://halide-lang.org/tutorials/tutorial_introduction.html). In particular, see Tutorial 01 for a basic introduction, Tutorial 07 for a convolution example, and Tutorial 05 for an introduction to Halide schedules, and Tutorial 08 for more advanced scheduling topics.
* [Exhaustive Halide documentation](http://halide-lang.org/docs/).

# Assignment mechanics #

Grab the assignment starter code.

    git clone git@github.com:stanford-cs348k/asst2-mobilenet.git

To run the assignment, you will need to download the scene datasets, which you can get from the course staff upon request.

__Build Instructions__

The codebase uses a simple `Makefile` as the build system. However, there is a dependency on Halide.

To build the starter code, run `make` from the top level directory. The assignment source code is in `src/`, and object files and binaries will be populated in `build/` and `bin/` respectively.

To install and use Halide follow the instructions at http://halide-lang.org/. In particular, you should [download a binary release of Halide](https://github.com/halide/Halide/releases). Once you've downloaded and untar'd the release, say into directory `halide_dir`, change the previous lines back, and also the following line in `Makefile`

    HALIDE_DIR=/Users/kayvonf/halide

to

    HALIDE_DIR=<halide_dir>

Then you can build the code using the instructions above.

__Running the starter code:__

Just run:
    ./bin/convlayer student 64 30 3 400 5 6 7

This code will run your (initially empty) version of the convolution layer using the activations in `DATA_DIR/activations.bin` and weights in `DATA_DIR/weights.bin`. It will run for `num_runs` trials, and report the timings across all runs, as well as validate the output against the data contained in `DATA_DIR/golden.bin`. To run correctly you must ensure that
Halide is in your library load path. On OSX this can be done like so:

    DYLD_LIBRARY_PATH=<halide_dir>/bin ./bin/convlayer <args>

and on Linux it will be:

    LD_LIBRARY_PATH=<halide_dir>/bin ./bin/convlayer <args>

__Modifying the code__

Your modifications to the code should only go in files `fast_convolution_layer.hpp` and `fast_convolution_layer.cpp`, in the regions marked

    // BEGIN: CS348K STUDENTS MODIFY THIS CODE
    
    // END: CS348K STUDENTS MODIFY THIS CODE

We have provided a reference implementation in `halide_convolution_layer.cpp`. You can use any of the code in these files for your implementation. In particular, you can copy the Halide algorithm (and just provide a custom schedule).
