Schnauzer
---
# A testing tool suit for multithreaded programs.

## Usage step by step

#### Clone the repo

`git clone git@github.com:midwinter1993/Schnauzer.git`

#### Set env variable

`export Schnauzer=`pwd` `

#### Install clang/llvm

`sudo apt-get install clang llvm`

#### Build sqlite3

```bash
cd $Schnauzer/extlibs
./build.sh
```

#### Install python dependencies

`cd $Schnauzer`
`pip install -r requirements.txt`

#### Set LN_CONFIG

`cd $Schnauzer/logs/pbzip2-0.9.4`

Edit LN.yaml: set `LOG_DIR=$Schnauzer/logs/pbzip2-0.9.4` and `PROJ_DIR=$Schnauzer`

`export LN_CONFIG=$Schnauzer/logs/pbzip2-0.9.4/LN.yaml`

`mkdir -pv srcinfo`

#### Compile pbzip2

`cd $Schnauzer/benchmark/pbzip2-0.9.4/pbzip2-0.9.4`
`./COMPILE`

#### Run pbzip2

`cd ..`
`./run.sh`

A lot of traces will be generated in `$Schnauzer/logs/pbzip2-0.9.4/detect`.

#### Analyze traces

`cd $Schnauzer`
`./LetsGo racesum logs/pbzip2-0.9.4/detect SCS `

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
`root dir` means the project directory (this README lies in).
All tools under `scripts` should be run in the root dir.



### Bugs we found
* https://github.com/transmission/transmission/issues/409
* https://github.com/cherokee/webserver/issues/1199
* https://github.com/transmission/transmission/issues/423
