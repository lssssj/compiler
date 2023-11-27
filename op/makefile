CC = clang
CXX = clang++
CFLAGS = -Wall -O2

objects = foo.o bar.o all.o
# all: $(objects)
# 	$(CXX) $(objects) -o all
fi = op/foo.cpp sysy-cmake-template/op/foo.cpp


one := $(patsubst %.o, %.c, $(objects))

two := $(dir $(objects))

all: $(objects)
	@echo $(one)
	@echo $(two)


# These files compile via implicit rules
# Syntax - targets ...: target-pattern: prereq-patterns ...
# In the case of the first target, foo.o, the target-pattern matches foo.o and sets the "stem" to be "foo".
# It then replaces the '%' in prereq-patterns with that stem
$(objects): %.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o all