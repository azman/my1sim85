# makefile for my1sim85

PROJECT = my1sim85
GUISPRO = $(PROJECT)
GUISOBJ = wxmain.o wxform.o wxcode.o wxpref.o wxled.o wxswitch.o my1sim85.o my1i8085.o
EXTPATH = ../my1asm85/src

DELETE = rm -rf

LOCAL_FLAGS =
WX_LIBS = stc,aui,html,adv,core,xml,base

ifeq ($(DO_MINGW),yes)
	GUISPRO = $(PROJECT).EXE
	GUISOBJ += wxmain.res
	ifeq ($(DO_WIN32),yes)
		DELETE = del
		CFLAGS += -Wall --static
	else
		XTOOL_DIR ?= /home/ftp/software/mingw-tool
		XTOOL_TARGET = $(XTOOL_DIR)
		CROSS_COMPILE = $(XTOOL_TARGET)/bin/i686-pc-mingw32-
		TARGET_ARCH = -Wall --static

		CFLAGS += -I$(XTOOL_DIR)/include -DDO_MINGW
		CFLAGS += $(TARGET_ARCH)
		LDFLAGS += -L$(XTOOL_DIR)/lib
	endif
	# below is to remove console at runtime
	LDFLAGS += -Wl,-subsystem,windows
	OFLAGS +=
	# can't remember why, but '-mthreads' is not playing nice with others - has to go!
	WX_LIBFLAGS = $(shell $(XTOOL_DIR)/bin/wx-config --libs $(WX_LIBS) | sed 's/-mthreads//g')
	WX_CXXFLAGS = $(shell $(XTOOL_DIR)/bin/wx-config --cxxflags | sed 's/-mthreads//g')
else
	WX_LIBFLAGS = $(shell wx-config --libs $(WX_LIBS))
	WX_CXXFLAGS = $(shell wx-config --cxxflags)
endif

CFLAGS += -I$(EXTPATH) -Wall
LFLAGS +=
OFLAGS +=

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
RES = $(CROSS_COMPILE)windres
debug: LOCAL_FLAGS += -DMY1DEBUG

all: gui

gui: $(GUISPRO)

new: clean all

debug: all

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

%.res: src/%.rc
	$(RES) $< -O coff -o $@

%.o: $(EXTPATH)/%.c $(EXTPATH)/%.h
	$(CC) $(CFLAGS) -DMY1CONSOLE -c $<

%.o: $(EXTPATH)/%.c
	$(CC) $(CFLAGS) -c $<

clean:
	-$(DELETE) $(GUISPRO) $(GUISOBJ)

