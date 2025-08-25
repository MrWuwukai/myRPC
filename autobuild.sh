# 1. 删除旧的 build 目录（安全清理）
rm -rf build/

# 2. 创建新的 build 目录并进入
mkdir -p build && cd build

# 3. 运行 CMake，明确指定构建类型，比如 Debug
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 4. 编译项目
make