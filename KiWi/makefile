CC := clang
# CC := clang --analyze
KIWIDIR := kiwi 
UTILDIR := util
BUILDDIR := build
BINDIR := bin
TARGET := $(BINDIR)/runner

SRCEXT := cpp
SOURCES := $(shell find $(KIWIDIR) $(UTILDIR) -type f -name "*.$(SRCEXT)")
OBJECTS := $(patsubst $(KIWIDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
OBJECTS := $(patsubst $(UTILDIR)/%,$(BUILDDIR)/%,$(OBJECTS))
CFLAGS := -g -Wall # -O0 -g3 -Wall -fmessage-length=0 -MMD
# LIB := -pthread -lmongoclient -L lib -lboost_thread-mt -lboost_filesystem-mt -lboost_system-mt 
INC := -I include -I /usr/local/include

all: $(TARGET) tester

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR) $(BINDIR)
	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(BINDIR)"; $(RM) -r $(BUILDDIR) $(BINDIR)

# Tests
tester:
	$(CC) $(CFLAGS) main.cpp $(INC) $(LIB) -o $(BINDIR)/tester

.PHONY: clean