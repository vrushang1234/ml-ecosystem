CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
SRCDIR := src
BUILDDIR := build
BINDIR := bin
TARGET := $(BINDIR)/compiler

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR) $(BINDIR):
	mkdir -p $@

run: $(TARGET)
	cd $(BINDIR) && ./compiler

clean:
	rm -rf $(BUILDDIR) $(BINDIR)
