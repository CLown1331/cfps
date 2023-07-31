# cfps

## Description

A simple tool to randomly choose a unsolved problem in a given rating range from [codeforces](https://codeforces.com/problemset).

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cd build
sudo make install
```

## Usage

```bash
cfps -l800 -u1300 -hCLown1331
```

### Flags
- `-l` - lower bound
- `-u` - upper bound
- `-h` - handle

## Web based version

[https://ajami1331.github.io/cfps-web/](https://ajami1331.github.io/cfps-web/)
