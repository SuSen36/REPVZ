# XP-PVZ (XPlant-Plants vs Zombies)

## 项目介绍
这是一个使用 SDL2 和 GLAD 重新实现的植物大战僵尸游戏，支持跨平台（Windows/Linux/Android）。本项目基于社区反编译的植物大战僵尸源码，进行了现代化重构和跨平台移植。

## 特性
- 使用现代 C++17 标准
- 跨平台支持 (Windows/Linux/Android)
- 基于 SDL2 的图形渲染
- GLAD OpenGL 加载器
- BASS 音频引擎
- CMake 构建系统

## 目录结构
```
├── android-project/    # Android 平台相关文件
├── build/             # 构建输出目录
├── CMake/             # CMake 配置文件
├── Libs/              # 第三方库
│   ├── SDL/          # SDL2 库
│   └── zlib/         # zlib 压缩库
├── Resources/         # 游戏资源文件
│   ├── compiled/     # 编译后的资源
│   ├── data/         # 游戏数据
│   ├── images/       # 图片资源
│   ├── particles/    # 粒子效果
│   ├── properties/   # 配置文件
│   ├── reanim/       # 动画资源
│   └── sounds/       # 音频资源
├── src/              # 源代码
│   ├── Lawn/         # 游戏主要逻辑
│   ├── SexyAppFramework/  # 游戏框架
│   └── Sexy.TodLib/  # 工具库
└── tools/            # 开发工具
    ├── copy_resources.sh  # 资源复制脚本
    ├── decrypt.c     # 资源解密工具
    └── metadata.c    # 元数据处理工具
```

## 构建说明

在开始构建之前，请确保已初始化并更新了所有子模块：

```bash
git submodule update --init --recursive
```

### Windows
需求：
- CMake 3.2.2 或更高版本
- MinGW-w64 或 MSVC
- SDL2 开发库
- BASS 音频库

```bash
mkdir build
cd build
cmake ..
cmake --build .

# 复制资源文件
../tools/copy_resources.sh ../Resources .
```

### Linux
需求：
- CMake 3.2.2+
- GCC/Clang
- SDL2 开发库
- BASS 音频库
- X11 开发库
- OpenGL 开发库

```bash
sudo apt-get update
sudo apt-get install libx11-dev libxkbcommon-dev
mkdir build
cd build
cmake ..
cmake --build .

# 复制资源文件
chmod +x ../tools/copy_resources.sh
../tools/copy_resources.sh ../Resources .
```

### Android
需求：
- Android Studio
- Android NDK
- CMake 3.2.2+
- SDL2 for Android
- BASS for Android

使用 Android Studio 打开 android-project 目录进行构建。

```bash
# 复制资源文件到Android项目
./tools/copy_resources.sh Resources android-project/app/src/main/assets
```

## 运行
Windows/Linux 平台编译完成后，游戏数据文件需要放在可执行文件同目录下。
Android 平台的数据文件存放在 Android/data/com.popcap.pvz/files 目录下。

## 开发者
- 移植者：SuSen36

## 许可说明
本项目仅用于学习和研究目的。所有游戏资源的版权归 PopCap Games 所有。

## 贡献指南
欢迎提交 Issue 和 Pull Request 来改进项目。在提交代码前，请确保：
1. 代码符合项目的编码规范
2. 通过所有平台的编译测试
3. 注意变量使用的前缀符号：“m”表示类成员，“the”表示传递给方法或函数的参数，“a”表示局部变量

## 已知问题
- 如遇到编译问题，请检查 CMake 配置和依赖库是否正确安装
- 音频相关问题请确认 BASS 库是否正确配置
- 资源文件复制失败时，请检查脚本执行权限和路径是否正确
