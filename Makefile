SRC_DIR := src

UNAME = $(shell uname)

# Note to CS348K students: this is the location of your Halide
# installation on your machine.
# You can use Halide 10 or 11.
HALIDE_DIR=/home/durst/big_dev/cs348k/halide/Halide-10.0.0-x86-64-linux
# The below line fixes the linker path on OSX and sets a
# harmless variable on Linux
MAC_LIBRARY_PATH := DYLD_LIBRARY_PATH=$(HALIDE_DIR)/lib/

BUILD_DIR := build
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDES := -I$(HALIDE_DIR)/include -I./$(BUILD_DIR)
DEFINES := -DUSE_HALIDE
LDFLAGS := -L$(HALIDE_DIR)/bin -L$(HALIDE_DIR)/lib -L./$(BUILD_DIR) -lHalide -ldl -lpthread
LD_LIBRARY_PATH := $(HALIDE_DIR)/lib
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
	$(CXX) conv_layer_generators.cpp $(HALIDE_DIR)/share/Halide/tools/GenGen.cpp -g -std=c++11 -fno-rtti $(INCLUDES) $(LDFLAGS) -o conv_layer_generator
# Note to CS348K students: uncomment this code if you have dyload problems on mac
# and then set ORIGINAL_HALIDE_DYLIB_PATH based on the results of `otool -L conv_layer_generator`
#ifeq ($(UNAME), Darwin)
	#install_name_tool -change $(ORIGINAL_HALIDE_DYLIB_PATH)/libHalide.dylib $(HALIDE_DIR)/bin/libHalide.dylib $@
#endif

$(BUILD_DIR)/StudentConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	$(MAC_LIBRARY_PATH) ./conv_layer_generator -g StudentConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false

$(BUILD_DIR)/DefaultConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	$(MAC_LIBRARY_PATH) ./conv_layer_generator -g DefaultConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false


$(BUILD_DIR)/AutoConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	$(MAC_LIBRARY_PATH) ./conv_layer_generator -g AutoConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=true -p $(HALIDE_DIR)/lib/libautoschedule_mullapudi2016.so
