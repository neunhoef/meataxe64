#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.77 2002/10/12 17:40:49 jon Exp $
#
all: debug rel profile profilena

GENERATED=

.PHONY: debug rel profile profilena clean full_clean

DTOU_TARGET=	dtou
DECOMP_TARGET=	decomp
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
ZAH_TARGET=	zah
ZBASE_TARGET=	zbase
ZCHAR_TARGET=	zchar
ZCHECK_TARGET=	zcheck
ZCLEAN_TARGET=	zclean
ZCONJ_TARGET=	zconj
ZCT_TARGET=	zct
ZCV_TARGET=	zcv
ZDIAG_TARGET=	zdiag
ZDIFF_TARGET=	zdiff
ZEX_TARGET=	zex
ZEXPORT_TARGET=	zexport
ZFE_TARGET=	zfe
ZFLN_TARGET=	zfln
ZFLNF_TARGET=	zflnf
ZFO_TARGET=	zfo
ZID_TARGET=	zid
ZIMPORT_TARGET=	zimport
ZIP_TARGET=	zip
ZIV_TARGET=	ziv
ZIVF_TARGET=	zivf
ZJOIN_TARGET=	zjoin
ZLP_TARGET=	zlp
ZLV_TARGET=	zlv
ZMSB_TARGET=	zmsb
ZMSBF_TARGET=	zmsbf
ZMSP_TARGET=	zmsp
ZMSPF_TARGET=	zmspf
ZMU_TARGET=	zmu
ZNOC_TARGET=	znoc
ZNOR_TARGET=	znor
ZNS_TARGET=	zns
ZNSF_TARGET=	znsf
ZPCO_TARGET=	zpco
ZPCV_TARGET=	zpcv
ZPR_TARGET=	zpr
ZPRIME_TARGET=	zprime
ZPRO_TARGET=	zpro
ZPROJ_TARGET=	zproj
ZPS_TARGET=	zps
ZQF_TARGET=	zqf
ZQS_TARGET=	zqs
ZRAND_TARGET=	zrand
ZRANKS_TARGET=	zranks
ZRE_TARGET=	zre
ZRES_TARGET=	zrestrict
ZRN_TARGET=	zrn
ZRNF_TARGET=	zrnf
ZRRANKS_TARGET=	zrranks
ZRSUMS_TARGET=	zrsums
ZRSUMSF_TARGET=	zrsumsf
ZSAD_TARGET=	zsad
ZSB_TARGET=	zsb
ZSBF_TARGET=	zsbf
ZSCRIPT_TARGET=	zscript
ZSEL_TARGET=	zsel
ZSID_TARGET=	zsid
ZSIGN_TARGET=	zsign
ZSING_TARGET=	zsing
ZSKCU_TARGET=	zskcu
ZSKFI_TARGET=	zskfi
ZSKFO_TARGET=	zskfo
ZSKSE_TARGET=	zskse
ZSKSI_TARGET=	zsksi
ZSKSQ_TARGET=	zsksq
ZSL_TARGET=	zsl
ZSP_TARGET=	zsp
ZSPAN_TARGET=	zspan
ZSPANMSP_TARGET=	zspanmsp
ZSPF_TARGET=	zspf
ZSS_TARGET=	zss
ZSUMS_TARGET=	zsums
ZSUMSF_TARGET=	zsumsf
ZSYMSQ_TARGET=	zsymsq
ZTCO_TARGET=	ztco
ZTCV_TARGET=	ztcv
ZTE_TARGET=	zte
ZTMU_TARGET=	ztmu
ZTR_TARGET=	ztr
ZTRACE_TARGET=	ztrace
ZTRECOV_TARGET=	ztrecover
ZTSP_TARGET=	ztsp
ZTSPF_TARGET=	ztspf
ZVP_TARGET=	zvp
ZWORDS_TARGET=	zwords

