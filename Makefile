# ==========================================
# 原生 Linux 工具鏈與旗標
# ==========================================
CXX = g++
CC = gcc
CXXFLAGS = -g -std=c++23 -I./include $(shell pkg-config --cflags freetype2)
CFLAGS = -g -I./include
LDFLAGS = -lglfw -lGL -ldl -llua -lpthread $(shell pkg-config --libs freetype2)

BUILD_DIR = build
ASSETS_DIR = assets
TARGET = $(BUILD_DIR)/game_window
PACKAGE_LINUX = $(BUILD_DIR)/ProjectGirigara_linux.tar.gz

# ==========================================
# MinGW-w64 Windows 交叉編譯設定
# ==========================================
WIN_CXX = x86_64-w64-mingw32-g++
WIN_CC  = x86_64-w64-mingw32-gcc
WIN_AR  = x86_64-w64-mingw32-ar
WIN_PREFIX = /usr/x86_64-w64-mingw32/sys-root/mingw
WIN_LIB_DIR = ./libs/win64

# Vendor Lua 相關路徑
LUA_DIR = vendor/lua-5.5.1
LUA_SRC_DIR = $(LUA_DIR)/src
WIN_LUA_BUILD_DIR = $(BUILD_DIR)/win64_lua
WIN_LUA_LIB = $(WIN_LUA_BUILD_DIR)/liblua.a

# 篩選 Lua 核心 C 檔案（自動排除 standalone 的 lua.c 與 luac.c）
LUA_SRCS = $(filter-out $(LUA_SRC_DIR)/lua.c $(LUA_SRC_DIR)/luac.c, $(wildcard $(LUA_SRC_DIR)/*.c))
WIN_LUA_OBJS = $(patsubst $(LUA_SRC_DIR)/%.c, $(WIN_LUA_BUILD_DIR)/%.o, $(LUA_SRCS))

WIN_CXXFLAGS = -O2 -std=c++23 -I./include -I$(LUA_SRC_DIR) -I$(WIN_LIB_DIR)/include -I$(WIN_PREFIX)/include/freetype2
WIN_CFLAGS   = -O2 -I./include

# 全靜態鏈結：libglfw3.a、liblua.a、libgcc、libstdc++
WIN_LDFLAGS  = -L$(WIN_LIB_DIR)/lib -L$(WIN_LUA_BUILD_DIR) -L$(WIN_PREFIX)/lib \
               -lglfw3 -llua -lopengl32 \
               -lfreetype -lpng -lz -lbz2 \
               -lgdi32 -limm32 -lwinmm -lpthread \
               -static-libgcc -static-libstdc++ -static

WIN_BUILD_DIR = $(BUILD_DIR)/win64
WIN_TARGET = $(WIN_BUILD_DIR)/game_window.exe
PACKAGE_WIN = $(BUILD_DIR)/ProjectGirigara_win64.zip

# ==========================================
# 專案主程式原始碼
# ==========================================
SRC = src/main.cpp src/Girigara.cpp src/glad.c src/Shader.cpp \
      src/WindowManager.cpp src/ResourceManager.cpp src/UIContainer.cpp \
      src/UIElement.cpp src/UIButton.cpp src/UIText.cpp src/QuadMesh.cpp \
      src/UIWindow.cpp src/KeyboardSystem.cpp src/SceneManager.cpp \
      src/MainMenuScene.cpp src/GameScene.cpp src/ScriptEngine.cpp \
      src/ScriptBindings.cpp src/ScriptResource.cpp src/ConsoleSystem.cpp \
      src/FontLoader.cpp

OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp, $(SRC))) \
       $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(filter %.c, $(SRC)))

WIN_OBJS = $(patsubst src/%.cpp, $(WIN_BUILD_DIR)/%.o, $(filter %.cpp, $(SRC))) \
           $(patsubst src/%.c, $(WIN_BUILD_DIR)/%.o, $(filter %.c, $(SRC)))

# 預設建置：Linux 與 Windows 同時編譯打包
all: linux win64

# ==========================================
# Linux 建置流程
# ==========================================
linux: $(TARGET) copy_assets_linux clean_linux_objs package_linux

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copy_assets_linux: | $(BUILD_DIR)
	@rm -rf $(BUILD_DIR)/$(ASSETS_DIR)
	@cp -r $(ASSETS_DIR) $(BUILD_DIR)/

clean_linux_objs:
	@rm -f $(BUILD_DIR)/*.o

package_linux: $(TARGET) copy_assets_linux
	@echo "==> 打包 Linux 發布包至 $(PACKAGE_LINUX) ..."
	@tar -czf $(PACKAGE_LINUX) -C $(BUILD_DIR) game_window $(ASSETS_DIR)

# ==========================================
# Windows 交叉編譯流程
# ==========================================
win64: $(WIN_LUA_LIB) $(WIN_TARGET) copy_assets_win clean_win_objs package_win

# 1. 自動交叉編譯 vendor 中的 Lua 核心並打包成 liblua.a
$(WIN_LUA_LIB): $(WIN_LUA_OBJS)
	@echo "==> 打包 Windows 靜態庫 $(WIN_LUA_LIB) ..."
	$(WIN_AR) rcs $@ $(WIN_LUA_OBJS)

$(WIN_LUA_BUILD_DIR)/%.o: $(LUA_SRC_DIR)/%.c | $(WIN_LUA_BUILD_DIR)
	$(WIN_CC) -O2 -c $< -o $@

$(WIN_LUA_BUILD_DIR):
	mkdir -p $(WIN_LUA_BUILD_DIR)

# 2. 編譯主程式並靜態鏈結
$(WIN_TARGET): $(WIN_OBJS) $(WIN_LUA_LIB) | $(WIN_BUILD_DIR)
	@echo "==> 靜態鏈結 $(WIN_TARGET) ..."
	$(WIN_CXX) $(WIN_OBJS) -o $@ $(WIN_LDFLAGS)

$(WIN_BUILD_DIR)/%.o: src/%.cpp | $(WIN_BUILD_DIR)
	$(WIN_CXX) $(WIN_CXXFLAGS) -c $< -o $@

$(WIN_BUILD_DIR)/%.o: src/%.c | $(WIN_BUILD_DIR)
	$(WIN_CC) $(WIN_CFLAGS) -c $< -o $@

$(WIN_BUILD_DIR):
	mkdir -p $(WIN_BUILD_DIR)

copy_assets_win: | $(WIN_BUILD_DIR)
	@echo "==> 複製 assets 至 Windows 目錄..."
	@rm -rf $(WIN_BUILD_DIR)/$(ASSETS_DIR)
	@cp -r $(ASSETS_DIR) $(WIN_BUILD_DIR)/

clean_win_objs:
	@echo "==> 清理 Windows 中間檔案 (*.o)..."
	@rm -f $(WIN_BUILD_DIR)/*.o
	@rm -rf $(WIN_LUA_BUILD_DIR)

package_win: $(WIN_TARGET) copy_assets_win
	@echo "==> 壓縮 Windows 發布包至 $(PACKAGE_WIN)..."
	@rm -f $(PACKAGE_WIN)
	@cd $(WIN_BUILD_DIR) && zip -rq ../../$(PACKAGE_WIN) .
	@echo "==> Windows x86_64 發布包打包完成！"

# ==========================================
# 清理規則
# ==========================================
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all linux win64 copy_assets_linux clean_linux_objs package_linux \
        copy_assets_win clean_win_objs package_win clean
