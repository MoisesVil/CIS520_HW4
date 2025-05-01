#!/usr/bin/env python3
# Script to analyze performance test results

import os
import csv
import matplotlib.pyplot as plt
import numpy as np
import sys

csv_file = "performance_data/summary.csv"
if not os.path.exists(csv_file):
    print(f"Error: Could not find {csv_file}")
    print("Please run the performance tests first or check the file path.")
    sys.exit(1)

data = []
with open(csv_file, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        data.append(row)

# Group by thread count
thread_data = {}
for row in data:
    thread_count = int(row['Threads'])
    if thread_count not in thread_data:
        thread_data[thread_count] = []
    thread_data[thread_count].append(row)

def parse_time(t):
    try:
        if ':' in t:
            mins, secs = t.split(':')
            return float(mins) * 60 + float(secs)
        return float(t)
    except Exception as e:
        print(f"Warning: Could not parse time value '{t}' — {e}")
        return 0.0

# Calculate average and standard deviation for each thread count
summary = []
for thread_count, rows in thread_data.items():

    elapsed_times = [parse_time(row['Elapsed_Time(s)']) for row in rows]

    memory_usages = [int(row['Memory(KB)']) for row in rows]

    avg_time = np.mean(elapsed_times)
    std_time = np.std(elapsed_times)
    avg_memory = np.mean(memory_usages)

    summary.append({
        'thread_count': thread_count,
        'avg_time': avg_time,
        'std_time': std_time,
        'avg_memory': avg_memory
    })

# Sort by thread count
summary.sort(key=lambda x: x['thread_count'])

# Calculate speedup and efficiency
if len(summary) > 0:
    base_time = next((item['avg_time'] for item in summary if item['thread_count'] == 1), None)
    if base_time is None and len(summary) > 0:
        base_time = summary[0]['avg_time']  # Use first available if no single-thread result

    for item in summary:
        item['speedup'] = base_time / item['avg_time'] if item['avg_time'] > 0 else 0
        item['efficiency'] = item['speedup'] / item['thread_count'] * 100 if item['thread_count'] > 0 else 0

# Create output directory for plots
os.makedirs('plots', exist_ok=True)

# Generate plots if we have data
if len(summary) > 0:
    thread_counts = [item['thread_count'] for item in summary]
    avg_times = [item['avg_time'] for item in summary]
    std_times = [item['std_time'] for item in summary]
    speedups = [item['speedup'] for item in summary]
    efficiencies = [item['efficiency'] for item in summary]
    avg_memories = [item['avg_memory'] / 1024 for item in summary]

    # Plot 1: Execution Time
    plt.figure(figsize=(10, 6))
    plt.errorbar(thread_counts, avg_times, yerr=std_times, fmt='o-', capsize=5)
    plt.xlabel('Number of Threads')
    plt.ylabel('Execution Time (seconds)')
    plt.title('Average Execution Time vs Number of Threads')
    plt.grid(True)
    plt.savefig('plots/execution_time.png')

    # Plot 2: Speedup
    plt.figure(figsize=(10, 6))
    plt.plot(thread_counts, speedups, 'o-', label='Actual Speedup')
    plt.plot(thread_counts, thread_counts, '--', label='Ideal Linear Speedup')
    plt.xlabel('Number of Threads')
    plt.ylabel('Speedup')
    plt.title('Speedup vs Number of Threads')
    plt.grid(True)
    plt.legend()
    plt.savefig('plots/speedup.png')

    # Plot 3: Efficiency
    plt.figure(figsize=(10, 6))
    plt.plot(thread_counts, efficiencies, 'o-')
    plt.xlabel('Number of Threads')
    plt.ylabel('Efficiency (%)')
    plt.title('Efficiency vs Number of Threads')
    plt.grid(True)
    plt.savefig('plots/efficiency.png')

    # Plot 4: Memory Usage
    plt.figure(figsize=(10, 6))
    plt.plot(thread_counts, avg_memories, 'o-')
    plt.xlabel('Number of Threads')
    plt.ylabel('Memory Usage (MB)')
    plt.title('Memory Usage vs Number of Threads')
    plt.grid(True)
    plt.savefig('plots/memory_usage.png')

    print("Plots generated in 'plots' directory")

    # Print summary
    print("\nPerformance Summary:")
    print("-" * 80)
    print(f"{'Threads':<8} {'Avg Time(s)':<12} {'Speedup':<10} {'Efficiency(%)':<14} {'Memory(MB)':<10}")
    print("-" * 80)
    for item in summary:
        print(f"{item['thread_count']:<8} {item['avg_time']:<12.2f} {item['speedup']:<10.2f} {item['efficiency']:<14.2f} {item['avg_memory']/1024:<10.2f}")
else:
    print("No data available for analysis")
