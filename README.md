<h1 align="center">Prerequisite-Aware Study Scheduler</h1>

<p align="center">
  <strong>Solving the Precedence-Constrained Knapsack Problem (PCKP) for Academic Curriculum Optimisation</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/C++-17-00599C?style=flat&logo=c%2B%2B&logoColor=white" />
  <img src="https://img.shields.io/badge/Python-3.x-3776AB?style=flat&logo=python&logoColor=white" />
  <img src="https://img.shields.io/badge/Problem-NP--Hard-critical?style=flat" />
  <img src="https://img.shields.io/badge/Approximation-FPTAS-2ea44f?style=flat" />
  <img src="https://img.shields.io/badge/Status-Complete-success?style=flat" />
</p>

---

## Overview

Standard study scheduling treats chapters as independent tasks, a direct mapping to the classical 0/1 Knapsack Problem where you pick whatever gives the highest marks per hour.

Real syllabi don't work that way. Topics are interconnected as **Directed Acyclic Graphs (DAGs)**. You cannot study *Deep Learning* without first completing *Linear Algebra* and *Calculus*. Choosing an advanced chapter forces you to bring its entire prerequisite closure with it.

This project models that reality and answers:

> **Given a time budget before an exam, which chapters should you study to maximise marks, without violating any prerequisite?**

We implement four algorithms of increasing sophistication, document classical failure modes (State Eviction Bug, Diamond Problem), and empirically prove theoretical bounds through automated benchmarking.

---

## Algorithms

| Algorithm | Complexity | Optimal? | Notes |
|---|---|---|---|
| SAT-Gated Greedy | O(N log N) | No | Fast heuristic baseline |
| SAT-Gated DP | O(N · W) | Yes* | Susceptible to State Eviction Bug |
| Bitmask DP | O(2ᴺ · N) | Yes | Exact solver; limited to N ≤ 30 |
| FPTAS | O(N² / ε) | (1−ε)·OPT | Best balance of speed and accuracy |

### 1. SAT-Gated Greedy
Ranks chapters by local value density (Marks / Time) and validates topological chains before selection. Extremely fast but falls into **Local Traps**, it may choose cheap independent chapters over a longer prerequisite chain that ultimately yields more marks.

### 2. SAT-Gated DP
Standard 0/1 knapsack augmented with prerequisite parent counters. Efficient for moderate budgets but suffers from the **State Eviction Bug**, a 1D DP table cannot remember the full topological history of how a state was reached, causing some valid paths to be incorrectly pruned.

### 3. Bitmask DP (Exact Solver)
Evaluates all valid subsets using binary masks. Handles the **Diamond Problem** (shared prerequisites between chains) correctly via idempotent bitwise OR, preventing double-counting. Hits a hard RAM wall at N > 30, intentionally timed out in benchmarks to empirically demonstrate the limits of exact exponential search.

### 4. FPTAS
Scales the Marks axis by a factor K = (ε · max\_marks) / N, reducing the problem to a bounded Dual DP. Guarantees a solution within **(1 − ε)** of optimal in fully polynomial time. Never performs worse than Greedy, automatically takes the maximum of both results, ensuring stable output across all edge cases.

---

## Repository Structure

```
IASP_Project/
├── Dataset/
│   ├── pathological_14_cases.csv    # Crafted edge cases (Diamond, Eviction Trap, etc.)
│   ├── syllabus_N8.csv              # Small curated dataset for testing
│   ├── syllabus_N10.csv             # Small dataset for exact solving
│   ├── syllabus_N50.csv
│   ├── syllabus_N100.csv
│   ├── syllabus_N500.csv
│   ├── syllabus_N1000.csv 
│   └── syllabus_N5000.csv           # Large dataset for FPTAS scaling tests
│
├── src/
│   ├── study_scheduler.cpp          # Core C++ solver (interactive)
│   ├── study_scheduler.exe          # Executable file
│   └── generate_csv.py              # Mega_dataset Generator
│   └── generate_csv.cpp             # Generator for pathological datasets
│
├── IASP_Project_Report.pdf          # Full project report
├── IASP_Project_Presentation.pdf
└── README.md                        # You are here!
```

---

## Getting Started

### Prerequisites
- g++ with C++17 support

### 1. Run the Interactive C++ Solver

Manually test a dataset and get the full ordered study sequence:

```bash
cd src
g++ -O3 -std=c++17 study_scheduler.cpp -o study_scheduler
./study_scheduler
```

The program prompts for the dataset path, time budget, and epsilon value for FPTAS.
### 2. Generate Pathological Edge Cases

```
Reconstructs the 14 stress-test cases (Eviction Trap, High Fan-In, Diamond, etc.):

```bash
cd src
g++ generate_csv.cpp -o generate
./generate
```

---

## CSV Format

```
name,time,marks,prerequisites
Calculus,4,20,
Linear Algebra,3,15,
Probability,3,18,Calculus
Statistics,2,12,Probability;Linear Algebra
Machine Learning,5,30,Statistics
```

| Field | Description |
|---|---|
| name | Unique chapter name |
| time | Study time in hours |
| marks | Expected exam marks contribution |
| prerequisites | Semicolon-separated list of required chapters; blank if none |

---

## Key Findings

- **Greedy** is fast but unreliable on deep prerequisite chains, can miss up to 40% of optimal marks on pathological inputs.
- **SAT-DP** is theoretically optimal but the State Eviction Bug causes incorrect results on diamond-shaped dependency graphs without careful implementation.
- **Bitmask DP** is the correctness gold standard but computationally infeasible beyond N ≈ 25.
- **FPTAS** is the practical winner, polynomial runtime, tunable accuracy, and a guaranteed (1 − ε)·OPT lower bound.





---

## Concepts Covered

`Graph Theory` &nbsp;`DAG Traversal` &nbsp;`Topological Sort` &nbsp;`0/1 Knapsack` &nbsp;`Dynamic Programming` &nbsp;`Bitmask Enumeration` &nbsp;`FPTAS Design` &nbsp;`NP-Hard Approximation` &nbsp;`Algorithm Benchmarking`

---

## License

MIT
