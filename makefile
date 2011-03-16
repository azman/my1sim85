# makefile for my1asm8085

PROJECT = my1asm85
OBJECTS = $(PROJECT).o my1i8085.o
GUISPRO = $(PROJECT)-gui
CONSOBJ = $(PROJECT).o my1i8085.o
CONSPRO = $(PROJECT)

DELETE = rm -rf

ifeq ($(DO_MINGW),yes)
	CONSPRO = $(PROJECT).exe
	GUISPRO = $(PROJECT)-gui.EXE
	OBJECTS += wxmain.res
	XTOOL_DIR	?= /home/ftp/software/mingw-tool
	XTOOL_TARGET	= $(XTOOL_DIR)
	CROSS_COMPILE	= $(XTOOL_TARGET)/bin/i686-pc-mingw32-
	TARGET_ARCH = -Wall --static

	CFLAGS += -I$(XTOOL_DIR)/include -DDO_MINGW
	CFLAGS += $(TARGET_ARCH)
	LDFLAGS += -L$(XTOOL_DIR)/lib
	# below is to remove console at runtime
	LDFLAGS += -Wl,-subsystem,windows
	OFLAGS +=
	# can't remember why, but '-mthreads' is not playing nice with others - has to go!
	WX_LIBS = $(shell $(XTOOL_DIR)/bin/wx-config --libs | sed 's/-mthreads//g')
	WX_CXXFLAGS = $(shell $(XTOOL_DIR)/bin/wx-config --cxxflags | sed 's/-mthreads//g')
else
	CFLAGS += -Wall --static
	LFLAGS +=
	OFLAGS +=
	WX_LIBS = $(shell wx-config --libs)
	WX_CXXFLAGS = $(shell wx-config --cxxflags)
endif

CC = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
RES = $(CROSS_COMPILE)windres
debug: CFLAGS += -DMY1DEBUG
con: CFLAGS += -DMY1CONSOLE

all: con

gui: $(GUISPRO)

con: $(CONSPRO)

new: clean all

debug: new

$(CONSPRO): $(CONSOBJ)
	$(CC) $(CFLAGS) -o $@ $+ $(LFLAGS) $(OFLAGS)

$(GUISPRO): $(OBJECTS)
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

clean:
	-$(DELETE) $(PROJECT) $(OBJECTS) $(CONSPRO) $(CONSOBJ)
