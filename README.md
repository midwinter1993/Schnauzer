Schnauzer
---
## A testing tool suit for multithreaded programs.

### Usage
`export LN_CONFIG` to the `LN.yaml` config file under log dir

### Dependency
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
