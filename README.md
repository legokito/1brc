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

# Times
naive - ~450s
day 1 - ~52s
day 2 - ~28s


# Log

## day 1
- got naive implementation done. 

- optimized my hash map to use heterogenous lookups to avoid repeated copies of strings (string_view). (before 1 billion string copies used to be made in hashmap checking, now only 10000 get made).

- implemented mmap to read in file from memory, and use string_view to deal with the data. (mmap copies data into kernel page cache and maps process virtual addresses to read from there. typical read copies data from kernel page cache into a local buffer which creates overhead. mmap also has page faults which help with extremely clean reading of a large file.)

- got started with using instruments to profile my code and logically derive opportunities for improvement. (xctrace)

## day 2
- first, I optimized some inefficiencies in my string processing which shaved off a few seconds (we love string_view :P).

- main optimization today came from using SIMD in parsing the strings:
- I knew that using .find() for each delimiter wasn't as optimal since each memory access could potentially have more delimiters. This meant there was opportunity to SIMD and optimize. 
- I implemented a bitmask on 16 byte chunks, and looped over the resulting bytes to check whether that element was a delimiter (';', '\n') or not. This didn't provide any meaningful benefit so I continued googling to see how I could actually have this SIMD idea pan out. 
- That led me to this [article](https://branchfree.org/2019/04/01/fitting-my-head-through-the-arm-holes-or-two-sequences-to-substitute-for-the-missing-pmovmskb-instruction-on-arm-neon) which helped materialize the SIMD efforts I'd poured in and gave me a ~9 second benefit. For string processing being the 2nd or 3rd bottleneck on my list, this was a MASSIVE improvement. It is insane how smart that algorithm was and why it works! 









## current goals 
- DIAGNOSE THE FREAKING HASHMAP 
- look into multithreading

problems:
- have to deal with final 64 bytes of data...

