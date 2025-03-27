#
# meataxe64 makefile for building on multiple targets
#
all: debug rel profile profilena

.PHONY: debug rel profile profilena clean relpp relasm

# Defaults for command line parameters
# Not sure we'll ever support non-unix systems
OS?=unix
# Possibilities: generic (pure C), em64t (64 bit x86), arm
ARCH?=generic
# Possibilities: gcc, clang (llvm)
COMPILER?=gcc

LIB_MODULES=	memfuns \
	gauss \
	memfuns \
	dfmtf \
	field \
	conex \
	pcritc \
	exten \
	tfarm1 \
	tfarm2 \
	io \
	proggies \
	bitstring \
	slab \
	pmul \
	tabmake \
	hpmi \
	mmul \
	mpef \
	mech \
	mfuns \
	linf \
	util \
	fech \
	fmul \
	ftra \
	fpef \
	funs1 \
	funs2 \
	funs3 \
	funs4 \
	funs5 \
	malloc \
	parse \
	utils

# Extra from meataxe2000
LIB_MODULES+=	header \
		endian \
		read \
		write \
		primes \
		elements \
		sums_utils \
		span

MODULES=	$(LIB_MODULES)

include dirs.txt
include defines.txt

SRCDIR=	src
# Compiler search path for headers
INCLUDES=-I$(SRCDIR)/$(OS)
# Preprocessor defined values
DEFINES=-DMEM_SIZE=200000 -DCACHE_SIZE=1000

# Make search paths for source
vpath %.c $(SRCDIR) $(SRCDIR)/$(OS) $(SRCDIR)/$(ARCH) $(GENDIR)
vpath %.h $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)
vpath %.s $(SRCDIR)

# Get OS and architecture specific portions
include $(OS)/GNUmake
include $(ARCH)/GNUmake
# Get implicit rules
include rules.txt
# Get dependencies. Don't include when we make clean. Avoid the empty set
DEPENDS=$(MODULES:%=$(DEPENDDIR)/%.$(DEPENDEXT))
ifeq "$(findstring clean,$(MAKECMDGOALS))" ""
ifneq "$(strip $(DEPENDS))" ""
-include $(DEPENDS)
endif
endif

REL_TARGETS:=
DEBUG_TARGETS:=
PROF_TARGETS:=
PROFNA_TARGETS:=
RELPP_TARGETS:=
RELASM_TARGETS:=

MTXLIB=	libmtx
include libs.txt

EXECS=	conreg \
	fieldreg \
	zad \
	zas \
	zbc \
	zch \
	zchar \
	zcn \
	zcp \
	zcs \
	zcv \
	zcx \
	zde \
	zdiff \
	zdm \
	ze2 \
	ze3 \
	zec \
	zexport \
	zfr \
	zga \
	zhr \
	zid \
	zimport \
	zis \
	ziv \
	zkc \
	zkx \
	zmp \
	zmu \
	zng \
	znoc \
	znor \
	znu \
	zor1 \
	zpc \
	zpe \
	zpe1 \
	zpk \
	zpk1 \
	zplain \
	zplain2 \
	zpm \
	zpr \
	zprime \
	zqa \
	zrd \
	zrn \
	zrr \
	zrx \
	zs2 \
	zsa \
	zsb \
	zsc \
	zscript \
	zsh \
	zsi \
	zsid \
	zsl \
	zspan \
	zspl \
	zsums \
	zsv \
	ztc \
	zte \
	ztr \
	zut \
	zvp \
	zzs

include exec.txt
include execs.txt

# The rules cope with the fact that the binary 64 imports and exporters
# come from the sources import.c and export.c
debug:	$(DEBUG_TARGETS)
	@cd $(DEBUGBINDIR) && rm -f zexport64 zimport64 && ln -s zexport zexport64 && ln -s zimport zimport64
rel:	$(REL_TARGETS)
	@cd $(RELBINDIR) && rm -f zexport64 zimport64 && ln -s zexport zexport64 && ln -s zimport zimport64
relpp:	$(RELPP_TARGETS)
relasm:	$(RELASM_TARGETS)
profile:	$(PROF_TARGETS)
	@cd $(PROFBINDIR) && rm -f zexport64 zimport64 && ln -s zexport zexport64 && ln -s zimport zimport64
profilena:	$(PROFNA_TARGETS)
	@cd $(PROFNABINDIR) && rm -f zexport64 zimport64 && ln -s zexport zexport64 && ln -s zimport zimport64

clean:
	rm -rf $(BASEDIR)
