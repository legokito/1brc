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


// to unordered hash with string view 
struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const        { return hash_type{}(str); }
    std::size_t operator()(std::string_view str) const   { return hash_type{}(str); }
    std::size_t operator()(std::string const& str) const { return hash_type{}(str); }
};


long floor_div(long a, long b) {
    long q = a / b;
    if (a % b != 0 && a < 0) --q;
    return q;
}


std::string format_tenths(long t) {
    std::string sign = t < 0 ? "-" : "";
    if (t < 0) t = -t;
    return sign + std::to_string(t / 10) + "." + std::to_string(t % 10);
}

int main() {
    std::unordered_map<std::string, std::array<long, 4>, string_hash, std::equal_to<>> dict;
    dict.reserve(10000);

    std::string filepath = "../data/measurements.txt";

    int fd = open(filepath.c_str(), O_RDONLY);
    // check correct

    struct stat sb;
    fstat(fd, &sb);
    size_t length = sb.st_size;
    // check correct

    void* addr = mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);

    close(fd);

    char* file_data = static_cast<char*>(addr);
    std::string_view view(file_data, length);

    
    size_t start = 0;
    while (start < view.length()) {

        size_t end = view.find('\n', start);
        std::string_view line = view.substr(start, end - start);

        int pos = std::string_view(line).find(';');
        int dotPos = std::string_view(line).find('.', pos + 1);
        std::string_view name = std::string_view(line).substr(0, pos);
        long actualNum = std::stol(std::string(line.substr(pos+1, dotPos-pos-1)) + std::string(line.substr(dotPos+1)));

        auto it = dict.find(name);
        if (it == dict.end()) {
            it = dict.emplace(std::string(name), std::array<long, 4>{}).first;
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

    std::cout << "{";
    std::string a;
    std::string b;
    std::string c;

    std::map<std::string, std::array<long, 4>> dict_ordered(dict.begin(), dict.end());

    int index = 0;
    for (auto& [name, math] : dict_ordered) {
        if (index++ != 0) std::cout << ", ";

        a = format_tenths(math[0]);
        b = format_tenths(
            floor_div(2 * math[1] + math[3], 2 * math[3])
        );
        c = format_tenths(math[2]);

        std::cout << name + "=" 
                    + a + "/" 
                    + b + "/" 
                    + c; 
    }
    std::cout << "}" << std::endl;

    return 0;
}