DTOU_MODULES=	dtou
DECOMP_MODULES=	decomp utils
EAD_MODULES=	add ead elements endian files header map map_or_row maps memory parse primes read rows system utils write
ECT_MODULES=	count ect elements endian files header map memory parse primes read utils
EID_MODULES=	eid elements endian exrows files header map memory parse primes rows utils write
EIM_MODULES=	eim elements endian exrows files header map memory parse primes read utils write
EIP_MODULES=	eip elements endian exrows files header map memory parse primes read rows utils write
EMU_MODULES=	command emu files map memory parse system utils
ESAD_MODULES=	add esad elements endian files header map map_or_row maps memory parse primes read rows system utils write
ESID_MODULES=	elements endian esid exrows files header map memory parse primes rows utils write
ETR_MODULES=	elements endian etr files header map maps matrix memory parse primes read rows tra utils write
MON_MODULES=	elements endian exrows files header map memory mmat mop mtx parse primes rows utils write
SNS_MODULES=	clean elements endian grease header matrix memory parse primes read rows sns utils write
SRN_MODULES=	clean elements endian grease header matrix memory parse primes read rows srn utils
STOP_MODULES=	command files stop system utils
ZAD_MODULES=	ad add elements endian header map_or_row maps memory parse primes read rows utils write
ZAH_MODULES=	elements endian files header memory parse primes read utils write zah
ZBASE_MODULES=	base clean clean_file elements endian grease header maps matrix memory parse primes read rows system utils write zbase
ZCHAR_MODULES=	endian header parse primes read utils zchar
ZCHECK_MODULES=	elements endian header memory parse primes read utils zcheck
ZCLEAN_MODULES=	clean clean_vectors elements endian grease header matrix memory parse primes read rows utils write zclean
ZCONJ_MODULES=	conj elements endian header memory parse primes read utils write zconj
ZCT_MODULES=	count ct elements endian header memory parse primes read utils
ZCV_MODULES=	elements endian header ip parse primes read utils write
ZDIAG_MODULES=	elements endian header memory parse primes read rows utils write zdiag
ZDIFF_MODULES=	diff elements endian header maps memory parse primes read rows utils write zdiff
ZEX_MODULES=	elements endian exrows files header map memory parse primes read rows utils write zex
ZEXPORT_MODULES=	elements endian header memory parse primes read rows utils write zexport
ZFE_MODULES=	elements endian extend extend_matrix header memory parse primes read rows utils write zfe
ZFLN_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rn rows sums utils write zfln
ZFLNF_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rnf rows sumsf system utils write zflnf
ZFO_MODULES=	elements endian header maps orbit parse primes read rows utils write zfo
ZID_MODULES=	id ident elements endian header maps memory parse primes rows utils write
ZIMPORT_MODULES=	elements endian header memory parse primes read rows utils write zimport
ZIP_MODULES=	elements endian header ipp memory parse primes read rows utils write
ZIV_MODULES=	clean elements endian grease header iv maps matrix memory parse primes read rows utils write ziv
ZIVF_MODULES=	clean elements endian grease header ivf maps matrix memory parse primes read rows system utils write zivf
ZJOIN_MODULES=	elements endian header join  map_or_row maps memory parse primes read rows utils write zjoin
ZLP_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows vp utils write zlp
ZLV_MODULES=	elements endian header lv matrix memory parse primes read rows ss_map utils write zlv
ZMSB_MODULES=	clean elements endian grease header maps matrix memory msb mul parse primes read rows utils write zmsb
ZMSBF_MODULES=	clean clean_file elements endian grease header maps matrix memory msbf mul parse primes read rows system utils write zmsbf
ZMSP_MODULES=	clean elements endian grease header maps matrix memory msp mul parse primes read rows utils write zmsp
ZMSPF_MODULES=	clean clean_file elements endian grease header maps matrix memory mspf mul parse primes read rows system utils write zmspf
ZMU_MODULES=	elements endian grease header maps matrix memory mu mul parse primes read rows utils write
ZNOC_MODULES=	endian header parse primes read utils znoc
ZNOR_MODULES=	endian header parse primes read utils znor
ZNS_MODULES=	clean elements endian grease header matrix memory parse primes read ns rows utils write zns
ZNSF_MODULES=	clean elements endian grease header ident maps matrix memory parse primes read nsf rows system utils write znsf
ZPCO_MODULES=	elements endian header maps memory orbit pco parse primes read rows utils write zpco
ZPCV_MODULES=	elements endian header memory orbit pcv parse primes read rows utils write zpcv
ZPR_MODULES=	elements endian header memory pr parse primes read rows utils write
ZPRIME_MODULES=	endian header parse primes read utils zprime
ZPRO_MODULES=	endian header orbit parse primes read utils write zpro
ZPROJ_MODULES=	clean elements endian grease header map_or_row maps matrix memory parse primes project read rows utils write zproj
ZPS_MODULES=	endian header matrix memory parse primes ps read utils write zps
ZQF_MODULES=	elements endian header memory parse primes read rows utils write zqf
ZQS_MODULES=	clean elements endian grease header matrix memory parse primes qs read rows utils write zqs
ZRAND_MODULES=	rand elements endian header maps memory parse primes rows utils write zrand
ZRANKS_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rn rows sums utils write zranks
ZRE_MODULES=	elements endian header memory parse primes read utils write zre
ZRES_MODULES=	elements endian header memory parse primes read restrict rows utils write zrestrict
ZRN_MODULES=	clean elements endian grease header maps matrix memory parse primes read rn rows utils write zrn
ZRNF_MODULES=	clean elements endian grease header maps matrix memory parse primes read rnf rows system utils write zrnf
ZRRANKS_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rn rows sums utils write zrranks
ZRSUMS_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rn rows sums utils write zrsums
ZRSUMSF_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rnf rows sumsf system utils write zrsumsf
ZSAD_MODULES=	add elements endian header map_or_row maps memory parse primes read rows utils write zsad
ZSB_MODULES=	clean elements endian grease header maps matrix memory mul parse primes read rows sb utils write zsb
ZSBF_MODULES=	clean clean_file elements endian grease header maps matrix memory mul parse primes read rows sbf system utils write zsbf
ZSCRIPT_MODULES=	add elements endian files grease header ident map_or_row maps matrix memory mul parse primes read rows scale script utils write zscript
ZSEL_MODULES=	endian header memory parse primes read utils write zse
ZSID_MODULES=	ident elements endian header maps memory parse primes rows utils write zsid
ZSIGN_MODULES=	clean elements endian grease header maps matrix memory mul parse primes read rows sign singular utils write zsign
ZSING_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows singular utils write zsing
ZSKCU_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zskcu
ZSKFI_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zskfi
ZSKFO_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zskfo
ZSKSE_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zskse
ZSKSI_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zsksi
ZSKSQ_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zsksq
ZSL_MODULES=	add elements endian files grease header map_or_row maps matrix memory mul parse primes read rows slave system utils write
ZSP_MODULES=	clean elements endian grease header maps matrix memory mul parse primes read rows sp utils write zsp
ZSPAN_MODULES=	elements endian header matrix memory parse primes read rows utils write zspan
ZSPANMSP_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows spanmsp utils write zspanmsp
ZSPF_MODULES=	clean clean_file elements endian grease header maps matrix memory mul parse primes read rows spf system utils write zspf
ZSS_MODULES=	elements endian header memory parse primes read rows ss ss_map utils write zss
ZSUMS_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rn rows sums utils write zsums
ZSUMSF_MODULES=	add clean elements endian grease header ident map_or_row maps matrix memory mul parse primes read rnf rows sumsf system utils write zsumsf
ZSYMSQ_MODULES=	dets elements endian header matrix memory powers parse primes read rows utils write zsymsq
ZTCO_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows tco utils write ztco
ZTCV_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows tcv utils write ztcv
ZTE_MODULES=	elements endian header map_or_row maps matrix memory parse primes read rows te utils write zte
ZTMU_MODULES=	elements endian grease header maps matrix memory mul mv parse primes read rows tmul tra utils write ztmu
ZTR_MODULES=	elements endian header maps matrix memory parse primes read rows tr tra utils write
ZTRACE_MODULES=	elements endian header memory parse primes read rows utils ztrace
ZTRECOV_MODULES=	elements endian header maps matrix memory mv parse primes read rows utils write ztrecover
ZTSP_MODULES=	clean elements endian grease header maps matrix memory mul mv parse primes read rows tra tsp utils write ztsp
ZTSPF_MODULES=	clean clean_file elements endian grease header maps matrix memory mul mv parse primes read rows system tra tspf utils write ztspf
ZVP_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows vp utils write zvp
ZWORDS_MODULES=	elements endian grease header maps matrix memory mul parse primes read rows utils write zwords

