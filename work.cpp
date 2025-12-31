#include "raylib.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <queue>       
#include <stack>       
#include <map>         
#include <algorithm>   
#include <limits>      
#include <string>     

using namespace std;

// ========= 1. 数据结构和常量定义 =========
static const int TILE = 32;
int ROW, COL;
vector<vector<int>> Maze;
Texture2D texFloor, texWall, texStart, texEnd;
Texture2D texGrass, texLava;

struct Point {
    int x, y;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator<(const Point& other) const {
        if (y != other.y) return y < other.y;
        return x < other.x;
    }
    bool operator!=(const Point& other) const { return !(*this == other); }
};

Point startPoint, endPoint;
vector<Point> pathBFS, pathDFS, pathDijkstra;


// ========= 2. 迷宫加载和绘制 =========

void LoadMaze(const string& filename)
{
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "Error: Could not open maze file: " << filename << endl;
        exit(1);
    }
    fin >> ROW >> COL;
    Maze.assign(ROW, vector<int>(COL));

    for (int i = 0; i < ROW; i++) {
        int rowIndex;
        fin >> rowIndex;
        for (int j = 0; j < COL; j++) {
            fin >> Maze[i][j];
            if (Maze[i][j] == -1) { startPoint = { j, i }; }
            if (Maze[i][j] == -2) { endPoint = { j, i }; }
        }
    }
    fin.close();
}

void DrawMaze()
{
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            int v = Maze[i][j];
            Texture2D* tex = &texFloor;
            switch (v) {
            case -1: tex = &texStart; break;
            case -2: tex = &texEnd; break;
            case 0:  tex = &texFloor; break;
            case 1:  tex = &texWall; break;
            case 2:  tex = &texGrass; break;
            case 3:  tex = &texLava; break;
            default: tex = &texFloor; break;
            }
            DrawTexture(*tex, j * TILE, i * TILE, WHITE);
        }
    }
}

void DrawPath(const vector<Point>& path, Color color)
{
    for (const auto& p : path) {
        if (p == startPoint || p == endPoint) continue;
        DrawRectangle(p.x * TILE, p.y * TILE, TILE, TILE, color);
    }
}

// ========= 3. 寻路算法和辅助函数 (代码保持不变，确保熔岩不可通行) =========

vector<Point> ReconstructPath(const map<Point, Point>& parent, const Point& end) {
    vector<Point> path;
    Point curr = end;
    while (!(curr == startPoint)) {
        path.push_back(curr);
        if (parent.find(curr) == parent.end()) { return {}; }
        curr = parent.at(curr);
    }
    path.push_back(startPoint);
    reverse(path.begin(), path.end());
    return path;
}

bool isTraversable(int x, int y) {
    if (x < 0 || x >= COL || y < 0 || y >= ROW) { return false; }
    int val = Maze[y][x];
    return val != 1 && val != 3;
}

int GetWeight(int x, int y) {
    int val = Maze[y][x];
    switch (val) {
    case 0: case -1: case -2: return 1;
    case 2:  return 3;
    case 3:  return 5;
    default: return 1;
    }
}

using P = pair<int, Point>;
struct Compare { bool operator()(const P& a, const P& b) { return a.first > b.first; } };

vector<Point> FindPathBFS() {
    vector<vector<bool>> visited(ROW, vector<bool>(COL, false));
    map<Point, Point> parent;
    queue<Point> q;
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    q.push(startPoint);
    visited[startPoint.y][startPoint.x] = true;

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();
        if (curr == endPoint) { return ReconstructPath(parent, curr); }
        for (int i = 0; i < 4; i++) {
            int newX = curr.x + dx[i];
            int newY = curr.y + dy[i];
            if (isTraversable(newX, newY) && !visited[newY][newX]) {
                visited[newY][newX] = true;
                parent[{newX, newY}] = curr;
                q.push({ newX, newY });
            }
        }
    }
    return {};
}

vector<Point> FindPathDFS() {
    vector<vector<bool>> visited(ROW, vector<bool>(COL, false));
    map<Point, Point> parent;
    stack<Point> s;
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    s.push(startPoint);
    visited[startPoint.y][startPoint.x] = true;

    while (!s.empty()) {
        Point curr = s.top();
        s.pop();
        if (curr == endPoint) { return ReconstructPath(parent, curr); }
        for (int i = 0; i < 4; i++) {
            int newX = curr.x + dx[i];
            int newY = curr.y + dy[i];
            if (isTraversable(newX, newY) && !visited[newY][newX]) {
                visited[newY][newX] = true;
                parent[{newX, newY}] = curr;
                s.push({ newX, newY });
            }
        }
    }
    return {};
}

