#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.24 2001/11/07 22:35:27 jon Exp $
#
all: debug rel profile profilena

GENERATED=

.PHONY: debug rel profile profilena clean full_clean

DTOU_TARGET=	dtou
EAD_TARGET=	ead
ECT_TARGET=	ect
EID_TARGET=	eid
EIM_TARGET=	eim
EIP_TARGET=	eip
EMU_TARGET=	emu
ETR_TARGET=	etr
MON_TARGET=	monst
STOP_TARGET=	stop
ZAD_TARGET=	zad
ZCT_TARGET=	zct
ZCV_TARGET=	zcv
ZEX_TARGET=	zex
ZID_TARGET=	zid
ZIP_TARGET=	zip
ZMU_TARGET=	zmu
ZPR_TARGET=	zpr
ZRN_TARGET=	zrn
ZSL_TARGET=	zsl
ZTR_TARGET=	ztr

DTOU_MODULES=	dtou
EAD_MODULES=	add ead endian files header map memory primes read rows system utils write
ECT_MODULES=	count ect elements endian files header map memory primes read utils
EID_MODULES=	eid elements endian exrows files header map memory primes rows utils write
EIM_MODULES=	eim elements endian files header map memory primes read utils write
EIP_MODULES=	eip elements endian exrows files header map memory primes read rows utils write
EMU_MODULES=	command emu files map memory system utils
ETR_MODULES=	elements endian etr files header map matrix memory primes read tra utils write
MON_MODULES=	endian exrows files header map memory mmat mop mtx primes rows utils write
STOP_MODULES=	command files stop system utils
ZAD_MODULES=	ad add elements endian header memory primes read rows utils write
ZCT_MODULES=	count ct elements endian header memory primes read utils
ZCV_MODULES=	elements endian header ip primes read utils write
ZEX_MODULES=	elements endian exrows files header map memory primes read rows utils write zex
ZID_MODULES=	id ident elements endian header memory primes rows utils write
ZIP_MODULES=	elements endian header ipp memory primes read rows utils write
ZMU_MODULES=	elements endian grease header matrix memory mu mul primes read rows utils write
ZPR_MODULES=	elements endian header memory pr primes read rows utils write
ZRN_MODULES=	clean elements endian grease header matrix memory primes read rn rows utils
ZSL_MODULES=	add elements endian files grease header matrix memory mul primes read rows slave system utils write
ZTR_MODULES=	elements endian header matrix memory primes read tr tra utils write

MODULES=	$(ZAD_MODULES) $(ZCT_MODULES) $(DTOU_MODULES) $(EAD_MODULES) $(ECT_MODULES) $(EID_MODULES) $(EIM_MODULES) $(EMU_MODULES) $(ETR_MODULES) $(ZID_MODULES) $(ZCV_MODULES) $(MON_MODULES) $(ZMU_MODULES) $(ZPR_MODULES) $(ZSL_MODULES) $(STOP_MODULES) $(ZTR_MODULES) $(ZEX_MODULES)

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
vpath %.c $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)
vpath %.h $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)

# Parameterically generate the link commands
CLEAN_ITEMS:=$(DEPENDS) $(GENFILES)

REL_TARGETS:=
DEBUG_TARGETS:=
PROF_TARGETS:=
PROFNA_TARGETS:=

TARGET:=DTOU
include targets.txt

TARGET:=EAD
include targets.txt

TARGET:=ECT
include targets.txt

TARGET:=EID
include targets.txt

TARGET:=EIM
include targets.txt

TARGET:=EIP
include targets.txt

TARGET:=EMU
include targets.txt

TARGET:=ETR
include targets.txt

TARGET:=MON
include targets.txt

TARGET:=STOP
include targets.txt

TARGET:=ZAD
include targets.txt

TARGET:=ZCV
include targets.txt

TARGET:=ZCT
include targets.txt

TARGET:=ZEX
include targets.txt

TARGET:=ZID
include targets.txt

TARGET:=ZIP
include targets.txt

TARGET:=ZMU
include targets.txt

TARGET:=ZPR
include targets.txt

TARGET:=ZRN
include targets.txt

TARGET:=ZSL
include targets.txt

TARGET:=ZTR
include targets.txt

debug: $(DEBUG_TARGETS)

rel: $(REL_TARGETS)

profile: $(PROF_TARGETS)

profilena: $(PROFNA_TARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
