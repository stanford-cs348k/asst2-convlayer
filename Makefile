SRC_DIR := src

UNAME = $(shell uname)

# Note to CS348K students: this is the location of your Halide
# installation on your machine.
HALIDE_DIR=/Users/dillon/CppWorkspace/Halide

# Note to CS348K students: On some OSX platforms we need to patch up
# the dyld path in the generated lib.  This value is being set to the
# default that is in the Halide binary as downloaded in April 2020.
ORIGINAL_HALIDE_DYLIB_PATH := /Users/halidenightly/build_bot_new/worker/mac-64-800/halide-build/bin

BUILD_DIR := build
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDES := -I$(HALIDE_DIR)/include -I./$(BUILD_DIR)
DEFINES := -DUSE_HALIDE
LDFLAGS := -L$(HALIDE_DIR)/bin -L./$(BUILD_DIR) -lHalide -ldl -lpthread
CPPFLAGS :=
CXXFLAGS := -std=c++11 -g -O3

CXX ?= g++


convlayer: $(OBJ_FILES) $(BUILD_DIR)/DefaultConvLayerGenerator.a $(BUILD_DIR)/StudentConvLayerGenerator.a $(BUILD_DIR)/AutoConvLayerGenerator.a
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

clean:
	\rm -rf $(BUILD_DIR) $(BIN_DIR)
	\rm -f conv_layer_generator

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)/DefaultConvLayerGenerator.a $(BUILD_DIR)/StudentConvLayerGenerator.a $(BUILD_DIR)/AutoConvLayerGenerator.a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<


conv_layer_generator: conv_layer_generators.cpp
	$(CXX) conv_layer_generators.cpp $(HALIDE_DIR)/tools/GenGen.cpp -g -std=c++11 -fno-rtti -I $(HALIDE_DIR)/include -L $(HALIDE_DIR)/bin -lHalide -lpthread -ldl -o conv_layer_generator
# Note to CS348K students: uncomment this code if you have dyload problems on mac
# and then set ORIGINAL_HALIDE_DYLIB_PATH based on the results of `otool -L conv_layer_generator`
#ifeq ($(UNAME), Darwin)
	#install_name_tool -change $(ORIGINAL_HALIDE_DYLIB_PATH)/libHalide.dylib $(HALIDE_DIR)/bin/libHalide.dylib $@
#endif

$(BUILD_DIR)/StudentConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	./conv_layer_generator -g StudentConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false

$(BUILD_DIR)/DefaultConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	./conv_layer_generator -g DefaultConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false


$(BUILD_DIR)/AutoConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	./conv_layer_generator -g AutoConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=true
