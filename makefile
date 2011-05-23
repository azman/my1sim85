# makefile for my1sim8085

PROJECT = my1sim8085
GUISPRO = $(PROJECT)
GUISOBJ = wxmain.o wxform.o my1sim85.o my1i8085.o
EXTPATH = ../my1asm85/src

DELETE = rm -rf

ifeq ($(DO_MINGW),yes)
	GUISPRO = $(PROJECT).EXE
	GUISOBJ += wxmain.res
	XTOOL_DIR	?= /home/ftp/software/mingw-tool
	XTOOL_TARGET	= $(XTOOL_DIR)
	CROSS_COMPILE	= $(XTOOL_TARGET)/bin/i686-pc-mingw32-
	TARGET_ARCH = 

	CFLAGS += $(TARGET_ARCH)
	CFLAGS += -I$(XTOOL_DIR)/include -DDO_MINGW
	LDFLAGS += -L$(XTOOL_DIR)/lib
	# below is to remove console at runtime
	LDFLAGS += -Wl,-subsystem,windows
	OFLAGS +=
	# can't remember why, but '-mthreads' is not playing nice with others - has to go!
	WX_LIBS = $(shell $(XTOOL_DIR)/bin/wx-config --libs | sed 's/-mthreads//g')
	WX_CXXFLAGS = $(shell $(XTOOL_DIR)/bin/wx-config --cxxflags | sed 's/-mthreads//g')
else
	WX_LIBS = $(shell wx-config --libs)
	WX_CXXFLAGS = $(shell wx-config --cxxflags)
endif

CFLAGS += -I$(EXTPATH) -Wall
LFLAGS +=
OFLAGS +=

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
RES = $(CROSS_COMPILE)windres
debug: CFLAGS += -DMY1DEBUG

all: gui

gui: $(GUISPRO)

new: clean all

debug: new

$(GUISPRO): $(GUISOBJ)
	$(CPP) $(CFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS) $(WX_LIBS)

%.o: src/%.c src/%.h
	$(CC) $(CFLAGS) -c $<

%.o: src/%.c
	$(CC) $(CFLAGS) -c $<

%.o: src/%.cpp src/%.hpp
	$(CPP) $(CFLAGS) -c $<

%.o: src/%.cpp
	$(CPP) $(CFLAGS) -c $<

wx%.o: src/wx%.cpp src/wx%.hpp
	$(CPP) $(CFLAGS) $(WX_CXXFLAGS) -c $<

wx%.o: src/wx%.cpp
	$(CPP) $(CFLAGS) $(WX_CXXFLAGS) -c $<

%.res: src/%.rc
	$(RES) $< -O coff -o $@

%.o: $(EXTPATH)/%.c $(EXTPATH)/%.h
	$(CC) $(CFLAGS) -c $<

%.o: $(EXTPATH)/%.c
	$(CC) $(CFLAGS) -c $<

clean:
	-$(DELETE) $(GUISPRO) $(GUISOBJ)
