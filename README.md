# Simple Profiler

Simple Profiler is a simple profiler for C++. Example usage is found in `example.cpp`.

Run using:

```
g++ -std=c++20 example.cpp -o example
./example
```

You should see the following output:

```
main: Mean: 1.46254e+07 μs, Std Dev: 0 μs, Min: 14625373 μs, Max: 14625373 μs, Count: 1
foo: Mean: 1.46253e+06 μs, Std Dev: 36741.1 μs, Min: 1407918 μs, Max: 1512958 μs, Count: 10
bar: Mean: 1348 μs, Std Dev: 580.371 μs, Min: 1010 μs, Max: 19764 μs, Count: 10000
```