vector<Point> FindPathDijkstra() {
    map<Point, int> dist;
    map<Point, Point> parent;
    priority_queue<P, vector<P>, Compare> pq;
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            dist[{j, i}] = numeric_limits<int>::max();
        }
    }
    int dx[] = { 0, 0, -1, 1 };
    int dy[] = { -1, 1, 0, 0 };
    dist[startPoint] = 0;
    pq.push({ 0, startPoint });

    while (!pq.empty()) {
        int d = pq.top().first;
        Point curr = pq.top().second;
        pq.pop();
        if (d > dist[curr]) continue;
        if (curr == endPoint) { return ReconstructPath(parent, curr); }

        for (int i = 0; i < 4; i++) {
            int newX = curr.x + dx[i];
            int newY = curr.y + dy[i];
            Point neighbor = { newX, newY };

            if (isTraversable(newX, newY)) {
                int cost = GetWeight(newX, newY);
                int newDist = d + cost;
                if (newDist < dist[neighbor]) {
                    dist[neighbor] = newDist;
                    parent[neighbor] = curr;
                    pq.push({ newDist, neighbor });
                }
            }
        }
    }
    return {};
}


// ========= 4. 主函数 (Main) =========

int main()
{
    // 请在这里修改要加载的迷宫文件
    LoadMaze("C:\\Users\\123\\Desktop\\migong\\资源文件\\maze0.txt");

    InitWindow(COL * TILE, ROW * TILE + 40, "Maze Pathfinding Game (Press B/D/S)");

    // 加载纹理
    texFloor = LoadTexture("C:\\Users\\123\\Desktop\\migong\\资源文件\\floor.png");
    texWall = LoadTexture("C:\\Users\\123\\Desktop\\migong\\资源文件\\wall.png");
    texStart = LoadTexture("C:\\Users\\123\\Desktop\\migong\\资源文件\\start.png");
    texEnd = LoadTexture("C:\\Users\\123\\Desktop\\migong\\资源文件\\end.png");
    texGrass = LoadTexture("C:\\Users\\123\\Desktop\\migong\\资源文件\\grass.png");
    texLava = LoadTexture("C:\\Users\\123\\Desktop\\migong\\资源文件\\lava.png");

    // 计算路径
    pathBFS = FindPathBFS();
    pathDFS = FindPathDFS();
    pathDijkstra = FindPathDijkstra();

    int pathMode = 0;
    string modeText = "Current: No path selected";

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // ----------------------------------------------------
        // ----- 核心输入处理：使用 B/D/S 字母键 -----
        // ----------------------------------------------------
        int key = GetKeyPressed();

        if (key == KEY_B) { // B for BFS (Breadth-First Search)
            pathMode = 1;
            modeText = "Current: B - BFS (Shortest Unweighted Path - Blue)";
        }
        else if (key == KEY_D) { // D for DFS (Depth-First Search)
            pathMode = 2;
            modeText = "Current: D - DFS (Arbitrary Path - Green)";
        }
        else if (key == KEY_S) { // S for Shortest (Dijkstra's Algorithm)
            pathMode = 3;
            modeText = "Current: S - Dijkstra (Shortest Weighted Path - Red)";
        }
        else if (key == KEY_C) { // 0 to clear
            pathMode = 0;
            modeText = "Current: No path selected";
        }
        // ----------------------------------------------------

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawMaze();

        // 绘制路径
        switch (pathMode) {
        case 1:
            DrawPath(pathBFS, { 0, 121, 241, 150 }); // 蓝色
            break;
        case 2:
            DrawPath(pathDFS, { 0, 228, 48, 150 });  // 绿色
            break;
        case 3:
            DrawPath(pathDijkstra, { 255, 0, 0, 150 }); // 红色
            break;
        }

        // 绘制 UI 提示 (英文)
        DrawRectangle(0, ROW * TILE, COL * TILE, 40, BLACK);
        DrawText(modeText.c_str(), 10, ROW * TILE + 10, 20, WHITE);
        // 更新提示信息
        DrawText("Press B/D/S to switch | 0 to clear | ESC to exit", COL * TILE - 500, ROW * TILE + 10, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(texFloor);
    UnloadTexture(texWall);
    UnloadTexture(texStart);
    UnloadTexture(texEnd);
    UnloadTexture(texGrass);
    UnloadTexture(texLava);
    CloseWindow();
    return 0;
}