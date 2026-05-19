#include <iostream>
#include <vector>
#include <queue>
#include <random>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <set>
#include <string>

using namespace std;

// ============================================================
// Edge structure
// ============================================================

struct Edge {
    int to;       // destination vertex
    int rev;      // index of the reverse edge
    int capacity; // residual capacity

    Edge(int to, int rev, int capacity)
        : to(to), rev(rev), capacity(capacity) {}
};

// ============================================================
// Flow network class
// ============================================================

class FlowNetwork {
private:
    int vertexCount;
    vector<vector<Edge>> graph;

public:
    explicit FlowNetwork(int vertexCount)
        : vertexCount(vertexCount), graph(vertexCount) {}

    void addEdge(int from, int to, int capacity) {
        Edge forward(to, static_cast<int>(graph[to].size()), capacity);
        Edge backward(from, static_cast<int>(graph[from].size()), 0);

        graph[from].push_back(forward);
        graph[to].push_back(backward);
    }

    int getVertexCount() const {
        return vertexCount;
    }

    vector<vector<Edge>>& getGraph() {
        return graph;
    }

    const vector<vector<Edge>>& getGraph() const {
        return graph;
    }
};

// ============================================================
// Base class for maximum flow algorithms
// ============================================================

class MaxFlowSolver {
protected:
    FlowNetwork network;
    vector<vector<Edge>>& graph;
    int n;
    int source;
    int sink;

public:
    MaxFlowSolver(const FlowNetwork& originalNetwork, int source, int sink)
        : network(originalNetwork),
          graph(network.getGraph()),
          n(network.getVertexCount()),
          source(source),
          sink(sink) {}

    virtual int solve() = 0;

    virtual ~MaxFlowSolver() = default;
};

// ============================================================
// Ford-Fulkerson algorithm using DFS
// ============================================================

class FordFulkersonSolver : public MaxFlowSolver {
public:
    FordFulkersonSolver(const FlowNetwork& network, int source, int sink)
        : MaxFlowSolver(network, source, sink) {}

    int solve() override {
        int maxFlow = 0;

        while (true) {
            vector<bool> visited(n, false);
            int pushed = dfs(source, 1000000000, visited);

            if (pushed == 0) {
                break;
            }

            maxFlow += pushed;
        }

        return maxFlow;
    }

private:
    int dfs(int vertex, int flow, vector<bool>& visited) {
        if (vertex == sink) {
            return flow;
        }

        visited[vertex] = true;

        for (Edge& edge : graph[vertex]) {
            if (!visited[edge.to] && edge.capacity > 0) {
                int pushed = dfs(edge.to, min(flow, edge.capacity), visited);

                if (pushed > 0) {
                    edge.capacity -= pushed;
                    graph[edge.to][edge.rev].capacity += pushed;
                    return pushed;
                }
            }
        }

        return 0;
    }
};

// ============================================================
// Edmonds-Karp algorithm
// ============================================================

class EdmondsKarpSolver : public MaxFlowSolver {
public:
    EdmondsKarpSolver(const FlowNetwork& network, int source, int sink)
        : MaxFlowSolver(network, source, sink) {}

    int solve() override {
        int maxFlow = 0;

        while (true) {
            vector<int> parentVertex(n, -1);
            vector<int> parentEdge(n, -1);

            bool foundPath = bfs(parentVertex, parentEdge);

            if (!foundPath) {
                break;
            }

            int flow = findPathFlow(parentVertex, parentEdge);
            pushFlow(parentVertex, parentEdge, flow);

            maxFlow += flow;
        }

        return maxFlow;
    }

private:
    bool bfs(vector<int>& parentVertex, vector<int>& parentEdge) {
        queue<int> q;

        parentVertex[source] = source;
        q.push(source);

        while (!q.empty() && parentVertex[sink] == -1) {
            int vertex = q.front();
            q.pop();

            for (int i = 0; i < static_cast<int>(graph[vertex].size()); i++) {
                Edge& edge = graph[vertex][i];

                if (parentVertex[edge.to] == -1 && edge.capacity > 0) {
                    parentVertex[edge.to] = vertex;
                    parentEdge[edge.to] = i;
                    q.push(edge.to);

                    if (edge.to == sink) {
                        break;
                    }
                }
            }
        }

        return parentVertex[sink] != -1;
    }

