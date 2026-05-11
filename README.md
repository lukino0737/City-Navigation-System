# 城市导航模拟系统 (City-Navigation-System)

## 1. 项目简介
本项目是一个基于 C++ 和 Qt 6 (QML) 开发的城市导航模拟系统。系统能够支持 10,000 个节点的地图生成、空间查找、动态车流模拟以及基于实时路况的最短路径规划。

### 核心功能
- **F1. 地图显示与周边检索**：支持显示坐标附近的节点及关联路径。
- **F2. 最短路径规划**：计算两点间的静态最短物理距离（Dijkstra/A* 算法）。
- **F3. 动态车流模拟**：模拟道路拥堵状况，基于公式 `cLf(n/v)` 动态更新通行时间。
- **F4. 实时路况寻路**：综合考虑当前路况，计算行车时间最短的最优路径。
- **地图缩放与交互**：支持平移、缩放，并根据缩放级别过滤显示节点。

## 2. 开发环境
- **操作系统**: Windows (推荐 Win10/11)
- **开发工具**: Visual Studio 2022 (配合 Qt VS Tools 扩展) 或 Qt Creator
- **构建系统**: CMake 3.16+
- **编译器**: MSVC 2019/2022 64-bit
- **Qt 版本**: Qt 6.10+(需要组件: Qt Quick, Qt QML, Qt Core, Qt Gui)
## 3. 项目结构
```text
City-Navigation-System/
├── CMakeLists.txt             # 构建配置文件
├── src/                       # 源代码根目录
│   ├── main.cpp               # 入口程序
│   ├── api/                   # 接口定义层
│   │   └── NavigationAPI.h    # 团队协作的核心接口规范 (契约)
│   ├── core/                  # 业务逻辑层
│   │   ├── DataModel/         # 数据模型 (Graph 类, 地图加载/存盘)
│   │   └── modules/           # 核心算法模块 (寻路、生成、搜索、模拟)
│   └── gui/                   # 图形界面层 (MainWindow, MapView)
├── data/                      # 存放 map.json 等地图文件
├── 3rdparty/                  # 第三方库 (如 nlohmann_json)
└── Documents/                 # 项目需求与分工文档
```

## 4. 成员分工
- **成员 A (算法核心)**: 负责寻路引擎 (`PathFinder`)，实现静态与动态最短路径算法。
- **成员 B (空间与动态引擎)**: 负责周边检索 (`SpatialSearch`) 和交通流模拟逻辑 (`TrafficSimulator`)。
- **成员 C (图形交互与底层数据)**: 负责地图生成与持久化 (`DataGenerator`, `Graph`) 以及 UI 界面集成 (`MapView`, `MainWindow`)。

## 5. 快速开始
1. 克隆仓库。
2. 在 Visual Studio 中通过 "打开文件夹" 加载 CMake 项目。
3. 确保已配置 Qt 6 环境路径。
4. 编译并运行项目。