#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <chrono>

using namespace std;
using namespace chrono;

struct Chapter {
    int id;
    string name;
    int time, marks;
    vector<string> prereq_names;
    vector<int> prereq_ids, children;
};

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────


static string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

vector<Chapter> readCSV(const string& f) {
    vector<Chapter> chs;
    ifstream file(f.c_str());
    if (!file.is_open()) {
        cout << "Error: Could not open file " << f << endl;
        return chs;
    }
    string line;
    getline(file, line); // skip header

    int id = 0;
    while (getline(file, line)) {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();

        istringstream ss(line);
        string tok;
        Chapter ch;
        ch.id = id++;

        getline(ss, ch.name, ',');
        ch.name = trim(ch.name);

        getline(ss, tok, ','); ch.time  = atoi(tok.c_str());
        getline(ss, tok, ','); ch.marks = atoi(tok.c_str());
        getline(ss, tok, ',');

        if (!tok.empty()) {
            istringstream ps(tok);
            string p;
            while (getline(ps, p, ';'))
                if (!trim(p).empty())
                    ch.prereq_names.push_back(trim(p)); // FIX: store trimmed names
        }
        chs.push_back(ch);
    }
    return chs;
}

void buildGraph(vector<Chapter>& chs) {
    map<string, int> idx;
    for (int i = 0; i < (int)chs.size(); i++) idx[chs[i].name] = i;

    for (int i = 0; i < (int)chs.size(); i++)
        for (auto& p : chs[i].prereq_names)
            if (idx.count(p)) {
                int pid = idx[p];
                chs[i].prereq_ids.push_back(pid);
                chs[pid].children.push_back(i);
            }
}

// FIX: returns empty vector and prints a warning if a cycle is detected
vector<int> topoSort(const vector<Chapter>& chs) {
    int n = chs.size();
    vector<int> indeg(n, 0);
    for (int i = 0; i < n; i++) indeg[i] = (int)chs[i].prereq_ids.size();

    queue<int> q;
    for (int i = 0; i < n; i++) if (indeg[i] == 0) q.push(i);

    vector<int> ord;
    while (!q.empty()) {
        int v = q.front(); q.pop();
        ord.push_back(v);
        for (int c : chs[v].children)
            if (--indeg[c] == 0) q.push(c);
    }

    if ((int)ord.size() != n) {
        cout << "Warning: Cycle detected in prerequisites! Results may be incorrect.\n";
    }
    return ord;
}

void printSequence(const string& name, const vector<Chapter>& chs,
                   const vector<int>& ord, const vector<bool>& chosen) {
    cout << name << " Sequence: ";
    bool first = true;
    for (int v : ord) {
        if (chosen[v]) {
            if (!first) cout << " -> ";
            cout << chs[v].name;
            first = false;
        }
    }
    if (first) cout << "None";
    cout << "\n";
}

// ─────────────────────────────────────────────
//  GREEDY
// ─────────────────────────────────────────────
// 
void greedySolve(const vector<Chapter>& chs, int W,
                 vector<bool>& chosen, int& tm, int& tt) {
    int n = chs.size();
    chosen.assign(n, false);
    tm = tt = 0;

    // Collect all ancestors (transitive prereqs) of a node
    // We do a simple DFS to find the full prereq closure
    auto closure = [&](int v) {
        vector<bool> visited(n, false);
        queue<int> bfs;
        bfs.push(v);
        while (!bfs.empty()) {
            int u = bfs.front(); bfs.pop();
            for (int p : chs[u].prereq_ids)
                if (!visited[p]) { visited[p] = true; bfs.push(p); }
        }
        return visited;
    };

    // Precompute bundled marks (chapter + all transitive prereqs)
    vector<int> bm(n), bt(n);
    vector<vector<bool>> cls(n);
    for (int i = 0; i < n; i++) {
        cls[i] = closure(i);
        bm[i] = chs[i].marks;
        bt[i] = chs[i].time;
        for (int j = 0; j < n; j++)
            if (cls[i][j]) { bm[i] += chs[j].marks; bt[i] += chs[j].time; }
    }

    vector<int> order(n);
    for (int i = 0; i < n; i++) order[i] = i;
    sort(order.begin(), order.end(), [&](int a, int b) {
        double r1 = (double)bm[a] / bt[a];
        double r2 = (double)bm[b] / bt[b];
        if (fabs(r1 - r2) > 1e-9) return r1 > r2;
        return bm[a] > bm[b];
    });

    int rem = W;
    bool progress = true;
    while (progress) {
        progress = false;
        for (int v : order) {
            if (chosen[v] || chs[v].time > rem) continue;

            // 
            int marginal = chs[v].time;
            bool prereqs_ok = true;
            for (int p : chs[v].prereq_ids) {
                if (!chosen[p]) {
                    
                    prereqs_ok = false;
                    break;
                }
            }
            if (!prereqs_ok) continue;
            (void)marginal;

            chosen[v] = true;
            rem -= chs[v].time;
            tm  += chs[v].marks;
            tt  += chs[v].time;
            progress = true;
        }
    }
}

