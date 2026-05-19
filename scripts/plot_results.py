import csv
import matplotlib.pyplot as plt
from pathlib import Path

CSV_PATH = Path("results/maxflow_results_cpp.csv")
OUTPUT_PATH = Path("results/maxflow_results_cpp.png")

vertices = []
ford = []
edmonds = []
dinic = []

with CSV_PATH.open("r", encoding="utf-8") as file:
    reader = csv.DictReader(file)
    for row in reader:
        vertices.append(int(row["vertices"]))
        ford.append(float(row["ford_fulkerson"]))
        edmonds.append(float(row["edmonds_karp"]))
        dinic.append(float(row["dinic"]))

plt.figure(figsize=(12, 7))
plt.plot(vertices, ford, marker="o", label="Ford-Fulkerson")
plt.plot(vertices, edmonds, marker="o", label="Edmonds-Karp")
plt.plot(vertices, dinic, marker="o", label="Dinic")

plt.xlabel("Number of vertices")
plt.ylabel("Average running time, seconds")
plt.title("Maximum Flow Algorithms Benchmark")
plt.legend()
plt.grid(True)

OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
plt.savefig(OUTPUT_PATH, dpi=300)
plt.show()

print(f"Plot saved to: {OUTPUT_PATH}")
