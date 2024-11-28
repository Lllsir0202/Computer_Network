# 编译器和编译选项
CXX := clang++
CXXFLAGS := -std=c++11 -Wall -Iinclude -Iinclude/datamanager -Iinclude/recv -Iinclude/send

# 路径
INCLUDE_DIR := include
CLIENT_DIR := client
SERVER_DIR := server
BUILD_DIR := build

# 公共代码文件
COMMON_SRCS := $(wildcard $(INCLUDE_DIR)/*.cpp) \
               $(wildcard $(INCLUDE_DIR)/datamanager/*.cpp) \
               $(wildcard $(INCLUDE_DIR)/recv/*.cpp) \
               $(wildcard $(INCLUDE_DIR)/send/*.cpp)
COMMON_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(COMMON_SRCS)))

# 客户端代码文件
CLIENT_SRCS := $(wildcard $(CLIENT_DIR)/*.cpp)
CLIENT_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(CLIENT_SRCS)))

# 服务端代码文件
SERVER_SRCS := $(wildcard $(SERVER_DIR)/*.cpp)
SERVER_OBJS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SERVER_SRCS)))

# 最终生成的可执行文件
CLIENT_BIN := $(BUILD_DIR)/client
SERVER_BIN := $(BUILD_DIR)/server

# 默认目标
all: $(CLIENT_BIN) $(SERVER_BIN)

# 客户端目标
$(CLIENT_BIN): $(COMMON_OBJS) $(CLIENT_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 服务端目标
$(SERVER_BIN): $(COMMON_OBJS) $(SERVER_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 编译规则：公共代码
$(BUILD_DIR)/%.o: $(INCLUDE_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译规则：客户端代码
$(BUILD_DIR)/%.o: $(CLIENT_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译规则：服务端代码
$(BUILD_DIR)/%.o: $(SERVER_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 单独目标
client: $(CLIENT_BIN)

server: $(SERVER_BIN)

# 清理
clean:
	rm -rf $(BUILD_DIR)
