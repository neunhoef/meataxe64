#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.46 2002/02/05 19:50:56 jon Exp $
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
ESAD_TARGET=	esad
ESID_TARGET=	esid
ETR_TARGET=	etr
MON_TARGET=	monst
SNS_TARGET=	sns
SRN_TARGET=	srn
STOP_TARGET=	stop
ZAD_TARGET=	zad
ZBASE_TARGET=	zbase
ZCHAR_TARGET=	zchar
ZCHECK_TARGET=	zcheck
ZCONJ_TARGET=	zconj
ZCT_TARGET=	zct
ZCV_TARGET=	zcv
ZDIAG_TARGET=	zdiag
ZDIFF_TARGET=	zdiff
ZEX_TARGET=	zex
ZFE_TARGET=	zfe
ZID_TARGET=	zid
ZIP_TARGET=	zip
ZIV_TARGET=	ziv
ZJOIN_TARGET=	zjoin
ZMU_TARGET=	zmu
ZNOC_TARGET=	znoc
ZNOR_TARGET=	znor
ZPR_TARGET=	zpr
ZPRIME_TARGET=	zprime
ZRE_TARGET=	zre
ZRN_TARGET=	zrn
ZRNF_TARGET=	zrnf
ZNS_TARGET=	zns
ZNSF_TARGET=	znsf
ZQF_TARGET=	zqf
ZQS_TARGET=	zqs
ZSAD_TARGET=	zsad
ZSB_TARGET=	zsb
ZSEL_TARGET=	zsel
ZSID_TARGET=	zsid
ZSL_TARGET=	zsl
ZSKSQ_TARGET=	zsksq
ZSB_TARGET=	zsb
ZSP_TARGET=	zsp
ZSPAN_TARGET=	zspan
ZSS_TARGET=	zss
ZSUMS_TARGET=	zsums
ZTE_TARGET=	zte
ZTR_TARGET=	ztr
ZTRACE_TARGET=	ztrace
ZVP_TARGET=	zvp
ZWORDS_TARGET=	zwords

DTOU_MODULES=	dtou
EAD_MODULES=	add ead endian files header map memory primes read rows system utils write
ECT_MODULES=	count ect elements endian files header map memory primes read utils
EID_MODULES=	eid elements endian exrows files header map memory primes rows utils write
EIM_MODULES=	eim elements endian exrows files header map memory primes read utils write
EIP_MODULES=	eip elements endian exrows files header map memory primes read rows utils write
EMU_MODULES=	command emu files map memory system utils
ESAD_MODULES=	add esad endian files header map memory primes read rows system utils write
ESID_MODULES=	elements endian esid exrows files header map memory primes rows utils write
ETR_MODULES=	elements endian etr files header map matrix memory primes read tra utils write
MON_MODULES=	elements endian exrows files header map memory mmat mop mtx primes rows utils write
SNS_MODULES=	clean elements endian grease header matrix memory primes read rows sns utils write
SRN_MODULES=	clean elements endian grease header matrix memory primes read rows srn utils
STOP_MODULES=	command files stop system utils
ZAD_MODULES=	ad add elements endian header memory primes read rows utils write
ZBASE_MODULES=	clean elements endian grease header matrix memory primes read rnf rows system utils write zbase
ZCHAR_MODULES=	endian header primes read utils zchar
ZCHECK_MODULES=	elements endian header memory primes read utils zcheck
ZCONJ_MODULES=	conj elements endian header memory primes read utils write zconj
ZCT_MODULES=	count ct elements endian header memory primes read utils
ZCV_MODULES=	elements endian header ip primes read utils write
ZDIAG_MODULES=	elements endian header memory primes read rows utils write zdiag
ZDIFF_MODULES=	diff endian header memory primes read utils zdiff
ZEX_MODULES=	elements endian exrows files header map memory primes read rows utils write zex
ZFE_MODULES=	elements endian extend extend_matrix header memory primes read rows utils write zfe
ZID_MODULES=	id ident elements endian header memory primes rows utils write
ZIP_MODULES=	elements endian header ipp memory primes read rows utils write
ZIV_MODULES=	clean elements endian grease header matrix memory primes read iv rows utils write ziv
ZJOIN_MODULES=	endian header join memory primes read utils write zjoin
ZMU_MODULES=	elements endian grease header matrix memory mu mul primes read rows utils write
ZNOC_MODULES=	endian header primes read utils znoc
ZNOR_MODULES=	endian header primes read utils znor
ZNS_MODULES=	clean elements endian grease header matrix memory primes read ns rows utils write zns
ZNSF_MODULES=	clean elements endian grease header ident matrix memory primes read nsf rows system utils write znsf
ZPR_MODULES=	elements endian header memory pr primes read rows utils write
ZPRIME_MODULES=	endian header primes read utils zprime
ZQF_MODULES=	elements endian header memory primes read rows utils write zqf
ZQS_MODULES=	clean elements endian grease header matrix memory primes qs read rows utils write zqs
ZRE_MODULES=	elements endian header memory primes read utils write zre
ZRN_MODULES=	clean elements endian grease header matrix memory primes read rn rows utils zrn
ZRNF_MODULES=	clean elements endian grease header matrix memory primes read rnf rows system utils write zrnf
ZSAD_MODULES=	add elements endian header memory primes read rows utils write zsad
ZSB_MODULES=	clean elements endian grease header matrix memory mul primes read rows sb utils write zsb
ZSEL_MODULES=	endian header memory primes read utils write zse
ZSID_MODULES=	ident elements endian header memory primes rows utils write zsid
ZSKSQ_MODULES=	elements endian header matrix memory powers primes read rows utils write zsksq
ZSL_MODULES=	add elements endian files grease header matrix memory mul primes read rows slave system utils write
ZSP_MODULES=	clean elements endian grease header matrix memory mul primes read rows sp utils write zsp
ZSPAN_MODULES=	elements endian header matrix memory primes read rows utils write zspan
ZSS_MODULES=	clean elements endian grease header matrix memory primes read rows ss utils write zss
ZSUMS_MODULES=	add clean elements endian grease header ident matrix memory mul primes read rn rows utils write zsums
ZTE_MODULES=	elements endian header matrix memory primes read rows te utils write zte
ZTR_MODULES=	elements endian header matrix memory primes read tr tra utils write
ZTRACE_MODULES=	elements endian header memory primes read rows utils ztrace
ZVP_MODULES=	elements endian grease header matrix memory mul primes read rows vp utils write zvp
ZWORDS_MODULES=	elements endian grease header matrix memory mul primes read rows utils write zwords

