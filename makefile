#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.13 2001/09/30 17:51:20 jon Exp $
#
all: debug rel profile profilena

GENERATED=

.PHONY: debug rel profile profilena clean full_clean

# Default arch and os, can be overridden by command line
OS=unix
ARCH=i386

AD_TARGET=	ad
DTOU_TARGET=	dtou
EAD_TARGET=	ead
EMU_TARGET=	emu
ID_TARGET=	id
IP_TARGET=	ip
MON_TARGET=	monst
MU_TARGET=	mu
PR_TARGET=	pr
SL_TARGET=	sl

DTOU_MODULES=	dtou
AD_MODULES=	ad add elements endian header memory primes read rows utils write
EAD_MODULES=	add ead endian header memory read rows system utils write
EMU_MODULES=	command emu memory system utils
ID_MODULES=	id elements endian header memory primes rows utils write
IP_MODULES=	elements endian header ip primes read utils write
MU_MODULES=	elements endian grease header matrix memory mu mul primes read rows utils write
MON_MODULES=	endian header memory mmat mop mtx primes utils write
PR_MODULES=	elements endian header memory pr primes read rows utils write
SL_MODULES=	add command elements endian grease header matrix memory mul primes read rows slave system utils write

MODULES=	$(AD_MODULES) $(DTOU_MODULES) $(EMU_MODULES) $(ID_MODULES) $(IP_MODULES) $(MON_MODULES) $(MU_MODULES) $(PR_MODULES) $(SL_MODULES)

include dirs.txt

.PRECIOUS: $(GENFILES)

SRCDIR=.
# Compiler search path for headers
INCLUDES=-I. -I$(GENDIR) -I$(SRCDIR) -I $(SRCDIR)/$(OS)
# Preprocessor defined values
DEFINES=-DMEM_SIZE=200

# Get OS specific portion
# Can similarly have arch specific portion
include $(OS)/GNUmake
# Get implicit rules
include rules.txt
# Get dependencies. Don't include when we make clean or full_clean. Avoid the empty set
DEPENDS=$(MODULES:%=$(DEPENDDIR)/%.$(DEPENDEXT))
ifeq "$(findstring clean,$(MAKECMDGOALS))" ""
ifneq "$(strip $(DEPENDS))" ""
-include $(DEPENDS)
endif
endif

# Make search paths for source
vpath %.c . $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)
vpath %.h . $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)

# Parameterically generate the link commands
CLEAN_ITEMS:=$(DEPENDS) $(GENFILES)

REL_TARGETS:=
DEBUG_TARGETS:=
PROF_TARGETS:=
PROFNA_TARGETS:=

TARGET:=AD
include targets.txt

TARGET:=DTOU
include targets.txt

TARGET:=EAD
include targets.txt

TARGET:=EMU
include targets.txt

TARGET:=ID
include targets.txt

TARGET:=IP
include targets.txt

TARGET:=MON
include targets.txt

TARGET:=MU
include targets.txt

TARGET:=PR
include targets.txt

TARGET:=SL
include targets.txt

debug: $(DEBUG_TARGETS)

rel: $(REL_TARGETS)

profile: $(PROF_TARGETS)

profilena: $(PROFNA_TARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