MODULES=	$(DECOMP_MODULES) \
	$(DTOU_MODULES) \
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
	$(ZAH_MODULES) \
	$(ZBASE_MODULES) \
	$(ZCHAR_MODULES) \
	$(ZCHECK_MODULES) \
	$(ZCLEAN_MODULES) \
	$(ZCONJ_MODULES) \
	$(ZCT_MODULES) \
	$(ZCV_MODULES) \
	$(ZDIAG_MODULES) \
	$(ZDIFF_MODULES) \
	$(ZEX_MODULES) \
	$(ZEXPORT_MODULES) \
	$(ZFE_MODULES) \
	$(ZFLN_MODULES) \
	$(ZFLNF_MODULES) \
	$(ZFO_MODULES) \
	$(ZID_MODULES) \
	$(ZIMPORT_MODULES) \
	$(ZIP_MODULES) \
	$(ZIV_MODULES) \
	$(ZIVF_MODULES) \
	$(ZJOIN_MODULES) \
	$(ZLP_MODULES) \
	$(ZLV_MODULES) \
	$(ZMSB_MODULES) \
	$(ZMSBF_MODULES) \
	$(ZMSP_MODULES) \
	$(ZMSPF_MODULES) \
	$(ZMU_MODULES) \
	$(ZNOC_MODULES) \
	$(ZNOR_MODULES) \
	$(ZNS_MODULES) \
	$(ZNSF_MODULES) \
	$(ZPCO_MODULES) \
	$(ZPCV_MODULES) \
	$(ZPR_MODULES) \
	$(ZPRIME_MODULES) \
	$(ZPRO_MODULES) \
	$(ZPROJ_MODULES) \
	$(ZPS_MODULES) \
	$(ZQF_MODULES) \
	$(ZQS_MODULES) \
	$(ZRAND_MODULES) \
	$(ZRANKS_MODULES) \
	$(ZRE_MODULES) \
	$(ZRES_MODULES) \
	$(ZRN_MODULES) \
	$(ZRNF_MODULES) \
	$(ZRRANKS_MODULES) \
	$(ZRSUMS_MODULES) \
	$(ZRSUMSF_MODULES) \
	$(ZSAD_MODULES) \
	$(ZSB_MODULES) \
	$(ZSBF_MODULES) \
	$(ZSCRIPT_MODULES) \
	$(ZSEL_MODULES) \
	$(ZSID_MODULES) \
	$(ZSIGN_MODULES) \
	$(ZSING_MODULES) \
	$(ZSKCU_MODULES) \
	$(ZSKFI_MODULES) \
	$(ZSKFO_MODULES) \
	$(ZSKSE_MODULES) \
	$(ZSKSI_MODULES) \
	$(ZSKSQ_MODULES) \
	$(ZSL_MODULES) \
	$(ZSP_MODULES) \
	$(ZSPAN_MODULES) \
	$(ZSPANMSP_MODULES) \
	$(ZSPF_MODULES) \
	$(ZSS_MODULES) \
	$(ZSUMS_MODULES) \
	$(ZSUMSF_MODULES) \
	$(ZSYMSQ_MODULES) \
	$(ZTCO_MODULES) \
	$(ZTCV_MODULES) \
	$(ZTE_MODULES) \
	$(ZTMU_MODULES) \
	$(ZTR_MODULES) \
	$(ZTRECOV_MODULES) \
	$(ZTRACE_MODULES) \
	$(ZTSP_MODULES) \
	$(ZTSPF_MODULES) \
	$(ZVP_MODULES) \
	$(ZWORDS_MODULES)

