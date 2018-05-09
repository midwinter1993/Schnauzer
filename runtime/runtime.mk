ROOT_DIR=../..
include $(ROOT_DIR)/common/common.mk

LIB_NAME = lib$(CURR_DIR).so
.DEFAULT_GOAL:=$(BIN_DIR)/$(LIB_NAME)

RUNTIME_CORE_SRCS = $(shell find $(ROOT_DIR)/runtime/core -name "*.cpp")
RUNTIME_CORE_OBJS = $(RUNTIME_CORE_SRCS:.cpp=.o)
RUNTIME_CORE_DEPS = $(RUNTIME_CORE_SRCS:.cpp=.d)

RUNTIME_SCHED_SRCS = $(shell find $(ROOT_DIR)/runtime/sched -name "*.cpp")
RUNTIME_SCHED_OBJS = $(RUNTIME_SCHED_SRCS:.cpp=.o)
RUNTIME_SCHED_DEPS = $(RUNTIME_SCHED_SRCS:.cpp=.d)


#DEPENDS
-include $(RUNTIME_CORE_DEPS)
-include $(RUNTIME_SCHED_DEPS)

$(BIN_DIR)/$(LIB_NAME): $(OBJS) $(COMMON_OBJS) $(RUNTIME_CORE_OBJS) $(RUNTIME_SCHED_OBJS)
	$(CXX) -shared -fPIC $(CXXFLAGS) -o $@ $^ -lssl $(LDFLAGS)

.PHONY: clean
clean:
	-$(RM) $(BIN_DIR)/$(LIB_NAME)
	-$(RM) $(DEPS)
	-$(RM) $(OBJS)
	-$(RM) $(COMMON_DEPS)
	-$(RM) $(COMMON_OBJS)
	-$(RM) $(RUNTIME_CORE_DEPS)
	-$(RM) $(RUNTIME_CORE_OBJS)
	-$(RM) $(RUNTIME_SCHED_DEPS)
	-$(RM) $(RUNTIME_SCHED_OBJS)
	-$(RM) *.d *.o
