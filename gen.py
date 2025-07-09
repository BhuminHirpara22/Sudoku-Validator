import sys
import subprocess
import matplotlib.pyplot as plt
import math

# --------------------------
# Sudoku Generator
# --------------------------
class SudokuGenerator:
    @staticmethod
    def generate(threadCount, gridSize, batchSize):
        """
        Generate a valid Sudoku grid and write to input.txt.
        The input file format is: 
          threadCount gridSize batchSize
          Then gridSize lines, each with gridSize space-separated numbers.
        (Note: gridSize must be a perfect square.)
        This generator uses a cyclic formula.
        """
        n = int(math.sqrt(gridSize))
        if n * n != gridSize:
            raise ValueError("gridSize must be a perfect square")
        
        rows = []
        for i in range(gridSize):
            row = [str(((n * (i % n) + (i // n) + j) % gridSize) + 1) for j in range(gridSize)]
            rows.append(" ".join(row))
        with open("input.txt", "w") as f:
            f.write(f"{threadCount} {gridSize} {batchSize}\n")
            f.write("\n".join(rows))


# --------------------------
# Experiment Runner
# --------------------------
class ExperimentRunner:
    @staticmethod
    def compile_programs():
        """Compile the four programs: CAS.cpp, TAS.cpp, BoundedCAS.cpp, sequential.cpp."""
        programs = {
            "CAS": "CAS.cpp",
            "TAS": "TAS.cpp",
            "BoundedCAS": "BoundedCAS.cpp",
            "sequential": "sequential.cpp"
        }
        for progName, filename in programs.items():
            flags = ["-lpthread", "-lm"] if progName != "sequential" else ["-lm"]
            cmd = ["g++", filename, "-o", progName] + flags
            print(f"Compiling {filename} as {progName} ...")
            subprocess.run(cmd, check=True)
    
    @staticmethod
    def run_program(program):
        """Run the given executable and extract metrics from output.txt.
           Returns a dictionary of metrics.
        """
        try:
            subprocess.run(["./" + program], check=True, capture_output=True, text=True)
            with open("output.txt", "r") as f:
                lines = f.readlines()
            metrics = {}
            for line in lines:
                line = line.strip()
                if line.startswith("The total time taken is"):
                    metrics["total"] = float(line.split("is")[1].split()[0])
            return metrics
        except Exception as e:
            print(f"Error running {program}: {e}")
            return None

    @classmethod
    def run_experiment(cls, programs, x_values, gen_input_func, repetitions=5):
        """Run experiment for each x in x_values.
           Returns a dict: {metric: {program: [(x, avg_value), ...], ...}, ...}
        """
        results = { "total": {p: [] for p in programs} }
        for x in x_values:
            gen_input_func(x)
            print(f"\nRunning tests for x = {x} ...")
            for prog in programs:
                print(f"Testing {prog}...", end=" ")
                collected = {}
                for _ in range(repetitions):
                    m = cls.run_program(prog)
                    if m:
                        for key, value in m.items():
                            collected[key] = collected.get(key, []) + [value]
                if "total" in collected and collected["total"]:
                    avg_total = sum(collected["total"]) / len(collected["total"])
                    results["total"][prog].append((x, avg_total))
                    print(f"Total: {avg_total:.2f}μs", end="; ")
                print()
        return results

# --------------------------
# Plotter
# --------------------------
class Plotter:
    @staticmethod
    def plot(data, xlabel, ylabel, title, filename):
        plt.figure(figsize=(10, 6))
        for prog, values in data.items():
            if not values:
                continue
            x = [v[0] for v in values]
            y = [v[1] for v in values]
            plt.plot(x, y, marker='o', linestyle='-', label=prog)
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.title(title)
        plt.legend()
        plt.grid(True)
        plt.savefig(filename)
        plt.close()

# --------------------------
# Experiment definitions
# --------------------------

def experiment1():
    """
    Experiment 1: Time vs. Sudoku Size (Fixed: 8 threads, batchSize = 20)
    X-axis: grid sizes from 10^2 x 10^2 to 100^2 x 100^2, i.e.,
           100x100, 400x400, 900x900, ..., 10000x10000.
    """
    programs = ["CAS", "TAS", "BoundedCAS", "sequential"]
    fixedThreads = 8
    fixedBatch = 20
    # x-values: 10,20,...,100; grid size = (x^2)
    sizes = [i*i for i in range(10, 101, 10)]
    
    def gen_input(size):
        with open("input.txt", "w") as f:
            f.write(f"{fixedThreads} {size} {fixedBatch}\n")
            n = int(math.sqrt(size))
            gridLines = []
            for i in range(size):
                row = [str(((n * (i % n) + (i // n) + j) % size) + 1) for j in range(size)]
                gridLines.append(" ".join(row))
            f.write("\n".join(gridLines))
    
    print("\nExperiment 1: Varying grid size")
    results = ExperimentRunner.run_experiment(programs, sizes, gen_input, repetitions=5)
    Plotter.plot(results["total"], "Grid Size", "Total Time (μs)", "Total Time vs. Grid Size", "exp1_total.png")
    # Plotter.plot(results["avg_entry"], "Grid Size", "Avg CS Entry (μs)", "Avg CS Entry vs. Grid Size", "exp1_avg_entry.png")
    # Plotter.plot(results["avg_exit"], "Grid Size", "Avg CS Exit (μs)", "Avg CS Exit vs. Grid Size", "exp1_avg_exit.png")
    # Plotter.plot(results["worst_entry"], "Grid Size", "Worst CS Entry (μs)", "Worst CS Entry vs. Grid Size", "exp1_worst_entry.png")
    # Plotter.plot(results["worst_exit"], "Grid Size", "Worst CS Exit (μs)", "Worst CS Exit vs. Grid Size", "exp1_worst_exit.png")

def experiment2():
    """
    Experiment 2: Time vs. TASk Increment (batchSize) (Fixed: grid size = 100, threads = 8)
    X-axis: batchSize values: 10, 20, 30, 40, 50.
    """
    programs = ["CAS", "TAS", "BoundedCAS", "sequential"]
    fixedThreads = 8
    fixedGridSize = 100  # 100x100 grid
    batchValues = [10, 20, 30, 40, 50]
    
    def gen_input(batch):
        with open("input.txt", "w") as f:
            f.write(f"{fixedThreads} {fixedGridSize} {batch}\n")
            n = int(math.sqrt(fixedGridSize))
            gridLines = []
            for i in range(fixedGridSize):
                row = [str(((n * (i % n) + (i // n) + j) % fixedGridSize) + 1) for j in range(fixedGridSize)]
                gridLines.append(" ".join(row))
            f.write("\n".join(gridLines))
    
    print("\nExperiment 2: Varying TASk increment (batchSize)")
    results = ExperimentRunner.run_experiment(programs, batchValues, gen_input, repetitions=5)
    Plotter.plot(results["total"], "Batch Size", "Total Time (μs)", "Total Time vs. Batch Size", "exp2_total.png")
    # Plotter.plot(results["avg_entry"], "Batch Size", "Avg CS Entry (μs)", "Avg CS Entry vs. Batch Size", "exp2_avg_entry.png")
    # Plotter.plot(results["avg_exit"], "Batch Size", "Avg CS Exit (μs)", "Avg CS Exit vs. Batch Size", "exp2_avg_exit.png")
    # Plotter.plot(results["worst_entry"], "Batch Size", "Worst CS Entry (μs)", "Worst CS Entry vs. Batch Size", "exp2_worst_entry.png")
    # Plotter.plot(results["worst_exit"], "Batch Size", "Worst CS Exit (μs)", "Worst CS Exit vs. Batch Size", "exp2_worst_exit.png")

def experiment3():
    """
    Experiment 3: Time vs. Number of Threads (Fixed: grid size = 100, batchSize = 20)
    X-axis: thread counts: 1, 2, 4, 8, 16, 32.
    """
    programs = ["CAS", "TAS", "BoundedCAS", "sequential"]
    fixedGridSize = 100
    fixedBatch = 20
    threadValues = [1, 2, 4, 8, 16, 32]
    
    def gen_input(tcount):
        with open("input.txt", "w") as f:
            f.write(f"{tcount} {fixedGridSize} {fixedBatch}\n")
            n = int(math.sqrt(fixedGridSize))
            gridLines = []
            for i in range(fixedGridSize):
                row = [str(((n * (i % n) + (i // n) + j) % fixedGridSize) + 1) for j in range(fixedGridSize)]
                gridLines.append(" ".join(row))
            f.write("\n".join(gridLines))
    
    print("\nExperiment 3: Varying thread count")
    results = ExperimentRunner.run_experiment(programs, threadValues, gen_input, repetitions=5)
    Plotter.plot(results["total"], "Thread Count", "Total Time (μs)", "Total Time vs. Thread Count", "exp3_total.png")
    # Plotter.plot(results["avg_entry"], "Thread Count", "Avg CS Entry (μs)", "Avg CS Entry vs. Thread Count", "exp3_avg_entry.png")
    # Plotter.plot(results["avg_exit"], "Thread Count", "Avg CS Exit (μs)", "Avg CS Exit vs. Thread Count", "exp3_avg_exit.png")
    # Plotter.plot(results["worst_entry"], "Thread Count", "Worst CS Entry (μs)", "Worst CS Entry vs. Thread Count", "exp3_worst_entry.png")
    # Plotter.plot(results["worst_exit"], "Thread Count", "Worst CS Exit (μs)", "Worst CS Exit vs. Thread Count", "exp3_worst_exit.png")

# --------------------------
# Main
# --------------------------
if __name__ == "__main__":
    ExperimentRunner.compile_programs()
    
    data1 = experiment1()
    data2 = experiment2()
    data3 = experiment3()
    
    print("Experiments completed. Check the generated plots.")