include dirs.txt

.PRECIOUS: $(GENFILES)

SRCDIR=.
# Compiler search path for headers
INCLUDES=-I. -I$(GENDIR) -I$(SRCDIR) -I $(SRCDIR)/$(OS)
# Preprocessor defined values
DEFINES=-DMEM_SIZE=200 -DCACHE_SIZE=1000

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

TARGET:=DECOMP
include targets.txt

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

TARGET:=ZAH
include targets.txt

TARGET:=ZBASE
include targets.txt

TARGET:=ZCHAR
include targets.txt

TARGET:=ZCHECK
include targets.txt

TARGET:=ZCLEAN
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

TARGET:=ZEXPORT
include targets.txt

TARGET:=ZFE
include targets.txt

TARGET:=ZFLN
include targets.txt

TARGET:=ZFLNF
include targets.txt

TARGET:=ZFO
include targets.txt

TARGET:=ZID
include targets.txt

TARGET:=ZIMPORT
include targets.txt

TARGET:=ZIP
include targets.txt

TARGET:=ZIV
include targets.txt

TARGET:=ZIVF
include targets.txt

TARGET:=ZJOIN
include targets.txt

TARGET:=ZLP
include targets.txt

TARGET:=ZLV
include targets.txt

TARGET:=ZMSB
include targets.txt