// ─────────────────────────────────────────────
//  SAT-GATED DP
// ─────────────────────────────────────────────
// 
void satGatedDP(const vector<Chapter>& chs,
                const vector<int>& ord, int W,
                vector<bool>& chosen, int& max_marks) {
    int n = chs.size();

    
    vector<int>          dp(W + 1, -1);
    


    dp[0] = 0;

    dp[0] = 0;

    // sel[w] = which chapters are chosen to achieve dp[w]
    // Updated layer by layer (one item at a time, reverse weight scan)
    vector<vector<bool>> sel(W + 1, vector<bool>(n, false));

    for (int pos = 0; pos < (int)ord.size(); pos++) {
        int v = ord[pos];
        int t = chs[v].time, mk = chs[v].marks;

        // Reverse scan to avoid using v twice (0-1 knapsack)
        for (int w = W; w >= t; w--) {
            int prev = w - t;
            if (dp[prev] == -1) continue;

            
            bool ok = true;
            for (int p : chs[v].prereq_ids)
                if (!sel[prev][p]) { ok = false; break; }
            if (!ok) continue;

            int val = dp[prev] + mk;
            if (val > dp[w]) {
                dp[w] = val;
             
                sel[w] = sel[prev];
                sel[w][v] = true;
            }
        }
    }

    max_marks = 0;
    int best_w = 0;
    for (int w = 0; w <= W; w++)
        if (dp[w] > max_marks) { max_marks = dp[w]; best_w = w; }

    chosen = sel[best_w];
}

// ─────────────────────────────────────────────
//  FPTAS
// ─────────────────────────────────────────────

void fptasSolve(const vector<Chapter>& chs, const vector<int>& ord,
                int W, double eps,
                vector<bool>& chosen, int& tm, int& tt) {
    int n = chs.size();
    if (n == 0) return;

    int mx = 0;
    for (auto& c : chs) mx = max(mx, c.marks);

    double K = (eps * mx) / n;
    if (K < 1.0) K = 1.0;

    vector<int> sm(n);
    for (int i = 0; i < n; i++)
        sm[i] = max(1, (int)(chs[i].marks / K));

    int M = 0;
    for (int x : sm) M += x;

    // dp[m] = minimum time to achieve scaled-marks budget m (-1 = unreachable)
    vector<int>          dp(M + 1, -1);
    vector<vector<bool>> sel(M + 1, vector<bool>(n, false));
    dp[0] = 0;

    int m_ord = (int)ord.size();
    for (int pos = 0; pos < m_ord; pos++) {
        int v   = ord[pos];
        int t   = chs[v].time;
        int s   = sm[v];       // scaled marks (always >= 1 now)

        // Reverse scan (0-1 knapsack on marks dimension)
        for (int mk = M; mk >= s; mk--) {
            int prev = mk - s;
            if (dp[prev] == -1) continue;

            bool ok = true;
            for (int p : chs[v].prereq_ids)
                if (!sel[prev][p]) { ok = false; break; }
            if (!ok) continue;

            int nt = dp[prev] + t;
            if (nt > W) continue;

            // Minimise time for this marks budget
            if (dp[mk] == -1 || nt < dp[mk]) {
                dp[mk] = nt;
                sel[mk] = sel[prev];  
                sel[mk][v] = true;
            }
        }
    }

    // Find best reachable scaled-marks budget
    int best = 0;
    for (int mk = 0; mk <= M; mk++)
        if (dp[mk] != -1 && dp[mk] <= W) best = mk;

    chosen = sel[best];
    tm = tt = 0;
    for (int i = 0; i < n; i++)
        if (chosen[i]) { tm += chs[i].marks; tt += chs[i].time; }
}

