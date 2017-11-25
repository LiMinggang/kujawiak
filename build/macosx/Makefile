PROJ = kuview.bin
BUNDLE = kuView
CXX = /usr/bin/g++
BUILD = debug

ARCH = -arch i386

WXCFG = ../../../wxMac-2.8.12/build_u$(BUILD)/wx-config
FIINC = ../../../FreeImage/Dist
FILIB = ../../../FreeImage/Dist

ifeq ($(BUILD), release)
CXXFLAGS = -I$(FIINC) -DFREEIMAGE_LIB $(shell $(WXCFG) --cxxflags) $(ARCH) -O2
else
CXXFLAGS = -I$(FIINC) -DFREEIMAGE_LIB $(shell $(WXCFG) --cxxflags) $(ARCH) -g
endif

LDFLAGS = -L$(FILIB) -lfreeimage $(shell $(WXCFG) --libs) $(ARCH)

INCDIR = ../../include
SRCDIR = ../../src
SRCS = $(shell cd $(SRCDIR) && ls *.cpp)
OBJS = $(SRCS:%.cpp=%.o)

all: info $(OBJS) $(PROJ) bundle

info:
	@echo "$(PROJ) BUILD = $(BUILD)"
ifneq ($(wildcard build.*), build.$(BUILD))
	@rm -f build.*
	@rm -f *.o
	@touch build.$(BUILD)
endif

$(PROJ): $(OBJS)
	@echo "Linking $@..."
	@$(CXX) $^ -o $@ $(LDFLAGS)
ifeq ($(BUILD), release)
endif

bundle:
	@echo "Creating bundle for $@..."
	mkdir -p $(BUNDLE).app/Contents/MacOS
	mkdir -p $(BUNDLE).app/Contents/Resources
	echo -n 'APPLkuVu' > kuView.app/Contents/PkgInfo
	cp $(PROJ) $(BUNDLE).app/Contents/MacOS/
	cp ../../icons/icns/kujawiak_t.icns $(BUNDLE).app/Contents/Resources/
	cp Info.plist $(BUNDLE).app/Contents/

%.o: $(SRCDIR)/%.cpp
	@echo "Compiling $<..."
	@$(CXX) -c -I$(INCDIR) $(CXXFLAGS) -o $@ $<

clean:
	rm -f *.o

