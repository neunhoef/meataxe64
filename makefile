#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.19 2001/10/10 23:19:42 jon Exp $
#
all: debug rel profile profilena

GENERATED=

.PHONY: debug rel profile profilena clean full_clean

AD_TARGET=	ad
CT_TARGET=	ct
DTOU_TARGET=	dtou
EAD_TARGET=	ead
ECT_TARGET=	ect
EID_TARGET=	eid
EIM_TARGET=	eim
EMU_TARGET=	emu
ETR_TARGET=	etr
ID_TARGET=	id
IP_TARGET=	ip
MON_TARGET=	monst
MU_TARGET=	mu
PR_TARGET=	pr
SL_TARGET=	sl
STOP_TARGET=	stop
TR_TARGET=	tr
ZEX_TARGET=	zex

AD_MODULES=	ad add elements endian header memory primes read rows utils write
CT_MODULES=	count ct elements endian header memory primes read utils
DTOU_MODULES=	dtou
EAD_MODULES=	add ead endian files header map memory read rows system utils write
ECT_MODULES=	count ect elements endian files header map memory primes read utils
EID_MODULES=	eid elements endian exrows files header map memory primes rows utils write
EIM_MODULES=	eim elements endian files header map memory primes read utils write
EMU_MODULES=	command emu files map memory system utils
ETR_MODULES=	elements endian etr files header map matrix memory primes read tra utils write
ID_MODULES=	id ident elements endian header memory primes rows utils write
IP_MODULES=	elements endian header ip primes read utils write
MON_MODULES=	endian exrows files header map memory mmat mop mtx primes utils write
MU_MODULES=	elements endian grease header matrix memory mu mul primes read rows utils write
PR_MODULES=	elements endian header memory pr primes read rows utils write
SL_MODULES=	add command elements endian files grease header matrix memory mul primes read rows slave system utils write
STOP_MODULES=	command files stop system utils
TR_MODULES=	elements endian header matrix memory primes read tr tra utils write
ZEX_MODULES=	elements endian exrows files header map memory primes read rows utils write zex

MODULES=	$(AD_MODULES) $(CT_MODULES) $(DTOU_MODULES) $(EAD_MODULES) $(ECT_MODULES) $(EID_MODULES) $(EIM_MODULES) $(EMU_MODULES) $(ETR_MODULES) $(ID_MODULES) $(IP_MODULES) $(MON_MODULES) $(MU_MODULES) $(PR_MODULES) $(SL_MODULES) $(STOP_MODULES) $(TR_MODULES) $(ZEX_MODULES)

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

TARGET:=AD
include targets.txt

TARGET:=CT
include targets.txt

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

TARGET:=EMU
include targets.txt

TARGET:=ETR
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

TARGET:=STOP
include targets.txt

TARGET:=TR
include targets.txt

TARGET:=ZEX
include targets.txt

debug: $(DEBUG_TARGETS)

rel: $(REL_TARGETS)

profile: $(PROF_TARGETS)

profilena: $(PROFNA_TARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
