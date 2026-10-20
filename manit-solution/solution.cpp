#include <fstream> 
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <array>
#include <map>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if __ARM_NEON
#include <arm_neon.h>
#endif


// to unordered hash with string view 
struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};


int floor_div(int a, int b) {
    int q = a / b;
    if (a % b != 0 && a < 0) --q;
    return q;
}

std::string_view format_tenths_better(char* buf, int buf_size, int t) {
    char* start = buf;
    if (t < 0) {
        *start++ = '-';
    }

    unsigned int abs_val = t < 0 ? -(unsigned int) t : (unsigned int) t;

    auto [ptr, ec] = std::to_chars(start, buf + buf_size - 2, abs_val/10);
    *ptr++ = '.';
    *ptr++ = '0' + abs_val % 10;
    return std::string_view(buf, ptr-buf);
}

int main() {
    std::unordered_map<std::string, std::array<int, 4>, string_hash, std::equal_to<>> dict;
    dict.reserve(10000);

    std::string filepath = "../data/measurements.txt";

    int fd = open(filepath.c_str(), O_RDONLY);

    struct stat sb;
    fstat(fd, &sb);
    size_t length = sb.st_size;

    void* addr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);

    close(fd);

    char* file_data = static_cast<char*>(addr);
    std::string_view view(file_data, length);

    size_t start = 0;
    while (start < view.length()) {
        size_t end = view.find('\n', start);
        std::string_view line = view.substr(start, end - start);

        int size = line.length();

        int pos = std::string_view(line).find(';');
        std::string_view name = std::string_view(line).substr(0, pos);

        int actualNum = 0;
        int mult = 1;

        for (int i = size-1; i > pos; i--) {
            if (line[i] == '-') {
                actualNum *= -1;
                continue;
            }
            else if (line[i] == '.') {
                i--;
            }
            actualNum += (line[i] - '0') * mult;
            mult *= 10;
        }

        auto it = dict.find(name);
        if (it == dict.end()) {
            it = dict.emplace(std::string(name), std::array<int, 4>{}).first;
        }

        if (it->second[3] == 0) {
            it->second[0] = actualNum;
            it->second[2] = actualNum;
        }        

        if (actualNum < it->second[0]) it->second[0] = actualNum; // min 
        if (actualNum > it->second[2]) it->second[2] = actualNum; // max
        it->second[1]+= actualNum; // sum (for mean later)
        it->second[3]++; // count


        if (end == std::string_view::npos) {
            break;
        }
    
        // Move past the newline character for the next iteration
        start = end + 1;
    }








    // output
    std::cout << "{";

    std::map<std::string, std::array<int, 4>> dict_ordered(dict.begin(), dict.end());

    char buf[32];
    int buf_size = 32;

    int index = 0;
    for (auto& [name, math] : dict_ordered) {
        if (index++ != 0) std::cout << ", ";

        std::string_view a = format_tenths_better(buf, buf_size, math[0]);
        std::cout << name << "=" << a;

        std::string_view b = format_tenths_better(
            buf, buf_size, floor_div(2 * math[1] + math[3], 2 * math[3])
        );
        std::cout << "/" << b; 

        std::string_view c = format_tenths_better(buf, buf_size, math[2]);
        std::cout << "/" << c; 

    }
    std::cout << "}" << std::endl;

    return 0;
}