    int findPathFlow(const vector<int>& parentVertex,
                     const vector<int>& parentEdge) {
        int flow = 1000000000;
        int current = sink;

        while (current != source) {
            int previous = parentVertex[current];
            int edgeIndex = parentEdge[current];

            flow = min(flow, graph[previous][edgeIndex].capacity);
            current = previous;
        }

        return flow;
    }

    void pushFlow(const vector<int>& parentVertex,
                  const vector<int>& parentEdge,
                  int flow) {
        int current = sink;

        while (current != source) {
            int previous = parentVertex[current];
            int edgeIndex = parentEdge[current];

            Edge& edge = graph[previous][edgeIndex];

            edge.capacity -= flow;
            graph[edge.to][edge.rev].capacity += flow;

            current = previous;
        }
    }
};

// ============================================================
// Dinic's algorithm
// ============================================================

class DinicSolver : public MaxFlowSolver {
private:
    vector<int> level;
    vector<int> ptr;

public:
    DinicSolver(const FlowNetwork& network, int source, int sink)
        : MaxFlowSolver(network, source, sink),
          level(n),
          ptr(n) {}

    int solve() override {
        int maxFlow = 0;

        while (bfsBuildLevels()) {
            fill(ptr.begin(), ptr.end(), 0);

            while (true) {
                int pushed = dfsPushFlow(source, 1000000000);

                if (pushed == 0) {
                    break;
                }

                maxFlow += pushed;
            }
        }

        return maxFlow;
    }

private:
    bool bfsBuildLevels() {
        fill(level.begin(), level.end(), -1);
        level[source] = 0;

        queue<int> q;
        q.push(source);

        while (!q.empty()) {
            int vertex = q.front();
            q.pop();

            for (const Edge& edge : graph[vertex]) {
                if (edge.capacity > 0 && level[edge.to] == -1) {
                    level[edge.to] = level[vertex] + 1;
                    q.push(edge.to);
                }
            }
        }

        return level[sink] != -1;
    }

    int dfsPushFlow(int vertex, int flow) {
        if (vertex == sink) {
            return flow;
        }

        while (ptr[vertex] < static_cast<int>(graph[vertex].size())) {
            Edge& edge = graph[vertex][ptr[vertex]];

            if (edge.capacity > 0 && level[edge.to] == level[vertex] + 1) {
                int pushed = dfsPushFlow(edge.to, min(flow, edge.capacity));

                if (pushed > 0) {
                    edge.capacity -= pushed;
                    graph[edge.to][edge.rev].capacity += pushed;
                    return pushed;
                }
            }

            ptr[vertex]++;
        }

        return 0;
    }
};

// ============================================================
// Random graph generator
// ============================================================

class GraphGenerator {
private:
    int maxCapacity;
    mt19937 rng;

public:
    explicit GraphGenerator(int maxCapacity = 100)
        : maxCapacity(maxCapacity), rng(random_device{}()) {}

    FlowNetwork generateComplexGraph(int vertexCount, int extraEdges) {
        FlowNetwork network(vertexCount);
        set<pair<int, int>> usedEdges;

        uniform_int_distribution<int> capacityDist(1, maxCapacity);
        uniform_int_distribution<int> vertexDist(0, vertexCount - 1);

        // Guaranteed path: 0 -> 1 -> 2 -> ... -> n - 1
        for (int i = 0; i < vertexCount - 1; i++) {
            int capacity = capacityDist(rng);
            network.addEdge(i, i + 1, capacity);
            usedEdges.insert({i, i + 1});
        }

        int added = 0;

        while (added < extraEdges) {
            int from = vertexDist(rng);
            int to = vertexDist(rng);

            if (from == to) {
                continue;
            }

            if (usedEdges.count({from, to})) {
                continue;
            }

            int capacity = capacityDist(rng);

            network.addEdge(from, to, capacity);
            usedEdges.insert({from, to});

            added++;
        }

        return network;
    }
};

