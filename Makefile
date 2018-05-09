ROOT_DIR=.

BIN_DIR=$(ROOT_DIR)/bin

# INSTR_DIRs=$(shell find pass/ -mindepth 1 -maxdepth 1 -type d -not -path "pass/utils")

# SCHEDULE_DIRs=$(shell find schedule/ -mindepth 1 -maxdepth 1 -type d)

# ====================================================
# Find all the directories which containing makefile,
# some special directories are excluded.
# ====================================================
MAKE_DIRS=$(shell find ./analyze ./llvm ./runtime ./extlibs -type f -name '[Mm]akefile' -not -path './Makefile' -printf '%h\n')

.PHONY: scs $(MAKE_DIRS) clean

scs: $(MAKE_DIRS)
	-rm ./bin/libruntime.so
	./scripts/LetsGO chglib scs

$(MAKE_DIRS):
	make -C $@

clean: $(CLEAN_DIRS)
	@for D in $(MAKE_DIRS); do\
		echo "Clean dir:  $$D";\
		make clean -C $$D;\
	done
