Schnauzer: A testing tool suit for multithreaded programs.
---

## Usage step by step

#### Clone the repo

```bash
git clone git@github.com:midwinter1993/Schnauzer.git
```

#### Set env variable

```bash
export Schnauzer=`pwd`
```

#### Install clang/llvm

```bash
sudo apt-get install clang llvm
```

#### Build sqlite3

```bash
cd $Schnauzer/extlibs
./build.sh
```

#### Install python dependencies

```bash
cd $Schnauzer
pip install -r requirements.txt
```

#### Set LN_CONFIG

```bash
cd $Schnauzer/logs/pbzip2-0.9.4
```

Edit LN.yaml:
```
LOG_DIR=$Schnauzer/logs/pbzip2-0.9.4
```
and
```
PROJ_DIR=$Schnauzer
```

```bash
export LN_CONFIG=$Schnauzer/logs/pbzip2-0.9.4/LN.yaml
mkdir -pv srcinfo
```

#### Compile pbzip2

```bash
cd $Schnauzer/benchmark/pbzip2-0.9.4/pbzip2-0.9.4
./COMPILE
```

#### Run pbzip2

```bash
cd ..
./run.sh
```

A lot of traces will be generated in `$Schnauzer/logs/pbzip2-0.9.4/detect`.

#### Analyze traces

```bash
cd $Schnauzer
./LetsGo racesum logs/pbzip2-0.9.4/detect SCS
```

## Dependency
* python2.7
* sqlite3
* LLVM-3.8
* Some python packages

### Explanation
```
.
├── analyze // HB data race detector
├── benchmark
├── bin
├── common
├── extlibs // sqlite3.so and malloc hijack
├── llvm // LLVM instrument
├── logs // Runtime log, source code infomation and detection results
├── runtime
│   ├── core
│   ├── sched // SCS implementation
│   └── scs // SCS definition
└── scripts // Tools for compiling, running and analyzing
    ├── codeGen.py // Used when add new functions to instrument
    ├── COMPILE // Compile the benchmark with instrument
    ├── control.py // Systematically sampling for testing
    ├── LetsGO // Main script
    ├── MYCC -> MYCC.py // clang wrapper
    ├── MYCC.py
    ├── MYCXX -> MYCXX.py // clang++ wrapper
    ├── MYCXX.py
```

### Note
`PROJ_DIR` means the project directory (this README lies in).
All tools under `scripts` should be run in the root dir.

### Bugs we found
* https://github.com/transmission/transmission/issues/409
* https://github.com/cherokee/webserver/issues/1199
* https://github.com/transmission/transmission/issues/423