TARGET:=ZMSBF
include targets.txt

TARGET:=ZMSP
include targets.txt

TARGET:=ZMSPF
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

TARGET:=ZPCO
include targets.txt

TARGET:=ZPCV
include targets.txt

TARGET:=ZPR
include targets.txt

TARGET:=ZPRIME
include targets.txt

TARGET:=ZPRO
include targets.txt

TARGET:=ZPROJ
include targets.txt

TARGET:=ZPS
include targets.txt

TARGET:=ZQF
include targets.txt

TARGET:=ZQS
include targets.txt

TARGET:=ZRAND
include targets.txt

TARGET:=ZRANKS
include targets.txt

TARGET:=ZRE
include targets.txt

TARGET:=ZRES
include targets.txt

TARGET:=ZRN
include targets.txt

TARGET:=ZRNF
include targets.txt

TARGET:=ZRRANKS
include targets.txt

TARGET:=ZRSUMS
include targets.txt

TARGET:=ZRSUMSF
include targets.txt

TARGET:=ZSAD
include targets.txt

TARGET:=ZSB
include targets.txt

TARGET:=ZSBF
include targets.txt

TARGET:=ZSCRIPT
include targets.txt

TARGET:=ZSEL
include targets.txt

TARGET:=ZSID
include targets.txt

TARGET:=ZSING
include targets.txt

TARGET:=ZSIGN
include targets.txt

TARGET:=ZSKCU
include targets.txt

TARGET:=ZSKFI
include targets.txt

TARGET:=ZSKFO
include targets.txt

TARGET:=ZSKSE
include targets.txt

TARGET:=ZSKSI
include targets.txt

TARGET:=ZSKSQ
include targets.txt

TARGET:=ZSL
include targets.txt

TARGET:=ZSP
include targets.txt

TARGET:=ZSPAN
include targets.txt

TARGET:=ZSPANMSP
include targets.txt

TARGET:=ZSPF
include targets.txt

TARGET:=ZSS
include targets.txt

TARGET:=ZSUMS
include targets.txt

TARGET:=ZSUMSF
include targets.txt

TARGET:=ZSYMSQ
include targets.txt

TARGET:=ZTCO
include targets.txt

TARGET:=ZTCV
include targets.txt

TARGET:=ZTE
include targets.txt

TARGET:=ZTMU
include targets.txt

TARGET:=ZTR
include targets.txt

TARGET:=ZTRACE
include targets.txt

TARGET:=ZTRECOV
include targets.txt

TARGET:=ZTSP
include targets.txt

TARGET:=ZTSPF
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
