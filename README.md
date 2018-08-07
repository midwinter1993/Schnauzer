Schnauzer: A testing tool suit for multithreaded programs
=========================================================

Building instructions
------------------

First, we need to fetch the repo
```bash
$ git clone git@github.com:midwinter1993/Schnauzer.git
```

Set environment variable:
```bash
$ export Schnauzer=`pwd`
```

### Install dependencies

We need `LLVM` to do instrumentations, install it:
```bash
$ sudo apt-get install clang llvm
```
LLVM-3.8 is preferred.

Schnauzer uses `sqlite3` as its database to store source code information;
these information helps check which two statements form a data race during the program execution:
```bash
$ cd $Schnauzer/extlibs
$ ./build.sh
```

Some python scripts are used to glue functionalities together, which depend on other python packages:
```bash
$ cd $Schnauzer
$ pip install -r requirements.txt
```
Python-2.7 is preferred.

Use Schnauzer
-------------

For each benchmark program to test, we should prepare a configuration file for it.
Here, we use the `pbzip2` as an example to demonstrate the usages.

First, we create a log directory to save source code information, logs and analysis results.
```bash
$ mkdir -pv $Schnauzer/logs/pbzip2-0.9.4
```

The log directory for `pbzip2` has been created beforehand, and we also put a configuration file (`LN.yaml`) in it.
This `LN.yaml` is also a template for you to test other benchmarks.
```bash
$ cd $Schnauzer/logs/pbzip2-0.9.4
```

The LN.yaml needs some changes:
We should set `LOG_DIR=$Schnauzer/logs/pbzip2-0.9.4` and `PROJ_DIR=$Schnauzer`

Export the configuration file path and create a directory to save source information:
```bash
$ export LN_CONFIG=$Schnauzer/logs/pbzip2-0.9.4/LN.yaml
$ mkdir -pv srcinfo
```

### Compile&Run pbzip2

We prepare a compile script `COMPILE` which can help build the projects with autoconf.
```bash
cd $Schnauzer/benchmark/pbzip2-0.9.4/pbzip2-0.9.4
./COMPILE
```

Run pbzip2 with `run.sh` script:
```bash
cd ..
./run.sh
```

The `run.sh` is just a wrapper of the `$Schnauzer/scripts/control.py` that samples
execution speeds systematically and execute the program.

A lot of traces will be generated in `$Schnauzer/logs/pbzip2-0.9.4/detect`.

### Analyze traces

```bash
cd $Schnauzer
./LetsGo racesum logs/pbzip2-0.9.4/detect SCS
```

Explanation
-----------
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

Note
----
`PROJ_DIR` means the project directory (this README lies in).
All tools under `scripts` should be run in the proj dir.

### Bugs we found
* https://github.com/transmission/transmission/issues/409
* https://github.com/cherokee/webserver/issues/1199
* https://github.com/transmission/transmission/issues/423
