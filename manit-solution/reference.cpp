// Naive, exact 1BRC solution.
// Usage: ./reference < file.txt     or     ./reference file.txt
#include <climits>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

struct Stat { long long mn = LLONG_MAX, mx = LLONG_MIN, sum = 0, n = 0; };

// "-12.3" -> -123
static long long parse_tenths(const char* p, const char* end) {
    bool neg = (*p == '-');
    if (neg) ++p;
    long long v = 0;
    for (; p < end; ++p)
        if (*p != '.') v = v * 10 + (*p - '0');
    return neg ? -v : v;
}

// floor(a / b) for b > 0
static long long floor_div(long long a, long long b) {
    long long q = a / b;
    if (a % b != 0 && a < 0) --q;
    return q;
}

static void print_tenths(long long t) {
    if (t < 0) { std::putchar('-'); t = -t; }
    std::printf("%lld.%lld", t / 10, t % 10);
}

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);
    std::ifstream file;
    if (argc > 1) {
        file.open(argv[1]);
        if (!file) { std::perror(argv[1]); return 1; }
    }
    std::istream& in = (argc > 1) ? static_cast<std::istream&>(file) : std::cin;

    std::map<std::string, Stat> stats;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        auto semi = line.find(';');
        if (semi == std::string::npos) continue;

        long long t = parse_tenths(line.data() + semi + 1, line.data() + line.size());
        Stat& s = stats[line.substr(0, semi)];
        if (t < s.mn) s.mn = t;
        if (t > s.mx) s.mx = t;
        s.sum += t;
        s.n++;
    }

    std::putchar('{');
    bool first = true;
    for (const auto& [name, s] : stats) {
        if (!first) std::fputs(", ", stdout);
        first = false;
        std::printf("%s=", name.c_str());
        print_tenths(s.mn);
        std::putchar('/');
        print_tenths(floor_div(2 * s.sum + s.n, 2 * s.n));  // mean, round half up
        std::putchar('/');
        print_tenths(s.mx);
    }
    std::fputs("}\n", stdout);
}
