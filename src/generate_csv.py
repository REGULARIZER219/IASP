import random
import csv
import os
import shutil
from google.colab import files

class Chapter:
    def __init__(self, name, marks, time):
        self.name = name
        self.marks = marks
        self.time = time
        self.prereqs = []

    def add_prereq(self, p_name):
        if p_name not in self.prereqs:
            self.prereqs.append(p_name)

def get_transitive_reduction(chapters):
    name_map = {c.name: c for c in chapters}

    for node in chapters:
        indirect = set()
        for p_name in node.prereqs:
            parent = name_map[p_name]
            for pp_name in parent.prereqs:
                indirect.add(pp_name)

        node.prereqs = [p for p in node.prereqs if p not in indirect]

    return chapters

def generate_mega_syllabus(n, num_levels=5):
    chapters = []
    nodes_per_level = max(1, n // num_levels)

    level_map = {}

    for i in range(n):
        level = min(i // nodes_per_level, num_levels - 1)
        name = f"Node_{i}"
        marks = random.randint(10, 50)
        time = random.randint(1, 10)

        chapter = Chapter(name, marks, time)
        chapters.append(chapter)
        level_map[name] = level

    for c in chapters:
        lvl = level_map[c.name]

        if lvl > 0:
            pool = [x for x in chapters if level_map[x.name] < lvl]
            if not pool:
                continue

            num_reqs = random.choices([1, 2, 3, 4], weights=[0.1, 0.4, 0.3, 0.2])[0]
            parents = random.sample(pool, min(len(pool), num_reqs))

            for p in parents:
                c.add_prereq(p.name)

    return get_transitive_reduction(chapters)

def run_pipeline(sizes):
    base_dir = "mega_dataset"

    if os.path.exists(base_dir):
        shutil.rmtree(base_dir)
    os.makedirs(base_dir)

    for n in sizes:
        random.seed(42)
        data = generate_mega_syllabus(n)

        with open(os.path.join(base_dir, f"syllabus_N{n}.csv"), 'w', newline='') as f:
            w = csv.writer(f)
            w.writerow(['name', 'time', 'marks', 'prerequisites'])

            for c in data:
                w.writerow([
                    c.name,
                    c.time,
                    c.marks,
                    ';'.join(c.prereqs)
                ])

    shutil.make_archive(base_dir, 'zip', base_dir)
    files.download(f"{base_dir}.zip")

if __name__ == "__main__":
    run_pipeline([10, 50, 100, 500, 1000, 5000])