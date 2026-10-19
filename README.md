# 1brc  
1 billion rows challenge (optimize reading in ~15GB input). 
original version: https://github.com/gunnarmorling/1brc

goal: learn performance optimization and cpp optimizations!

currently using c++20 (for heterogenous lookup), and compiling with clang++17. 

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

## week 1
- got naive implementation done. 
- optimized my hash map to use heterogenous lookups to avoid repeated copies of strings (string_view).
- implemented mmap to read in file from memory, and use string_view to deal with the data. 
- got started with using instruments to profile my code and logically derive opportunities for improvement.

