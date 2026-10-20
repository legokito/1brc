#include <fstream> 
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <array>
#include <map>
#include <bit>
#include <charconv>
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

// politely interpolated from geofflangdale
// truly insane how this works, i'm blown away
// https://branchfree.org/2019/04/01/fitting-my-head-through-the-arm-holes-or-two-sequences-to-substitute-for-the-missing-pmovmskb-instruction-on-arm-neon
uint64_t neonmovemask_bulk(uint8x16_t p0, uint8x16_t p1, uint8x16_t p2, uint8x16_t p3) {
    const uint8x16_t bitmask = { 0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
                               0x01, 0x02, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80};
  uint8x16_t t0 = vandq_u8(p0, bitmask);
  uint8x16_t t1 = vandq_u8(p1, bitmask);
  uint8x16_t t2 = vandq_u8(p2, bitmask);
  uint8x16_t t3 = vandq_u8(p3, bitmask);
  uint8x16_t sum0 = vpaddq_u8(t0, t1);
  uint8x16_t sum1 = vpaddq_u8(t2, t3);
  sum0 = vpaddq_u8(sum0, sum1);
  sum0 = vpaddq_u8(sum0, sum0);
  return vgetq_lane_u64(vreinterpretq_u64_u8(sum0), 0);
}



int process_num(std::string_view num) {
    int num_size = num.length();

    int actualNum = 0;
    int mult = 1;

    for (int i = num_size-1; i >= 0; i--) {
        if (num[i] == '-') {
            actualNum *= -1;
            continue;
        }
        else if (num[i] == '.') {
            i--;
        }
        actualNum += (num[i] - '0') * mult;
        mult *= 10;
    }

    return actualNum;
}

void insert_into_map(
    std::string_view name,
    int actualNum,
    std::unordered_map<std::string, std::array<int, 4>, string_hash, std::equal_to<>>& dict
    )
{
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
}


int main() {
    // setup + mmap file
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


    // simd parsing
    uint8x16_t bitmask_sc = vdupq_n_u8(';');
    uint8x16_t bitmask_nl = vdupq_n_u8('\n');

    std::string_view num;
    std::string_view name;

    size_t start = 0;
    size_t delim_pos = 0;
    bool cur_delim = 0; // 0 for ';', 1 for '\n'

    size_t scan_pos = 0;
    size_t scan_size = 64;

    while (scan_pos + scan_size <= view.length()) {
        const uint8_t* view_ptr = reinterpret_cast<const uint8_t*>(view.data());
        uint8x16_t bytesA = vld1q_u8(view_ptr + scan_pos);
        uint8x16_t bytesB = vld1q_u8(view_ptr + scan_pos + 16);
        uint8x16_t bytesC = vld1q_u8(view_ptr + scan_pos + 32);
        uint8x16_t bytesD = vld1q_u8(view_ptr + scan_pos + 48);
        
        uint8x16_t delimsA = vorrq_u8(vceqq_u8(bitmask_sc, bytesA), vceqq_u8(bitmask_nl, bytesA));
        uint8x16_t delimsB = vorrq_u8(vceqq_u8(bitmask_sc, bytesB), vceqq_u8(bitmask_nl, bytesB));
        uint8x16_t delimsC = vorrq_u8(vceqq_u8(bitmask_sc, bytesC), vceqq_u8(bitmask_nl, bytesC));
        uint8x16_t delimsD = vorrq_u8(vceqq_u8(bitmask_sc, bytesD), vceqq_u8(bitmask_nl, bytesD));

        uint64_t mask = neonmovemask_bulk(delimsA, delimsB, delimsC, delimsD);

        while (mask != 0) {
            size_t search_delims = std::countr_zero(mask);
            
            delim_pos = scan_pos + search_delims;

            if (cur_delim == 0) {
                name = view.substr(start, delim_pos - start);

                cur_delim = 1;
            }
            else if (cur_delim == 1) {
                num = view.substr(start, delim_pos - start);
                int actualNum = process_num(num);                    
                insert_into_map(name, actualNum, dict);

                cur_delim = 0;
            }
            start = delim_pos + 1;

            mask &= mask - 1;
        } 

        scan_pos += scan_size;
    }

    // sequential for final <64. 

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
