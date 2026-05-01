#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

// ─────────────────────────────────────────────
//  Data structure
// ─────────────────────────────────────────────
struct Chapter {
    string         name;
    int            time;
    int            marks;
    vector<string> prerequisites;
};

// ─────────────────────────────────────────────
//  Build name -> index map
// ─────────────────────────────────────────────
map<string, int> buildIndex(const vector<Chapter>& chapters) {
    map<string, int> idx;
    for (int i = 0; i < (int)chapters.size(); i++)
        idx[chapters[i].name] = i;
    return idx;
}

// ─────────────────────────────────────────────
//  DFS to collect all nodes reachable from src
// ─────────────────────────────────────────────
set<int> reachable(int src, const vector<vector<int> >& adj) {
    set<int>    visited;
    vector<int> stack;
    stack.push_back(src);
    while (!stack.empty()) {
        int u = stack.back();
        stack.pop_back();
        for (int i = 0; i < (int)adj[u].size(); i++) {
            int v = adj[u][i];
            if (visited.find(v) == visited.end()) {
                visited.insert(v);
                stack.push_back(v);
            }
        }
    }
    return visited;
}

// ─────────────────────────────────────────────
//  Transitive reduction
//  Removes edge A->B if B is already reachable
//  from A through another path — keeps the DAG
//  minimal with no redundant prerequisite links
// ─────────────────────────────────────────────
void transitiveReduction(vector<Chapter>& chapters) {
    map<string, int>     idx = buildIndex(chapters);
    int                  n   = (int)chapters.size();
    vector<vector<int> > adj(n);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < (int)chapters[i].prerequisites.size(); j++) {
            string p = chapters[i].prerequisites[j];
            if (idx.count(p))
                adj[idx[p]].push_back(i);
        }

    for (int i = 0; i < n; i++) {
        vector<string> kept;
        for (int j = 0; j < (int)chapters[i].prerequisites.size(); j++) {
            string p   = chapters[i].prerequisites[j];
            if (!idx.count(p)) continue;
            int    pid = idx[p];

            // Collect what is reachable via all OTHER prerequisites
            set<int> others;
            for (int k = 0; k < (int)chapters[i].prerequisites.size(); k++) {
                string q = chapters[i].prerequisites[k];
                if (q == p || !idx.count(q)) continue;
                set<int> r = reachable(idx[q], adj);
                others.insert(r.begin(), r.end());
            }
            // Keep p only if NOT reachable through others
            if (others.find(pid) == others.end())
                kept.push_back(p);
        }
        chapters[i].prerequisites = kept;
    }
}

