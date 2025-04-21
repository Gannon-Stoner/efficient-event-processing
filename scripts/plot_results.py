import json
import sys
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

plt.rcParams["figure.autolayout"] = True

if len(sys.argv) < 2:
    print("Provide at least one results file (JSON or latency CSV).")
    sys.exit(1)

json_path = None
latency_paths = []
for arg in sys.argv[1:]:
    p = Path(arg)
    if not p.is_file():
        raise FileNotFoundError(p)
    if p.suffix == ".json":
        json_path = p
    else:
        latency_paths.append(p)


# Throughput summary (Google Benchmark JSON)
if json_path:
    data = json.loads(json_path.read_text())
    rows = []
    for bm in data["benchmarks"]:
        if "aggregate_name" in bm:
            continue
        name = bm["name"]
        threads = 1
        if "/threads:" in name:
            threads = int(name.split("/threads:")[-1])
            name = name.split("/")[0]

        time_unit = bm.get("time_unit", "ns")
        real_time = bm["real_time"]
        scale = {"ns": 1, "us": 1e3, "ms": 1e6, "s": 1e9}[time_unit]
        ns_per_event = real_time * scale
        rows.append({
            "benchmark": name,
            "threads": threads,
            "iter": bm["iterations"],
            "ns_per_event": ns_per_event,
            "events_per_sec": 1e9 / ns_per_event,
        })

    df = pd.DataFrame(rows).sort_values(["benchmark", "threads"])
    print(df.to_string(index=False, formatters={
        "ns_per_event": "{:,.1f}".format,
        "events_per_sec": "{:,.0f}".format
    }))
    df.to_csv("summary_table.csv", index=False)

    # Bar chart
    for bench, grp in df.groupby("benchmark"):
        plt.figure()
        plt.bar(grp["threads"], grp["events_per_sec"])
        plt.xlabel("Worker Threads")
        plt.ylabel("Throughput (events/sec)")
        plt.title(f"{bench} – throughput vs threads")
        plt.xticks(grp["threads"])
        safe_name = bench.replace("/", "_")
        plt.savefig(f"throughput_{safe_name}.png", dpi=300)

    print("Saved summary_table.csv and throughput_*.png")

# Latency histograms + percentile chart
if latency_paths:
    p99_rows = []
    for csv in latency_paths:
        samples = np.loadtxt(csv)
        threads = int(csv.stem.split("_")[-1])  # latency_4 → 4
        # histogram
        plt.figure()
        plt.hist(samples, bins=100, log=True)
        plt.xlabel("Latency (ns)")
        plt.ylabel("Frequency (log scale)")
        plt.title(f"Latency histogram – {threads} thread(s)")
        plt.savefig(f"{csv.stem}_hist.png", dpi=300)

        p99 = np.percentile(samples, 99)
        p999 = np.percentile(samples, 99.9)
        p99_rows.append({"threads": threads, "p99": p99, "p999": p999})

    pct = pd.DataFrame(p99_rows).sort_values("threads")
    # line chart
    plt.figure()
    plt.plot(pct["threads"], pct["p99"], marker="o", label="p99")
    plt.plot(pct["threads"], pct["p999"], marker="o", label="p99.9")
    plt.xlabel("Worker Threads")
    plt.ylabel("Latency (ns)")
    plt.title("Tail latency vs threads")
    plt.legend()
    plt.savefig("latency_p99_vs_threads.png", dpi=300)

    print("Saved latency_*_hist.png and latency_p99_vs_threads.png")
