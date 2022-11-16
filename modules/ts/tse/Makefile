#(C)2004-2005 AMX Mod X Development Team
# Makefile written by David "BAILOPAN" Anderson

HLSDK = ../../../hlsdk-bu
METAMOD = ../../../metamod-bu/metamod

OPT_FLAGS = -O2 -s -pipe -fomit-frame-pointer -fno-strict-aliasing -std=c++11
#DEBUG_FLAGS = -g -ggdb3
CPP = g++
NAME = tse

BIN_SUFFIX_32 = amxx_i386.so
BIN_SUFFIX_64 = amxx_amd64.so

OBJECTS = ./CPlayer.cpp ./functions.cpp ./gameinfo.cpp ./main.cpp ./msghooking.cpp ./weapinfo.cpp

LINK = 

INCLUDE = -I. -I$(HLSDK)/public -I$(HLSDK)/common -I$(HLSDK)/dlls -I$(HLSDK)/engine -I$(HLSDK)/pm_shared \
			-I$(METAMOD) -I../../../public -I../../../public/sdk

GCC_VERSION := $(shell $(CPP) -dumpversion >&1 | cut -b1)

ifeq "$(GCC_VERSION)" "4"
	OPT_FLAGS += -fvisibility=hidden -fvisibility-inlines-hidden
endif

ifeq "$(DEBUG)" "true"
	BIN_DIR = Debug
	CFLAGS = $(DEBUG_FLAGS)
else
	BIN_DIR = Release
	CFLAGS = $(OPT_FLAGS)
endif

CFLAGS += -DNDEBUG -Wall -Wno-char-subscripts -Wno-unknown-pragmas -Wno-write-strings -Wno-deprecated -Wno-non-virtual-dtor \
			-fno-exceptions -DHAVE_STDINT_H -fno-rtti -static-libgcc -m32 -Dlinux -D__linux__

ifeq "$(AMD64)" "true"
	BINARY = $(NAME)_$(BIN_SUFFIX_64)
	CFLAGS += -DPAWN_CELL_SIZE=64 -DHAVE_I64 -m64 
else
	BINARY = $(NAME)_$(BIN_SUFFIX_32)
	CFLAGS += -DPAWN_CELL_SIZE=32 -DJIT -DASM32
	OPT_FLAGS += -march=i686
endif

OBJ_LINUX := $(OBJECTS:%.cpp=$(BIN_DIR)/%.o)

$(BIN_DIR)/%.o: %.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) -o $@ -c $<

all:
	mkdir -p $(BIN_DIR)/$(CPP_DIR)
	$(MAKE) $(NAME)

amd64:
	$(MAKE) all AMD64=true

$(NAME): $(OBJ_LINUX)
	$(CPP) $(INCLUDE) $(CFLAGS) $(OBJ_LINUX) $(LINK) -shared -ldl -lm -o$(BIN_DIR)/$(BINARY)

debug:	
	$(MAKE) all DEBUG=true

default: all

clean:
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/$(NAME)_$(BIN_SUFFIX_32)
	rm -rf $(BIN_DIR)/$(NAME)_$(BIN_SUFFIX_64)
