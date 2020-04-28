SRC_DIR := src

HALIDE_DIR=/Users/dillon/CppWorkspace/Halide/
BUILD_DIR := build
BIN_DIR := bin

SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRC_FILES))

INCLUDES := -I$(HALIDE_DIR)/include
DEFINES := -DUSE_HALIDE
LDFLAGS := -L$(HALIDE_DIR)/bin -lHalide -ldl -lpthread
CPPFLAGS :=
CXXFLAGS := -std=c++11 -g

CXX ?= g++

lesson_15_generate:
	g++ lesson_15*.cpp $(HALIDE_DIR)/tools/GenGen.cpp -g -std=c++11 -fno-rtti -I ../include -L ../bin -lHalide -lpthread -ldl -o lesson_15_generate

convlayer: $(OBJ_FILES) lesson_15_generate.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

clean:
	\rm -rf $(BUILD_DIR) $(BIN_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c -o $@ $<