MODULES=	$(DTOU_MODULES) \
	$(EAD_MODULES) \
	$(ECT_MODULES) \
	$(EID_MODULES) \
	$(EIM_MODULES) \
	$(EIP_MODULES) \
	$(EMU_MODULES) \
	$(ESAD_MODULES) \
	$(ESID_MODULES) \
	$(ETR_MODULES) \
	$(MON_MODULES) \
	$(SNS_MODULES) \
	$(SRN_MODULES) \
	$(STOP_MODULES) \
	$(ZAD_MODULES) \
	$(ZBASE_MODULES) \
	$(ZCHAR_MODULES) \
	$(ZCHECK_MODULES) \
	$(ZCONJ_MODULES) \
	$(ZCT_MODULES) \
	$(ZCV_MODULES) \
	$(ZDIAG_MODULES) \
	$(ZDIFF_MODULES) \
	$(ZEX_MODULES) \
	$(ZFE_MODULES) \
	$(ZID_MODULES) \
	$(ZIP_MODULES) \
	$(ZIV_MODULES) \
	$(ZJOIN_MODULES) \
	$(ZMU_MODULES) \
	$(ZNOC_MODULES) \
	$(ZNOR_MODULES) \
	$(ZNS_MODULES) \
	$(ZNSF_MODULES) \
	$(ZPR_MODULES) \
	$(ZPRIME_MODULES) \
	$(ZQF_MODULES) \
	$(ZQS_MODULES) \
	$(ZRE_MODULES) \
	$(ZRN_MODULES) \
	$(ZRNF_MODULES) \
	$(ZSAD_MODULES) \
	$(ZSB_MODULES) \
	$(ZSEL_MODULES) \
	$(ZSID_MODULES) \
	$(ZSKSQ_MODULES) \
	$(ZSL_MODULES) \
	$(ZSP_MODULES) \
	$(ZSPAN_MODULES) \
	$(ZSS_MODULES) \
	$(ZSUMS_MODULES) \
	$(ZTE_MODULES) \
	$(ZTR_MODULES) \
	$(ZTRACE_MODULES) \
	$(ZVP_MODULES) \
	$(ZWORDS_MODULES)

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

TARGET:=ESAD
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

TARGET:=ESID
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

TARGET:=ZBASE
include targets.txt

TARGET:=ZCHAR
include targets.txt

TARGET:=ZCHECK
include targets.txt

TARGET:=ZCONJ
include targets.txt

TARGET:=ZCT
include targets.txt

TARGET:=ZCV
include targets.txt

TARGET:=ZDIAG
include targets.txt

TARGET:=ZDIFF
include targets.txt

TARGET:=ZEX
include targets.txt

TARGET:=ZFE
include targets.txt

TARGET:=ZID
include targets.txt

TARGET:=ZIP
include targets.txt

TARGET:=ZIV
include targets.txt

TARGET:=ZJOIN
include targets.txt

TARGET:=ZMU
include targets.txt

TARGET:=ZNOC
include targets.txt

TARGET:=ZNOR
include targets.txt

TARGET:=ZNS
include targets.txt

TARGET:=ZNSF
include targets.txt

TARGET:=ZPR
include targets.txt

TARGET:=ZPRIME
include targets.txt

TARGET:=ZQF
include targets.txt

TARGET:=ZQS
include targets.txt

TARGET:=ZRE
include targets.txt

TARGET:=ZRN
include targets.txt

TARGET:=ZRNF
include targets.txt

TARGET:=ZSAD
include targets.txt

TARGET:=ZSB
include targets.txt

TARGET:=ZSEL
include targets.txt

TARGET:=ZSID
include targets.txt

TARGET:=ZSKSQ
include targets.txt

TARGET:=ZSL
include targets.txt

TARGET:=ZSP
include targets.txt

TARGET:=ZSPAN
include targets.txt

TARGET:=ZSS
include targets.txt

TARGET:=ZSUMS
include targets.txt

TARGET:=ZTE
include targets.txt

TARGET:=ZTR
include targets.txt

TARGET:=ZTRACE
include targets.txt

TARGET:=ZVP
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
