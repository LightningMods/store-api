# Library metadata.
TARGET := store_api.prx

# Libraries linked into the ELF.
LIBS        :=  -lc++ -ljbc -lc -lz -lkernel  -lcurl  -lpolarssl  -lSceUserService -lSceSysmodule \
                -lSceNet -lSceSystemService -lSceCommonDialog -lSceMsgDialog -lSceImeDialog -lSceIme -lSceBgft -lSceAppInstUtil -lSceLibcInternal


# Additional compile flags.
EXTRAFLAGS  := -Wall -Wno-int-to-pointer-cast -Werror -Wno-for-loop-analysis -fcolor-diagnostics -Iinclude -Wall -D__ORBIS__ -D__PS4__

# You likely won't need to touch anything below this point.

# Root vars
TOOLCHAIN 	:= $(OO_PS4_TOOLCHAIN)
PROJDIR   	:= src
INTDIR    	:= build
TARGETSTATIC  = libstore_api.a

# Define objects to build
CFILES      := $(wildcard $(PROJDIR)/*.c)
CPPFILES    := $(wildcard $(PROJDIR)/*.cpp)
COMMONFILES := $(wildcard $(COMMONDIR)/*.cpp)
OBJS        := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o, $(CPPFILES)) $(patsubst $(COMMONDIR)/%.cpp, $(INTDIR)/%.o, $(COMMONFILES))
STUBOBJS    := $(patsubst $(PROJDIR)/%.c, $(INTDIR)/%.o, $(CFILES)) $(patsubst $(PROJDIR)/%.cpp, $(INTDIR)/%.o.stub, $(CPPFILES)) $(patsubst $(COMMONDIR)/%.cpp, $(INTDIR)/%.o.stub, $(COMMONFILES))

# Define final C/C++ flags
CFLAGS      := --target=x86_64-pc-freebsd12-elf -fPIC -funwind-tables -c $(EXTRAFLAGS) -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include -Iinclude -std=c11 -D_DEFAULT_SOURCE -DFMT_HEADER_ONLY 
CXXFLAGS    := $(CFLAGS) -isystem $(TOOLCHAIN)/include/c++/v1 -std=c++11 -Iinclude -DFMT_HEADER_ONLY -Wno-invalid-noreturn
LDFLAGS     := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crtlib.o

# Create the intermediate directory incase it doesn't already exist.
_unused     := $(shell mkdir -p $(INTDIR))

# Check for linux vs macOS and account for clang/ld path
UNAME_S     := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
		CC      := clang
		CCX     := clang++
		LD      := ld.lld
		CDIR    := linux
endif
ifeq ($(UNAME_S),Darwin)
		CC      := /usr/local/opt/llvm/bin/clang
		CCX     := /usr/local/opt/llvm/bin/clang++
		LD      := /usr/local/opt/llvm/bin/ld.lld
		CDIR    := macos
endif

$(TARGET): $(INTDIR) $(OBJS)
	$(LD) $(INTDIR)/*.o -o $(INTDIR)/$(PROJDIR).elf $(LDFLAGS)
	$(TOOLCHAIN)/bin/$(CDIR)/create-fself -in=$(INTDIR)/$(PROJDIR).elf -out=$(INTDIR)/$(PROJDIR).oelf --lib=$(TARGET) --paid 0x3800000000000011

$(TARGETSTATIC): $(INTDIR) $(OBJS)
	$(AR) rcs $(TARGETSTATIC) $(INTDIR)/*.o
	@echo Built static library successfully!

$(INTDIR)/%.o: $(PROJDIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(INTDIR)/%.o: $(PROJDIR)/%.cpp
	$(CCX) $(CXXFLAGS) -o $@ $<

.PHONY: clean
.DEFAULT_GOAL := all

all: $(TARGET) $(TARGETSTATIC)

clean:
	rm -f $(TARGET)  $(INTDIR)/$(PROJDIR).elf $(INTDIR)/$(PROJDIR)/oelf $(OBJS)
install:
	@echo Installing...
	@cp include/store_api.h $(OO_PS4_TOOLCHAIN)/include/store_api.h
	@yes | cp -f $(TARGETSTATIC) $(OO_PS4_TOOLCHAIN)/lib/$(TARGETSTATIC) 2>/dev/null && echo Installed!|| echo Failed to install, is it built?