// ============================================================
// Benchmark class
// ============================================================

class Benchmark {
private:
    vector<int> sizes;
    int testsPerSize;

    vector<double> fordTimes;
    vector<double> edmondsTimes;
    vector<double> dinicTimes;

    GraphGenerator generator;

public:
    Benchmark(const vector<int>& sizes, int testsPerSize)
        : sizes(sizes), testsPerSize(testsPerSize), generator(100) {}

    template <typename Solver>
    pair<double, int> measureTime(const FlowNetwork& network,
                                  int source,
                                  int sink) {
        auto start = chrono::high_resolution_clock::now();

        Solver solver(network, source, sink);
        int result = solver.solve();

        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> elapsed = end - start;

        return {elapsed.count(), result};
    }

    void run() {
        for (int n : sizes) {
            cout << "Testing graph with " << n << " vertices..." << endl;

            double totalFord = 0.0;
            double totalEdmonds = 0.0;
            double totalDinic = 0.0;

            for (int test = 1; test <= testsPerSize; test++) {
                cout << "  Test " << test << "/" << testsPerSize << endl;

                int extraEdges = n * 8;

                FlowNetwork network = generator.generateComplexGraph(n, extraEdges);

                int source = 0;
                int sink = n - 1;

                auto [fordTime, fordFlow] =
                    measureTime<FordFulkersonSolver>(network, source, sink);

                auto [edmondsTime, edmondsFlow] =
                    measureTime<EdmondsKarpSolver>(network, source, sink);

                auto [dinicTime, dinicFlow] =
                    measureTime<DinicSolver>(network, source, sink);

                if (!(fordFlow == edmondsFlow && edmondsFlow == dinicFlow)) {
                    cout << "Error: algorithms returned different results!" << endl;
                    cout << "Ford-Fulkerson: " << fordFlow << endl;
                    cout << "Edmonds-Karp:   " << edmondsFlow << endl;
                    cout << "Dinic:          " << dinicFlow << endl;
                    return;
                }

                totalFord += fordTime;
                totalEdmonds += edmondsTime;
                totalDinic += dinicTime;
            }

            double avgFord = totalFord / testsPerSize;
            double avgEdmonds = totalEdmonds / testsPerSize;
            double avgDinic = totalDinic / testsPerSize;

            fordTimes.push_back(avgFord);
            edmondsTimes.push_back(avgEdmonds);
            dinicTimes.push_back(avgDinic);

            cout << "Average results for " << n << " vertices:" << endl;
            cout << "  Ford-Fulkerson: " << avgFord << " sec" << endl;
            cout << "  Edmonds-Karp:   " << avgEdmonds << " sec" << endl;
            cout << "  Dinic:          " << avgDinic << " sec" << endl;
            cout << endl;
        }
    }

    void saveToCSV(const string& filename) {
        ofstream file(filename);

        file << "vertices,ford_fulkerson,edmonds_karp,dinic\n";

        for (int i = 0; i < static_cast<int>(sizes.size()); i++) {
            file << sizes[i] << ","
                 << fordTimes[i] << ","
                 << edmondsTimes[i] << ","
                 << dinicTimes[i] << "\n";
        }

        file.close();

        cout << "Results saved to file: " << filename << endl;
    }
};

// ============================================================
// Main function
// ============================================================

int main(int argc, char* argv[]) {
    vector<int> sizes = {
        50, 100, 150, 200, 250,
        300, 350, 400, 450, 500,
        600, 700, 800, 900, 1000,
        1200, 1400, 1600, 1800, 2000,
        2500, 3000, 3500, 4000
    };

    int testsPerSize = 100;

    if (argc >= 2) {
        testsPerSize = stoi(argv[1]);
    }

    Benchmark benchmark(sizes, testsPerSize);

    benchmark.run();
    benchmark.saveToCSV("results/maxflow_results_cpp.csv");

    return 0;
}
