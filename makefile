#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.39 2001/12/23 23:31:42 jon Exp $
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
SNS_TARGET=	sns
SRN_TARGET=	srn
STOP_TARGET=	stop
ZAD_TARGET=	zad
ZCHECK_TARGET=	zcheck
ZCT_TARGET=	zct
ZCV_TARGET=	zcv
ZDIFF_TARGET=	zdiff
ZEX_TARGET=	zex
ZID_TARGET=	zid
ZIP_TARGET=	zip
ZIV_TARGET=	ziv
ZMU_TARGET=	zmu
ZNOC_TARGET=	znoc
ZNOR_TARGET=	znor
ZPR_TARGET=	zpr
ZPRIME_TARGET=	zprime
ZRE_TARGET=	zre
ZRN_TARGET=	zrn
ZRNF_TARGET=	zrnf
ZNS_TARGET=	zns
ZQS_TARGET=	zqs
ZSB_TARGET=	zsb
ZSEL_TARGET=	zsel
ZSL_TARGET=	zsl
ZSB_TARGET=	zsb
ZSP_TARGET=	zsp
ZSPAN_TARGET=	zspan
ZSS_TARGET=	zss
ZTE_TARGET=	zte
ZTR_TARGET=	ztr
ZTRACE_TARGET=	ztrace
ZWORDS_TARGET=	zwords

DTOU_MODULES=	dtou
EAD_MODULES=	add ead endian files header map memory primes read rows system utils write
ECT_MODULES=	count ect elements endian files header map memory primes read utils
EID_MODULES=	eid elements endian exrows files header map memory primes rows utils write
EIM_MODULES=	eim elements endian files header map memory primes read utils write
EIP_MODULES=	eip elements endian exrows files header map memory primes read rows utils write
EMU_MODULES=	command emu files map memory system utils
ETR_MODULES=	elements endian etr files header map matrix memory primes read tra utils write
MON_MODULES=	endian exrows files header map memory mmat mop mtx primes rows utils write
SNS_MODULES=	clean elements endian grease header matrix memory primes read rows sns utils write
SRN_MODULES=	clean elements endian grease header matrix memory primes read rows srn utils
STOP_MODULES=	command files stop system utils
ZAD_MODULES=	ad add elements endian header memory primes read rows utils write
ZCHECK_MODULES=	elements endian header memory primes read utils zcheck
ZCT_MODULES=	count ct elements endian header memory primes read utils
ZCV_MODULES=	elements endian header ip primes read utils write
ZDIFF_MODULES=	diff endian header memory primes read utils zdiff
ZEX_MODULES=	elements endian exrows files header map memory primes read rows utils write zex
ZID_MODULES=	id ident elements endian header memory primes rows utils write
ZIP_MODULES=	elements endian header ipp memory primes read rows utils write
ZIV_MODULES=	clean elements endian grease header matrix memory primes read iv rows utils write ziv
ZMU_MODULES=	elements endian grease header matrix memory mu mul primes read rows utils write
ZNOC_MODULES=	endian header primes read utils znoc
ZNOR_MODULES=	endian header primes read utils znor
ZNS_MODULES=	clean elements endian grease header matrix memory primes read ns rows utils write zns
ZPR_MODULES=	elements endian header memory pr primes read rows utils write
ZPRIME_MODULES=	endian header primes read utils zprime
ZQS_MODULES=	clean elements endian grease header matrix memory primes qs read rows utils write zqs
ZRE_MODULES=	elements endian header memory primes read utils write zre
ZRN_MODULES=	clean elements endian grease header matrix memory primes read rn rows utils zrn
ZRNF_MODULES=	clean elements endian grease header matrix memory primes read rnf rows system utils zrnf
ZSB_MODULES=	clean elements endian grease header matrix memory mul primes read rows sb utils write zsb
ZSEL_MODULES=	endian header memory primes read utils write zse
ZSL_MODULES=	add elements endian files grease header matrix memory mul primes read rows slave system utils write
ZSP_MODULES=	clean elements endian grease header matrix memory mul primes read rows sp utils write zsp
ZSPAN_MODULES=	elements endian header matrix memory primes read rows utils write zspan
ZSS_MODULES=	clean elements endian grease header matrix memory primes read rows ss utils write zss
ZTE_MODULES=	elements endian header matrix memory primes read rows te utils write zte
ZTR_MODULES=	elements endian header matrix memory primes read tr tra utils write
ZTRACE_MODULES=	elements endian header memory primes read rows utils ztrace
ZWORDS_MODULES=	elements endian grease header matrix memory mul primes read rows utils write zwords

MODULES=	$(DTOU_MODULES) $(EAD_MODULES) $(ECT_MODULES) $(EID_MODULES) $(EIM_MODULES) $(EIP_MODULES) $(EMU_MODULES) $(ETR_MODULES) $(MON_MODULES) $(SNS_MODULES) $(SRN_MODULES) $(STOP_MODULES) $(ZAD_MODULES) $(ZCHECK_MODULES) $(ZCT_MODULES) $(ZCV_MODULES) $(ZDIFF_MODULES) $(ZEX_MODULES) $(ZID_MODULES) $(ZIP_MODULES) $(ZIV_MODULES) $(ZMU_MODULES) $(ZNOC_MODULES) $(ZNOR_MODULES) $(ZNS_MODULES) $(ZPR_MODULES) $(ZPRIME_MODULES) $(ZQS_MODULES) $(ZRE_MODULES) $(ZRN_MODULES) $(ZRNF_MODULES) $(ZSP_MODULES) $(ZSEL_MODULES) $(ZSL_MODULES) $(ZSP_MODULES) $(ZSPAN_MODULES) $(ZSS_MODULES) $(ZTE_MODULES) $(ZTR_MODULES) $(ZTRACE_MODULES) $(ZWORDS_MODULES)

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

TARGET:=SNS
include targets.txt

TARGET:=SRN
include targets.txt

TARGET:=STOP
include targets.txt

TARGET:=ZAD
include targets.txt

TARGET:=ZCHECK
include targets.txt

TARGET:=ZCT
include targets.txt

TARGET:=ZCV
include targets.txt

TARGET:=ZDIFF
include targets.txt

TARGET:=ZEX
include targets.txt

TARGET:=ZID
include targets.txt

TARGET:=ZIP
include targets.txt

TARGET:=ZIV
include targets.txt

TARGET:=ZMU
include targets.txt

TARGET:=ZNOC
include targets.txt

TARGET:=ZNOR
include targets.txt

TARGET:=ZNS
include targets.txt

TARGET:=ZPR
include targets.txt

TARGET:=ZPRIME
include targets.txt

TARGET:=ZQS
include targets.txt

TARGET:=ZRE
include targets.txt

TARGET:=ZRN
include targets.txt

TARGET:=ZRNF
include targets.txt

TARGET:=ZSB
include targets.txt

TARGET:=ZSEL
include targets.txt

TARGET:=ZSL
include targets.txt

TARGET:=ZSP
include targets.txt

TARGET:=ZSPAN
include targets.txt

TARGET:=ZSS
include targets.txt

TARGET:=ZTE
include targets.txt

TARGET:=ZTR
include targets.txt

TARGET:=ZTRACE
include targets.txt

TARGET:=ZWORDS
include targets.txt

debug: $(DEBUG_TARGETS)

rel: $(REL_TARGETS)

profile: $(PROF_TARGETS)

profilena: $(PROFNA_TARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
