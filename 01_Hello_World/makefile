TARGET   = executable.out
OBJDIR   = _build_

CXX      = g++
CXXFLAGS = -O3 -std=c++11 -pthread $(shell sdl2-config --cflags) $(INCDIR)

INCDIR   = -I.
LFLAGS   = -L. $(shell sdl2-config --libs) -lGL -lm
# The order of the libraries matter (dependent libraries go first).

rwildcard = $(foreach f,$(wildcard $1*),$(call rwildcard,$f/,$2) $(filter $2,$f))

HEADERS = $(call rwildcard,,%.hpp) $(call rwildcard,,%.h)
SOURCES = $(call rwildcard,,%.cpp)
OBJECTS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJDIR)
	@rm -f  $(TARGET)

.PHONY: clean
