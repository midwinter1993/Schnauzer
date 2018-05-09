#
# include common files and files in *CURRENT* dir
#
BIN_DIR=$(ROOT_DIR)/bin

CURR_DIR = $(notdir $(shell pwd))
CXX=clang++
CXXFLAGS=-std=c++11 -fPIC -I$(ROOT_DIR)  -Wall -g -O2
LDFLAGS=-L$(ROOT_DIR)/extlibs -Wl,-rpath,$(ROOT_DIR)/extlibs -lsqlite3 -ldl -lpthread -lunwind

COMMON_SRCS = $(shell find $(ROOT_DIR)/common/ -name "*.cpp")
COMMON_OBJS = $(COMMON_SRCS:.cpp=.o)
COMMON_DEPS = $(COMMON_SRCS:.cpp=.d)

SRCS = $(shell find ./ -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

.cpp.o:
	$(CXX) -MD -MP $(CXXFLAGS) -o $@ -c $<

#DEPENDS
-include $(COMMON_DEPS) $(DEPS)
%.d:
	@touch $@
