SRC_DIR := src

UNAME = $(shell uname)

HALIDE_DIR=/Users/dillon/CppWorkspace/Halide
BUILD_DIR := build
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDES := -I$(HALIDE_DIR)/include -I./$(BUILD_DIR)
DEFINES := -DUSE_HALIDE
LDFLAGS := -L$(HALIDE_DIR)/bin -L./$(BUILD_DIR) -lHalide -ldl -lpthread
CPPFLAGS :=
CXXFLAGS := -std=c++11 -g

CXX ?= g++

# Ignore this field unless you have dynamic loading problems
# on mac
ORIGINAL_HALIDE_PATH := leave/blank/if/you/dont/have/dyload/problems

convlayer: $(OBJ_FILES) $(BUILD_DIR)/DefaultConvLayerGenerator.a $(BUILD_DIR)/StudentConvLayerGenerator.a $(BUILD_DIR)/AutoConvLayerGenerator.a
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ $(LDFLAGS) -o $(BIN_DIR)/$@
# Uncomment this code if you have dyload problems on mac
# and then set ORIGINAL_HALIDE_PATH based on the results of `otool -L conv_layer_generator`
#ifeq ($(UNAME), Darwin)
	#install_name_tool -change $(ORIGINAL_HALIDE_PATH)/libHalide.dylib $(HALIDE_DIR)/bin/libHalide.dylib $(BIN_DIR)/$@
#endif

clean:
	\rm -rf $(BUILD_DIR) $(BIN_DIR)
	\rm -f conv_layer_generator

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)/DefaultConvLayerGenerator.a $(BUILD_DIR)/StudentConvLayerGenerator.a $(BUILD_DIR)/AutoConvLayerGenerator.a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<


conv_layer_generator:
	$(CXX) conv_layer_generators.cpp $(HALIDE_DIR)/tools/GenGen.cpp -g -std=c++11 -fno-rtti -I $(HALIDE_DIR)/include -L $(HALIDE_DIR)/bin -lHalide -lpthread -ldl -o conv_layer_generator

$(BUILD_DIR)/StudentConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	./conv_layer_generator -g StudentConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false

$(BUILD_DIR)/DefaultConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	./conv_layer_generator -g DefaultConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false


$(BUILD_DIR)/AutoConvLayerGenerator.a: conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	./conv_layer_generator -g AutoConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=true
