#include <vector>
using namespace std;

// 基础数据结构定义
struct Point {
    int id;
    double x, y;
};

struct PathResult {
    vector<int> pathNodes; // 途径的节点ID序列
    double totalCost;      // 总花费（距离或时间）
};

// ==========================================
// 成员 C 提供的接口 (底层数据加载)
// ==========================================
// 生成地图并存盘
bool generateAndSaveMap(int nodeCount, string filePath);
// 读取地图文件到内存
bool loadMap(string filePath); 

// ==========================================
// 成员 B 提供的接口 (空间搜索与动态车流)
// ==========================================
// 获取距离 (x,y) 最近的 count 个点
vector<Point> getNearestPoints(double x, double y, int count);
// 更新全图的交通状况（每次调用，车流状态按公式刷新一次）
void updateTrafficStatus();
// 获取某条边的当前拥堵系数（供成员A算权重，以及成员C画颜色使用）
double getEdgeTrafficWeight(int edgeId);

// ==========================================
// 成员 A 提供的接口 (核心寻路引擎)
// ==========================================
// 计算最短路径。considerTraffic 为 false 算最短距离，为 true 算最短时间
PathResult calculateShortestPath(int startId, int endId, bool considerTraffic);