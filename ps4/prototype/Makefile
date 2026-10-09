# PS4 native boot prototype 0.2 - OpenOrbis >= v0.5.4.
# No commercial COD binaries/data or SDK binaries are checked into Git.
TITLE := COD1 PS4 - Prototipo
VERSION := 01.05
TITLE_ID := CODP40001
CONTENT_ID := IV0000-CODP40001_00-COD1PS4PROTOTYPE
# Authinfo from standard OpenOrbis/GoldHEN homebrew examples (not PSN credentials).
AUTHINFO := 000000000000000000000000001C004000FF000000000080000000000000000000000000000000000000008000400040000000000000008000000000000000080040FFFF000000F000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
TOOLCHAIN := $(OO_PS4_TOOLCHAIN)
BIN := $(TOOLCHAIN)/bin/linux
CC := clang++
LD := ld.lld
CFLAGS := --target=x86_64-pc-freebsd12-elf -std=c++11 -O2 -fPIC -funwind-tables \
          -isysroot $(TOOLCHAIN) -isystem $(TOOLCHAIN)/include \
          -isystem $(TOOLCHAIN)/include/c++/v1 -c -Wall -Wextra
LIBS := -lkernel -lSceVideoOut -lScePad -lSceUserService -lSceSysmodule -lSceSystemService -lc -lc++ -lm
LDFLAGS := -m elf_x86_64 -pie --script $(TOOLCHAIN)/link.x --eh-frame-hdr \
           -L$(TOOLCHAIN)/lib $(LIBS) $(TOOLCHAIN)/lib/crt1.o

.PHONY: all clean
all: $(CONTENT_ID).pkg

build/main.o: main.cpp
	@mkdir -p build
	$(CC) $(CFLAGS) $< -o $@

build/prototype.elf: build/main.o
	$(LD) $< -o $@ $(LDFLAGS)

eboot.bin: build/prototype.elf
	$(BIN)/create-fself -in=$< -out=build/prototype.oelf --eboot eboot.bin --paid 0x3800000000000011 --authinfo $(AUTHINFO)

sce_sys/icon0.png: tools/make_icon.py
	@mkdir -p sce_sys
	python3 tools/make_icon.py $@

sce_sys/param.sfo: Makefile
	@mkdir -p sce_sys
	$(BIN)/PkgTool.Core sfo_new $@
	$(BIN)/PkgTool.Core sfo_setentry $@ APP_TYPE --type Integer --maxsize 4 --value 1
	$(BIN)/PkgTool.Core sfo_setentry $@ APP_VER --type Utf8 --maxsize 8 --value '$(VERSION)'
	$(BIN)/PkgTool.Core sfo_setentry $@ ATTRIBUTE --type Integer --maxsize 4 --value 32
	$(BIN)/PkgTool.Core sfo_setentry $@ CATEGORY --type Utf8 --maxsize 4 --value 'gde'
	$(BIN)/PkgTool.Core sfo_setentry $@ CONTENT_ID --type Utf8 --maxsize 48 --value '$(CONTENT_ID)'
	$(BIN)/PkgTool.Core sfo_setentry $@ DOWNLOAD_DATA_SIZE --type Integer --maxsize 4 --value 0
	$(BIN)/PkgTool.Core sfo_setentry $@ SYSTEM_VER --type Integer --maxsize 4 --value 0
	$(BIN)/PkgTool.Core sfo_setentry $@ TITLE --type Utf8 --maxsize 128 --value '$(TITLE)'
	$(BIN)/PkgTool.Core sfo_setentry $@ TITLE_ID --type Utf8 --maxsize 12 --value '$(TITLE_ID)'
	$(BIN)/PkgTool.Core sfo_setentry $@ VERSION --type Utf8 --maxsize 8 --value '$(VERSION)'

pkg.gp4: eboot.bin sce_sys/param.sfo sce_sys/icon0.png
	$(BIN)/create-gp4 -out $@ --content-id=$(CONTENT_ID) --files "eboot.bin sce_sys/param.sfo sce_sys/icon0.png"

$(CONTENT_ID).pkg: pkg.gp4
	$(BIN)/PkgTool.Core pkg_build $< .

clean:
	rm -rf build eboot.bin pkg.gp4 sce_sys/param.sfo sce_sys/icon0.png $(CONTENT_ID).pkg
