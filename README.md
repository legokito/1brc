1brc - 1 billion rows challenge (optimize reading in 10GB+ input). 

Usage: 
```
cd src/main/python
python3 create_measurements.py 10_000_000 #increase to 1 billion later
```

```
cd manit-solution
make
./reference < ../data/measurements.txt > expected.out
./solution 
# solution writes to a file which you can diff with expected.out
```