// ─────────────────────────────────────────────
//  BITMASK (brute-force exact)
// ─────────────────────────────────────────────

int bitmapSolve(const vector<Chapter>& chs,
                const vector<int>& /*ord*/,
                int W, vector<bool>& chosen) {
    int n = chs.size();
    if (n > 30) {
        cout << "[Bitmask DP] Skipped: N=" << n << " exceeds safe limit (>30).\n";
        return -1;
    }

    long long N    = 1LL << n;
    int       best = 0;
    long long bmask = 0;

    for (long long mask = 0; mask < N; mask++) {
        int  total_time  = 0;
        int  total_marks = 0;
        bool ok          = true;

        for (int i = 0; i < n && ok; i++) {
            if (!(mask & (1LL << i))) continue;
            for (int p : chs[i].prereq_ids) {
                if (!(mask & (1LL << p))) { ok = false; break; }
            }
            if (!ok) break;
            total_time  += chs[i].time;
            total_marks += chs[i].marks;
        }

        if (!ok || total_time > W) continue;
        if (total_marks > best) { best = total_marks; bmask = mask; }
    }

    chosen.assign(n, false);
    for (int i = 0; i < n; i++)
        if (bmask & (1LL << i)) chosen[i] = true;

    return best;
}

// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────
int main() {
    string filename;
    int    W;
    double eps;

    cout << "Enter .csv datafile name: ";
    cin  >> filename;
    cout << "Enter weight (time budget): ";
    cin  >> W;
    cout << "Enter epsilon value for FPTAS (e.g., 0.01): ";
    cin  >> eps;

    auto chs = readCSV(filename);
    if (chs.empty()) return 1;

    buildGraph(chs);
    auto ord = topoSort(chs);
    int  n   = (int)chs.size();

    cout << "\nRunning Algorithms for N = " << n << " with Budget = " << W << "\n";
    cout << string(60, '-') << "\n";

    // --- GREEDY ---
    vector<bool> gc;
    int gm = 0, gt = 0;
    auto t1 = high_resolution_clock::now();
    greedySolve(chs, W, gc, gm, gt);
    auto t2 = high_resolution_clock::now();

    // --- SAT-DP ---
    vector<bool> sc(n, false);
    int sm = 0;
    auto t3 = high_resolution_clock::now();
    satGatedDP(chs, ord, W, sc, sm);
    auto t4 = high_resolution_clock::now();

    // --- FPTAS ---
    vector<bool> fc;
    int fm = 0, ft = 0;
    auto t5 = high_resolution_clock::now();
    fptasSolve(chs, ord, W, eps, fc, fm, ft);
    auto t6 = high_resolution_clock::now();

    // --- BITMASK ---
    vector<bool> bc(n, false);
    auto t7 = high_resolution_clock::now();
    int exact = bitmapSolve(chs, ord, W, bc);
    auto t8 = high_resolution_clock::now();

    cout << "\n=== OPTIMAL ANSWERS ===\n";
    cout << "Greedy : " << gm << " marks\n";
    cout << "SAT-DP : " << sm << " marks\n";
    cout << "FPTAS  : " << fm << " marks\n";
    if (exact != -1) cout << "Bitmap : " << exact << " marks\n";

    cout << "\n=== STUDY SEQUENCES ===\n";
    printSequence("Greedy", chs, ord, gc);
    printSequence("SAT-DP", chs, ord, sc);
    printSequence("FPTAS ", chs, ord, fc);
    if (exact != -1) printSequence("Bitmap", chs, ord, bc);

    cout << "\n=== RUNTIME (microseconds) ===\n";
    cout << "Greedy : " << duration_cast<microseconds>(t2 - t1).count() << " us\n";
    cout << "SAT-DP : " << duration_cast<microseconds>(t4 - t3).count() << " us\n";
    cout << "FPTAS  : " << duration_cast<microseconds>(t6 - t5).count() << " us\n";
    if (exact != -1)
        cout << "Bitmap : " << duration_cast<microseconds>(t8 - t7).count() << " us\n";

    if (exact != -1 && exact > 0) {
        cout << "\n=== ACCURACY ===\n";
        cout << "Greedy : " << (int)round(100.0 * gm / exact) << "%\n";
        cout << "SAT-DP : " << (int)round(100.0 * sm / exact) << "%\n";
        cout << "FPTAS  : " << (int)round(100.0 * fm / exact) << "%\n";
        cout << "Bitmap : 100%\n";
    }

    return 0;
}