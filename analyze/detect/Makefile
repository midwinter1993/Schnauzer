ROOT_DIR=../..
include $(ROOT_DIR)/common/common.mk

EXEC_NAME = $(CURR_DIR)
.DEFAULT_GOAL:=$(BIN_DIR)/$(EXEC_NAME)

DEP_GRAPH_SRCS = $(shell find ../depgraph -name "*.cpp")
DEP_GRAPH_OBJS = $(DEP_GRAPH_SRCS:.cpp=.o)
DEP_GRAPH_DEPS = $(DEP_GRAPH_SRCS:.cpp=.d)


$(BIN_DIR)/$(EXEC_NAME): $(OBJS) $(COMMON_OBJS) $(DEP_GRAPH_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

-include $(DEP_GRAPH_DEPS)

.PHONY: clean
clean:
	-rm $(BIN_DIR)/$(EXEC_NAME) *.bc *.ll
	-$(RM) $(COMMON_DEPS)
	-$(RM) $(COMMON_OBJS)
	-$(RM) $(DEP_GRAPH_OBJS)
	-$(RM) $(DEP_GRAPH_DEPS)
	-$(RM) $(DEPS)
	-$(RM) $(OBJS)
