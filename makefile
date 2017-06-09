HEADER_FILES = $(shell find -iregex ".*\(.hpp\)")
SOURCE_FILES = # Add .cpp files here
OBJECT_FILES = $(patsubst %.cpp,%.o,$(SOURCE_FILES))

TEST_FILES = $(shell find -iregex ".*\(_test.cpp\)")

all: tests $(OBJECT_FILES)
	./tests -r compact

clean:
	rm -rf tests

tests: tests.o $(TEST_FILES) $(HEADER_FILES) $(MAKEFILE_LIST)
	g++ -o $@ $< $(TEST_FILES)

%.o: %.cpp
	g++ -c $<
