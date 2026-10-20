# 1brc  
1 billion rows challenge (optimize reading in ~15GB input). 
original version: https://github.com/gunnarmorling/1brc

goal: learn performance optimization and cpp optimizations!

currently using c++26 (for simd), and compiling with clang++17. 

# Usage: 
```
cd src/main/python
python3 create_measurements.py 100_000_000 #increase to 1 billion later
```

check speed:

```
cd manit-solution
make
time ./solution 
```

check correctness:
```
cd manit-solution
make
./reference < ../data/measurements.txt > expected.out
./solution > output.txt
diff output.txt expected.out
```

# Times
naive - ~450s
day 1 - ~56s


# Log

## day 1
- got naive implementation done. 

- optimized my hash map to use heterogenous lookups to avoid repeated copies of strings (string_view). (before 1 billion string copies used to be made in hashmap checking, now only 10000 get made).

- implemented mmap to read in file from memory, and use string_view to deal with the data. (mmap copies data into kernel page cache and maps process virtual addresses to read from there. typical read copies data from kernel page cache into a local buffer which creates overhead. mmap also has page faults which help with extremely clean reading of a large file.)

- got started with using instruments to profile my code and logically derive opportunities for improvement. (xctrace)

## day 2
current bottlenecks to fix:
- hashmap is taking 60% + time - fix with custom implementation.
- memchr (related to .find()) is taking 20% of time - fix with SIMD?? ik csv parsers use them so i could try copying them.
- main itself takes 15% of time (haven't diagnosed)

new ideas:
- since program shares memory, multithreading could massively help

progress:
- fixed unoptimized formatting of numbers.
- working on simd to help with the memchr

