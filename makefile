# makefile for my1sim85 - 8085 system simulator

PROJECT = my1sim85
GUISPRO = $(PROJECT)
GUISOBJ = wxmain.o wxform.o wxcode.o wxpref.o wxled.o wxswitch.o my1sim85.o my1i8085.o
EXTPATH = ../my1asm85/src
PACKDIR = $(PROJECT)-package
PACKDAT = README TODO CHANGELOG asm
PLATBIN = $(shell uname -m)

DELETE = rm -rf
COPY = cp -R
ARCHIVE = tar cjf
ARCHEXT = .tar.bz2
CONVERT = convert

CFLAGS += -Wall -I$(EXTPATH)
LFLAGS +=
OFLAGS +=
LOCAL_FLAGS =
WX_LIBS = stc,aui,html,adv,core,xml,base

ifeq ($(DO_MINGW),YES)
	GUISPRO = $(PROJECT).exe
	GUISOBJ += wxmain.res
	PLATBIN = mingw
	ARCHIVE = zip -r
	ARCHEXT = .zip
	# cross-compiler settings
	XTOOL_DIR ?= /home/share/tool/mingw
	XTOOL_TARGET = $(XTOOL_DIR)
	CROSS_COMPILE = $(XTOOL_TARGET)/bin/i686-pc-mingw32-
	TARGET_ARCH = --static
	# below is to remove console at runtime
	LDFLAGS += -Wl,-subsystem,windows
	# extra switches
	CFLAGS += $(TARGET_ARCH) -I$(XTOOL_DIR)/include -DDO_MINGW -DWIN32_LEAN_AND_MEAN
	LDFLAGS += -L$(XTOOL_DIR)/lib
	# can't remember why, but '-mthreads' is not playing nice with others - has to go!
	WX_LIBFLAGS = $(shell $(XTOOL_DIR)/bin/wx-config --libs $(WX_LIBS) | sed 's/-mthreads//g')
	WX_CXXFLAGS = $(shell $(XTOOL_DIR)/bin/wx-config --cxxflags | sed 's/-mthreads//g')
else
	WX_LIBFLAGS = $(shell wx-config --libs $(WX_LIBS))
	WX_CXXFLAGS = $(shell wx-config --cxxflags)
endif

PACKDAT += $(GUISPRO)
CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
RES = $(CROSS_COMPILE)windres
debug: LOCAL_FLAGS += -DMY1DEBUG

all: gui

gui: $(GUISPRO)

new: clean all

debug: all

pack: gui
	mkdir -pv $(PACKDIR)
	$(COPY) $(PACKDAT) $(PACKDIR)/
	ARCNAME = $(PROJECT)-$(PLATBIN)-$(shell date +%Y%m%d).$(ARCHEXT)
	$(DELETE) $(ARCNAME)
	$(ARCHIVE) $(ARCNAME) $(PACKDIR)

$(GUISPRO): $(GUISOBJ)
	$(CPP) $(CFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS) $(WX_LIBFLAGS)

wx%.o: src/wx%.cpp src/wx%.hpp
	$(CPP) $(CFLAGS) $(WX_CXXFLAGS) -c $<

wx%.o: src/wx%.cpp
	$(CPP) $(CFLAGS) $(WX_CXXFLAGS) -c $<

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) $(LOCAL_FLAGS) -c $<

%.o: src/%.c
	$(CC) $(CFLAGS) $(LOCAL_FLAGS) -c $<

%.o: src/%.cpp src/%.hpp
	$(CPP) $(CFLAGS) $(LOCAL_FLAGS) -c $<

%.o: src/%.cpp
	$(CPP) $(CFLAGS) $(LOCAL_FLAGS) -c $<

%.ico: res/%.xpm
	$(CONVERT) $< $@

%.res: src/%.rc apps.ico
	$(RES) --include-dir res --include-dir /home/share/tool/mingw/include --include-dir /home/share/tool/mingw/include/wx-2.9 -O COFF $< -o $@

%.o: $(EXTPATH)/%.c $(EXTPATH)/%.h
	$(CC) $(CFLAGS) -DMY1CONSOLE -c $<

%.o: $(EXTPATH)/%.c
	$(CC) $(CFLAGS) -c $<

clean:
	-$(DELETE) $(GUISPRO) $(GUISOBJ) $(PACKDIR) *.exe *.bz2 *.zip *.o *.ico *.res
