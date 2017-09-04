# Force make to use g++ for linking instead of gcc
LINK.o = $(LINK.cc)

SRC_FILES = $(wildcard templates/*.cpp)
OBJ_FILES = $(addsuffix .o,$(basename $(SRC_FILES)))
DEP_FILES = $(addsuffix .d,$(basename $(SRC_FILES)))

CPPFLAGS += -MD
CPPFLAGS += -MP

all: tests
	./tests

clean:
	rm -rf $(DEP_FILES)
	rm -rf $(OBJ_FILES)

tests: tests.o $(OBJ_FILES)

-include $(DEP_FILES)
