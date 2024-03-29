# CS348K (Mini-) Assignment 2: <br/> Optimizing a Convolutional Layer in Halide #

__Due 11:59pm, April 29th__

In this assignment you will create an efficient schedule for a DNN convolution layer in Halide. 
Implementing the *Halide algorithm* for a conv layer [is quite easy](https://gfxcourses.stanford.edu/cs348k/spring22/lecture/dnninference/slide_13) (we give you the algorithm in Halide in the starter code). The challenge is coming up with an efficient schedule. Good schedules will use a combination of ideas discussed in class, such as: SIMD vector processing, multi-core execution, and efficient blocking for cache locality. 

In general, this is a free-for-all assignment.  We want to you learn a bit about writing Halide schedules, and try your hand at making performance go faster.  You *will* have to do some Halide documentation reading on your own.

# Assignment mechanics #

__Step 1: Grab the assignment starter code:__

    git clone https://github.com/stanford-cs348k/asst2-convlayer.git
   
The codebase uses a simple `Makefile` as the build system. However, there is a dependency on Halide.  

__Step 2: Install Halide:__

To install and use Halide follow the instructions at http://halide-lang.org/. In particular, you should [download a binary release of Halide](https://github.com/halide/Halide/releases). You do not need to install Halide from source. We recommend version 14, but older versions should also be acceptable for this assignment, if you're using an old linux distribution that lacks an up-to-date glibc. Make sure you select the correct OS and architecture for your device: a linux user with an Intel or AMD CPU should download `Halide-14.0.0-x86-64-linux-HASH.tar.gz` for example, while a user of a new M1 Mac should download `Halide-14.0.0-arm-64-osx-HASH.tar.gz`. Additionally, macOS users should ensure that they download the release tarball via the terminal, rather than their browser. When downloading the tarball via browser, macOS marks all the shared libraries inside as untrusted, and will stop the Halide compiler from loading them when building the project. Therefore use curl as follows:

    curl -OJL <TARBALL_URL_FROM_RELEASE_PAGE>

Once you've downloaded the release, extract the tarball as follows (replacing `<TARBALL_PATH>` with the path to the downloaded tarball).

    tar -xvf <TARBALL_PATH>

Now that you've downloaded and untar'd the release, say into directory `my_halide_dir`, change the line in the `Makefile` that looks like this

    HALIDE_DIR ?= /PATH/TO/Halide-14.0.0-ARCH-OS

to (without the `<>`)

    HALIDE_DIR = <my_halide_dir>

__Step 3: Build the code:__

To build the starter code, run `make` from the top level directory. The driver source code is in `src/`, and
the implementation of the convolution layer generator you will modify is in the top-level file `conv_layer_generators.cpp`.
Object files and binaries will be populated in `build/` and `bin/` respectively.

When complete, this will create a binary `bin/convlayer`.

You will see the following output when building the code:

```
asst2-convlayer $ make
c++ conv_layer_generators.cpp <my_halide_dir>/share/Halide/tools/GenGen.cpp -g -std=c++17 -fno-rtti -I<my_halide_dir>/include -I./build -L<my_halide_dir>/bin -L<my_halide_dir>/lib -L./build -lHalide -ldl -lpthread -Wl,-rpath,<my_halide_dir>/lib -o build/conv_layer_generator
build/conv_layer_generator -g DefaultConvLayerGenerator -o ./build target=host auto_schedule=false
build/conv_layer_generator -g StudentConvLayerGenerator -o ./build target=host auto_schedule=false
Loop nests...
produce forward:
  for v3:
    for v2:
      for v1:
        for v0:
          forward(...) = ...
  for v3:
    for v2:
      for v1:
        for v0:
          for r21:
            for r21:
              for r21:
                forward(...) = ...
build/conv_layer_generator -g AutoConvLayerGenerator -o ./build target=host auto_schedule=true -p <my_halide_dir>/lib/libautoschedule_mullapudi2016.so
Warning: Outer dim vectorization of var "v0" in function "forward.update(0)"
c++  -std=c++17 -g -O3 -DUSE_HALIDE -I<my_halide_dir>/include -I./build -c -o build/auto_convolution_layer.o src/auto_convolution_layer.cpp
c++  -std=c++17 -g -O3 -DUSE_HALIDE -I<my_halide_dir>/include -I./build -c -o build/convlayer_main.o src/convlayer_main.cpp
c++  -std=c++17 -g -O3 -DUSE_HALIDE -I<my_halide_dir>/include -I./build -c -o build/default_convolution_layer.o src/default_convolution_layer.cpp
c++  -std=c++17 -g -O3 -DUSE_HALIDE -I<my_halide_dir>/include -I./build -c -o build/student_convolution_layer.o src/student_convolution_layer.cpp
c++ build/auto_convolution_layer.o build/convlayer_main.o build/default_convolution_layer.o build/student_convolution_layer.o build/DefaultConvLayerGenerator.a build/StudentConvLayerGenerator.a build/AutoConvLayerGenerator.a -L<my_halide_dir>/bin -L<my_halide_dir>/lib -L./build -lHalide -ldl -lpthread -Wl,-rpath,<my_halide_dir>/lib -o bin/convlayer
```

__Running the starter code:__

To get commandline help, run the command:

    ./bin/convlayer --help

To run your scheduled conv layer, please run the command:

    ./bin/convlayer --schedule student

This code will run your version of the convolution layer using randomly generated inputs and weights. It will run for three trials, and report the minimum time of the three runs.

You can also compile the Halide program with a schedule generated by the Halide autoscheduler using a value "auto" for the algorithm type. 

    ./bin/convlayer --schedule auto

You can also use "default" to get the default Halide schedule. 

    ./bin/convlayer --schedule default

Here is output from two different program runs on a M1 Pro laptop.  Notice that the autoscheduler produces a schedule that is about 10 times faster than the default schedule.

```
asst2-convlayer $ ./bin/convlayer --schedule auto
Schedule     : auto
width        : 128
height       : 128
channels     : 3
batch size   : 16
num filters  : 256
filter width : 3
filter height: 3

Auto-scheduler convolution layer:0.128934 seconds
asst2-convlayer $ ./bin/convlayer --schedule student
Schedule     : student
width        : 128
height       : 128
channels     : 3
batch size   : 16
num filters  : 256
filter width : 3
filter height: 3

Student schedule convolution layer: 1.63975 seconds
```

__Modifying the code__

Programming in Halide is a form of [meta-programming](https://en.wikipedia.org/wiki/Metaprogramming).  Halide is embedded in C++, so you write C++ code that calls the Halide API to build up a DAG of Halide operations (a Halide program representation).  Then Halide compiles this representation into a library, which is linked by the binary `convlayer`.  

In the code base, a generator is a C++ class that creates a Halide program.  Your modifications to the code should only go in the file `conv_layer_generators.cpp` inside the class `StudentConvLayerGenerator`. Inside that file you **should not modify the implementation of the convolution layer algorithm.** You should only add Halide scheduling directives to the program to make it run faster. Regions you should modify will be marked by comments that look like this:

    // BEGIN: CS348K STUDENTS MODIFY THIS CODE
    
    // END: CS348K STUDENTS MODIFY THIS CODE

We have provided a reference implementation in `conv_layer_generators.cpp` called `DefaultConvLayerGenerator`. This implementation generates a conv layer implementation using the default Halide schedule. The code generated by `DefaultConvLayerGenerator` is called by `HalideConvolutionLayer`, which is located in the source file `./src/default_convolution_layer.cpp`.

The starter code uses a naive/default Halide schedule, which corresponds to an evaluation order equivalent to code with loops that look like:

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

Your job is to implement a custom Halide schedule that makes notable improvements on the default. (See [`Halide::Func::print_loop_nest()`](https://halide-lang.org/docs/class_halide_1_1_func.html#a03f839d9e13cae4b87a540aa618589ae) to inspect and debug your schedule like this.). Note: you can use the autoscheduler to get a sense of how much performance you have to gain, although you are not expected to equal the autoscheduler in performance.  You might also be interested to compare the performance of the autoscheduler against a hand-optimized professional grade performance library like the [Intel oneAPI Deep Neural Network Library](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onednn.html).  There's a lot of performance to be gained!

You may wish to consider:
* Parallelization onto multiple CPU cores (`.parallel()`)    
* SIMD vector processing (`.vectorize()`) or [loop unrolling](https://en.wikipedia.org/wiki/Loop_unrolling) (`.unroll()`)
* Tiling the computation using `.split()` and `.reorder`.
* More advanced implementations might even consider data layout transformations the interchange the storage order of different axes. See [tutorial 16](https://halide-lang.org/tutorials/tutorial_lesson_16_rgb_generate.html) and [tutorial 17](https://halide-lang.org/tutorials/tutorial_lesson_17_predicated_rdom.html) for more info.

**IMPORTANT NOTE ABOUT THE AUTOSCHEDULER:** The auto-scheduler is currently optimizing assuming the input dimensions that are currently hardcoded in `conv_layer_generators.cpp`. (Please see [here](https://github.com/stanford-cs348k/asst2-convlayer/blob/master/conv_layer_generators.cpp#L116-L119).) If you compare performance against the autoscheduler for different inputs you should  change these parameters to the autoscheduler schedules specifically to your new conv layer size.

# Resources and Documentation # 

* [Halide tutorials](http://halide-lang.org/tutorials/tutorial_introduction.html). In particular, see Tutorial 01 for a basic introduction, Tutorial 07 for a 2D convolution example, and Tutorial 05 for an introduction to Halide schedules, and Tutorial 08 for more advanced scheduling topics.
* [Exhaustive Halide documentation](http://halide-lang.org/docs/). 
* This is a [good Youtube video](https://www.youtube.com/watch?time_continue=476&v=3uiEyEKji0M&feature=emb_logo) about Halide scheduling. 

# Handin and Grading #

Your handin should contain two components:
   * Your full source tree (as a zip file). Please make sure you `make clean` prior to zipping.
   * __A writeup, in a file named `writeup.pdf`, that describes the iterative process you used to arrive at your solution.__  At each step, we expect the writeup to say:
       * I tried XXX for the following reason.
       * Then I measured my performance and I saw an increase/decrease, etc.
       
This assignment is all about performance optimization, but we are not grading on the actual performance obtained.  We are grading on a three category basis: no-credit, credit, amazing.  Anyone would makes a legimate effort to optimized the code should get credit.  Just please make sure your handin writeup properly describes your process.  Students that obtain impressively high performance will get the grade "amazing"!

