ROOT_DIR=../..
include $(ROOT_DIR)/common/common.mk

LIB_NAME = $(CURR_DIR).so
.DEFAULT_GOAL:=$(BIN_DIR)/$(LIB_NAME)

CXXFLAGS +=-Wno-unknown-warning-option $(shell llvm-config --cxxflags) -g
LLVM_LDFLAGS=$(shell llvm-config --ldflags) -lLLVM-3.8

$(BIN_DIR)/$(LIB_NAME): $(OBJS) $(COMMON_OBJS)
	clang++ -shared $(LLVM_CXXFLAGS) $(CXXFLAGS) $^ -o $@ $(LLVM_LDFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	-rm $(BIN_DIR)/$(LIB_NAME)
	-$(RM) $(COMMON_DEPS)
	-$(RM) $(COMMON_OBJS)
	-$(RM) $(DEPS)
	-$(RM) $(OBJS)