// ─────────────────────────────────────────────
//  14 Pathological seed cases
// ─────────────────────────────────────────────
vector<Chapter> buildPathologicalCases() {
    vector<Chapter> d;
    Chapter ch;

    // 1. Eviction Bug Trap
    ch.name = "E_Prereq"; ch.time = 2; ch.marks = 5;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "E_Adv"; ch.time = 3; ch.marks = 100;
    ch.prerequisites.clear(); ch.prerequisites.push_back("E_Prereq"); d.push_back(ch);

    ch.name = "E_Indep"; ch.time = 2; ch.marks = 40;
    ch.prerequisites.clear(); d.push_back(ch);

    // 2. Diamond Problem
    ch.name = "D_Root"; ch.time = 2; ch.marks = 10;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "D_PathA"; ch.time = 2; ch.marks = 10;
    ch.prerequisites.clear(); ch.prerequisites.push_back("D_Root"); d.push_back(ch);

    ch.name = "D_PathB"; ch.time = 2; ch.marks = 10;
    ch.prerequisites.clear(); ch.prerequisites.push_back("D_Root"); d.push_back(ch);

    ch.name = "D_Junct"; ch.time = 1; ch.marks = 100;
    ch.prerequisites.clear();
    ch.prerequisites.push_back("D_PathA");
    ch.prerequisites.push_back("D_PathB");
    d.push_back(ch);

    // 3. Deep Chain (10 nodes)
    ch.name = "C_0"; ch.time = 1; ch.marks = 10;
    ch.prerequisites.clear(); d.push_back(ch);
    for (int i = 1; i < 10; i++) {
        ch.name  = "C_" + to_string(i);
        ch.time  = 1;
        ch.marks = 10 + i;
        ch.prerequisites.clear();
        ch.prerequisites.push_back("C_" + to_string(i - 1));
        d.push_back(ch);
    }

    // 4. High Fan-In
    for (int i = 1; i <= 5; i++) {
        ch.name  = "F_Indep_" + to_string(i);
        ch.time  = 1; ch.marks = 10;
        ch.prerequisites.clear(); d.push_back(ch);
    }
    ch.name = "F_Adv"; ch.time = 2; ch.marks = 200;
    ch.prerequisites.clear();
    ch.prerequisites.push_back("F_Indep_1");
    ch.prerequisites.push_back("F_Indep_2");
    ch.prerequisites.push_back("F_Indep_3");
    ch.prerequisites.push_back("F_Indep_4");
    ch.prerequisites.push_back("F_Indep_5");
    d.push_back(ch);

    // 5. Value-Heavy Leaf vs Long Chain
    ch.name = "V_Leaf"; ch.time = 10; ch.marks = 1000;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "V_Chain0"; ch.time = 1; ch.marks = 10;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "V_Chain1"; ch.time = 1; ch.marks = 2000;
    ch.prerequisites.clear(); ch.prerequisites.push_back("V_Chain0"); d.push_back(ch);

    // 6. Zero-Value Prerequisite
    ch.name = "Z_Prereq"; ch.time = 3; ch.marks = 0;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "Z_Adv"; ch.time = 1; ch.marks = 500;
    ch.prerequisites.clear(); ch.prerequisites.push_back("Z_Prereq"); d.push_back(ch);

    // 7. Time-Heavy Chain
    ch.name = "T_Root"; ch.time = 10; ch.marks = 50;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "T_Adv"; ch.time = 10; ch.marks = 50;
    ch.prerequisites.clear(); ch.prerequisites.push_back("T_Root"); d.push_back(ch);

    // 8. Binary Tree depth 2
    ch.name = "B_Root"; ch.time = 1; ch.marks = 5;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "B_L"; ch.time = 1; ch.marks = 10;
    ch.prerequisites.clear(); ch.prerequisites.push_back("B_Root"); d.push_back(ch);

    ch.name = "B_R"; ch.time = 1; ch.marks = 10;
    ch.prerequisites.clear(); ch.prerequisites.push_back("B_Root"); d.push_back(ch);

    ch.name = "B_LL"; ch.time = 1; ch.marks = 20;
    ch.prerequisites.clear(); ch.prerequisites.push_back("B_L"); d.push_back(ch);

    ch.name = "B_LR"; ch.time = 1; ch.marks = 20;
    ch.prerequisites.clear(); ch.prerequisites.push_back("B_L"); d.push_back(ch);

    ch.name = "B_RL"; ch.time = 1; ch.marks = 20;
    ch.prerequisites.clear(); ch.prerequisites.push_back("B_R"); d.push_back(ch);

    ch.name = "B_RR"; ch.time = 1; ch.marks = 20;
    ch.prerequisites.clear(); ch.prerequisites.push_back("B_R"); d.push_back(ch);

    // 9. Independent Swarm
    for (int i = 0; i < 5; i++) {
        ch.name  = "S_Indep_" + to_string(i);
        ch.time  = 2; ch.marks = 15;
        ch.prerequisites.clear(); d.push_back(ch);
    }

    // 10. Twin Paradox
    ch.name = "TW_A0"; ch.time = 2; ch.marks = 20;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "TW_A1"; ch.time = 2; ch.marks = 20;
    ch.prerequisites.clear(); ch.prerequisites.push_back("TW_A0"); d.push_back(ch);

    ch.name = "TW_B0"; ch.time = 2; ch.marks = 20;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "TW_B1"; ch.time = 2; ch.marks = 20;
    ch.prerequisites.clear(); ch.prerequisites.push_back("TW_B0"); d.push_back(ch);

    // 11. Overlapping Diamonds
    ch.name = "OD_Root"; ch.time = 1; ch.marks = 5;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "OD_A"; ch.time = 1; ch.marks = 5;
    ch.prerequisites.clear(); ch.prerequisites.push_back("OD_Root"); d.push_back(ch);

    ch.name = "OD_B"; ch.time = 1; ch.marks = 5;
    ch.prerequisites.clear(); ch.prerequisites.push_back("OD_Root"); d.push_back(ch);

    ch.name = "OD_C"; ch.time = 1; ch.marks = 5;
    ch.prerequisites.clear(); ch.prerequisites.push_back("OD_Root"); d.push_back(ch);

    ch.name = "OD_J1"; ch.time = 1; ch.marks = 50;
    ch.prerequisites.clear();
    ch.prerequisites.push_back("OD_A");
    ch.prerequisites.push_back("OD_B");
    d.push_back(ch);

    ch.name = "OD_J2"; ch.time = 1; ch.marks = 50;
    ch.prerequisites.clear();
    ch.prerequisites.push_back("OD_B");
    ch.prerequisites.push_back("OD_C");
    d.push_back(ch);

    // 12. Reversal Trap
    ch.name = "R_Root"; ch.time = 10; ch.marks = 1;
    ch.prerequisites.clear(); d.push_back(ch);

    ch.name = "R_Adv"; ch.time = 1; ch.marks = 500;
    ch.prerequisites.clear(); ch.prerequisites.push_back("R_Root"); d.push_back(ch);

    // 13. High-Cost Isolated
    ch.name = "H_Iso"; ch.time = 20; ch.marks = 400;
    ch.prerequisites.clear(); d.push_back(ch);

    // 14. Star Graph
    ch.name = "Star_Core"; ch.time = 2; ch.marks = 10;
    ch.prerequisites.clear(); d.push_back(ch);
    for (int i = 0; i < 5; i++) {
        ch.name  = "Star_Spoke_" + to_string(i);
        ch.time  = 1; ch.marks = 20;
        ch.prerequisites.clear();
        ch.prerequisites.push_back("Star_Core");
        d.push_back(ch);
    }

    return d;
}


