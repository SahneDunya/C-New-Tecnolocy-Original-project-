CXX = clang++

CXXSTANDARD = c++17

CXXFLAGS = -std=$(CXXSTANDARD) -Wall -Wextra -g -I. $(shell llvm-config --cxxflags)

LDFLAGS = $(shell llvm-config --ldflags --system-libs --libs core native)

SOURCES = \
    main.cpp \
    diagnostics.cpp \
    utils.cpp \
    token.cpp \
    lexer.cpp \
    parser.cpp \
    ast.cpp \
    type_system.cpp \
    symbol_table.cpp \
    ownership_checker.cpp \
    module_interface.cpp \
    module_manager.cpp \
    interface_extractor.cpp \
    codegen.cpp

OBJECTS = $(SOURCES:.cpp=.o)

TARGET = c new tecnology standard compiler

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp \
    diagnostics.h \
    utils.h \
    token.h \
    lexer.h \
    parser.h \
    ast.h \
    expressions.h \
    statements.h \
    declarations.h \
    types.h \
    type_system.h \
    symbol_table.h \
    ownership_checker.h \
    module_interface.h \
    module_manager.h \
    interface_extractor.h \
    codegen.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean
