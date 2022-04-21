SRC_DIR := src

UNAME = $(shell uname)

# Note to CS348K students: this is the location of your Halide
# installation on your machine.
HALIDE_DIR ?= /PATH/TO/Halide-14.0.0-ARCH-OS

BUILD_DIR := build
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDES := -I$(HALIDE_DIR)/include -I./$(BUILD_DIR)
DEFINES := -DUSE_HALIDE
LDFLAGS := -L$(HALIDE_DIR)/bin -L$(HALIDE_DIR)/lib -L./$(BUILD_DIR) -lHalide -ldl -lpthread -Wl,-rpath,$(HALIDE_DIR)/lib
CPPFLAGS :=
CXXFLAGS := -std=c++17 -g -O3

CXX ?= g++

convlayer: $(OBJ_FILES) $(BUILD_DIR)/DefaultConvLayerGenerator.a $(BUILD_DIR)/StudentConvLayerGenerator.a $(BUILD_DIR)/AutoConvLayerGenerator.a
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

clean:
	\rm -rf $(BUILD_DIR) $(BIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(BUILD_DIR)/DefaultConvLayerGenerator.a $(BUILD_DIR)/StudentConvLayerGenerator.a $(BUILD_DIR)/AutoConvLayerGenerator.a
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<


$(BUILD_DIR)/conv_layer_generator: conv_layer_generators.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) conv_layer_generators.cpp $(HALIDE_DIR)/share/Halide/tools/GenGen.cpp -g -std=c++17 -fno-rtti $(INCLUDES) $(LDFLAGS) -o $(BUILD_DIR)/conv_layer_generator

$(BUILD_DIR)/StudentConvLayerGenerator.a: $(BUILD_DIR)/conv_layer_generator
	$(BUILD_DIR)/conv_layer_generator -g StudentConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false

$(BUILD_DIR)/DefaultConvLayerGenerator.a: $(BUILD_DIR)/conv_layer_generator
	$(BUILD_DIR)/conv_layer_generator -g DefaultConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=false


$(BUILD_DIR)/AutoConvLayerGenerator.a: $(BUILD_DIR)/conv_layer_generator
	@mkdir -p $(BUILD_DIR)
	$(BUILD_DIR)/conv_layer_generator -g AutoConvLayerGenerator -o ./$(BUILD_DIR) target=host auto_schedule=true -p $(HALIDE_DIR)/lib/libautoschedule_mullapudi2016.so