void appendRandomNodes(vector<Chapter>& chapters, int target_n) {
    int existing = (int)chapters.size();
    if (target_n <= existing) return;

    int to_add       = target_n - existing;
    int n_levels     = 5;
    int nodes_per_level = (to_add > n_levels) ? (to_add / n_levels) : 1;

    // Running list of all available names (seeds + newly created)
    vector<string> allNames;
    for (int i = 0; i < (int)chapters.size(); i++)
        allNames.push_back(chapters[i].name);

    for (int i = 0; i < to_add; i++) {
        int level = i / nodes_per_level;
        if (level >= n_levels) level = n_levels - 1;

        Chapter ch;
        ch.name  = "Node_" + to_string(existing + i);
        ch.marks = 10 + rand() % 41;   // 10 to 50
        ch.time  =  1 + rand() % 10;   // 1  to 10

        if (level > 0 && !allNames.empty()) {
            // Number of parents: weights [0.1, 0.4, 0.3, 0.2] for 1,2,3,4
            int r = rand() % 10;
            int num_parents;
            if      (r < 1) num_parents = 1;
            else if (r < 5) num_parents = 2;
            else if (r < 8) num_parents = 3;
            else            num_parents = 4;

            int pool_size = (int)allNames.size();
            if (num_parents > pool_size) num_parents = pool_size;

            // Fisher-Yates shuffle on a copy, pick first num_parents
            vector<string> pool = allNames;
            for (int j = pool_size - 1; j > 0; j--) {
                int k = rand() % (j + 1);
                swap(pool[j], pool[k]);
            }

            set<string> chosen;
            for (int j = 0; j < num_parents; j++)
                chosen.insert(pool[j]);

            for (set<string>::iterator it = chosen.begin(); it != chosen.end(); ++it)
                ch.prerequisites.push_back(*it);
        }

        chapters.push_back(ch);
        allNames.push_back(ch.name);
    }
}

// ─────────────────────────────────────────────
//  Write CSV — columns: Name,Time,Marks,Prerequisites
// ─────────────────────────────────────────────
void writeCSV(const string& filename, const vector<Chapter>& chapters) {
    ofstream file(filename.c_str());
    if (!file.is_open()) {
        cerr << "Error: could not open " << filename << " for writing.\n";
        return;
    }

    file << "Name,Time,Marks,Prerequisites\n";
    for (int i = 0; i < (int)chapters.size(); i++) {
        const Chapter& ch = chapters[i];
        file << ch.name << "," << ch.time << "," << ch.marks << ",";
        for (int j = 0; j < (int)ch.prerequisites.size(); j++) {
            file << ch.prerequisites[j];
            if (j != (int)ch.prerequisites.size() - 1) file << ";";
        }
        file << "\n";
    }

    file.close();
    cout << "Generated: " << filename << "  (" << chapters.size() << " chapters)\n";
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main() {
    srand((unsigned int)time(NULL));

    int N;
    cout << "Enter number of chapters (N): ";
    cin  >> N;

    if (N < 1) {
        cerr << "N must be at least 1.\n";
        return 1;
    }

    // Step 1: embed all 14 pathological cases (61 chapters)
    vector<Chapter> chapters = buildPathologicalCases();
    int seed_count = (int)chapters.size();
    cout << "Embedded " << seed_count << " pathological seed chapters.\n";

    // Step 2: fill remaining slots with random DAG nodes
    if (N > seed_count) {
        appendRandomNodes(chapters, N);
    } else if (N < seed_count) {
        cout << "Warning: N=" << N << " is less than the " << seed_count
             << " seed chapters. Truncating to first " << N << ".\n";
        chapters.resize(N);
    }

    // Step 3: remove redundant prerequisite edges
    transitiveReduction(chapters);

    // Step 4: write output file named syllabus_N<N>.csv
    string filename = "syllabus_N" + to_string(N) + ".csv";
    writeCSV(filename, chapters);

    return 0;
}