#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.112 2012/01/24 22:15:43 jon Exp $
#
all: debug rel profile profilena

GENERATED=9.c

.PHONY: debug rel profile profilena clean full_clean relpp relasm

CONS_TARGET=	cons
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
MAKE_TARGET=	maketab
MON_TARGET=	monst
SIZES_TARGET=	sizes
SNS_TARGET=	sns
SRN_TARGET=	srn
STOP_TARGET=	stop
ZAD_TARGET=	zad
ZAH_TARGET=	zah
ZBASE_TARGET=	zbase
ZCFL_TARGET=	zcfl
ZCHAR_TARGET=	zchar
ZCHECK_TARGET=	zcheck
ZCLEAN_TARGET=	zclean
ZCONJ_TARGET=	zconj
ZCT_TARGET=	zct
ZCV_TARGET=	zcv
ZCV32_TARGET=	zcv32to64
ZCV64_TARGET=	zcv64to32
ZDC_TARGET=	zdc
ZDIAG_TARGET=	zdiag
ZDIFF_TARGET=	zdiff
ZDIFFD_TARGET=	zdiffd
ZEX_TARGET=	zex
ZEXPORT_TARGET=	zexport
ZFE_TARGET=	zfe
ZFLN_TARGET=	zfln
ZFLNF_TARGET=	zflnf
ZFN_TARGET=	zfn
ZFO_TARGET=	zfo
ZFOL_TARGET=	zfol
ZFORMAT_TARGET=	zformat
ZFR_TARGET=	zfr
ZID_TARGET=	zid
ZIMPORT_TARGET=	zimport
ZIP_TARGET=	zip
ZIV_TARGET=	ziv
ZIVF_TARGET=	zivf
ZJOIN_TARGET=	zjoin
ZLP_TARGET=	zlp
ZLPF_TARGET=	zlpf
ZLV_TARGET=	zlv
ZMSB_TARGET=	zmsb
ZMSBF_TARGET=	zmsbf
ZMSP_TARGET=	zmsp
ZMSPF_TARGET=	zmspf
ZMU_TARGET=	zmu
ZMVP_TARGET=	zmvp
ZNOC_TARGET=	znoc
ZNOR_TARGET=	znor
ZNS_TARGET=	zns
ZNSF_TARGET=	znsf
ZNTCO_TARGET=	zntco
ZPCO_TARGET=	zpco
ZPCOL_TARGET=	zpcol
ZPFL_TARGET=	zpfl
ZPOFP_TARGET=	zpofp
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
ZSCRIPT_TARGET=	zscript
ZSEL_TARGET=	zsel
ZSID_TARGET=	zsid
ZSIGN_TARGET=	zsign
ZSIGNF_TARGET=	zsignf
ZSING_TARGET=	zsing
ZSIZE_TARGET=	zsize
ZSKCU_TARGET=	zskcu
ZSKFI_TARGET=	zskfi
ZSKFO_TARGET=	zskfo
ZSKSE_TARGET=	zskse
ZSKSI_TARGET=	zsksi
ZSKSQ_TARGET=	zsksq
ZSL_TARGET=	zsl
ZSPAN_TARGET=	zspan
ZSPANMSP_TARGET=	zspanmsp
ZSS_TARGET=	zss
ZSUMS_TARGET=	zsums
ZSUMSF_TARGET=	zsumsf
ZSV_TARGET=	zsv
ZSYMB_TARGET=	zsymb
ZSYMSQ_TARGET=	zsymsq
ZTCO_TARGET=	ztco
ZTCOCON_TARGET=	ztcocon
ZTCV_TARGET=	ztcv
ZTE_TARGET=	zte
ZTMU_TARGET=	ztmu
ZTR_TARGET=	ztr
ZTRACE_TARGET=	ztrace
ZTRECOV_TARGET=	ztrecover
ZTSP_TARGET=	ztsp
ZTSPF_TARGET=	ztspf
ZTTR_TARGET=	zttr
ZVP_TARGET=	zvp
ZVPF_TARGET=	zvpf
ZWORDS_TARGET=	zwords

LIB_MODULES=	add clean clean_file clean_vectors command conj count dc dets diff diffd elements endian exrows extend extend_matrix files gen grease header ident join map map_or_row maps matrix memory msb msbf msp mspf mul mv mvp ns nsf ntco orbit parse pco pcol pfl pofp powers primes project ps qs rand read restrict retract retract_matrix rn rnf scale script singular span spanmsp ss ss_map sums sumsf sums_utils sv symb system tco te tmul tra tsp tspf ttr utils vp vpf write

CONS_MODULES=	constrain
DTOU_MODULES=	dtou
DECOMP_MODULES=	decomp
EAD_MODULES=	ead rows
ECT_MODULES=	ect
EID_MODULES=	eid rows
EIM_MODULES=	eim
EIP_MODULES=	eip rows
EMU_MODULES=	emu
ESAD_MODULES=	esad rows
ESID_MODULES=	esid rows
ETR_MODULES=	etr rows
MAKE_MODULES=	maketab primes utils
MON_MODULES=	mmat mop mtx rows
SIZES_MODULES=	sizes
SNS_MODULES=	rows sns
SRN_MODULES=	rows srn
STOP_MODULES=	stop
ZAD_MODULES=	ad rows
ZAH_MODULES=	zah
ZBASE_MODULES=	base rows zbase
ZCFL_MODULES=	cfl rows zcfl
ZCHAR_MODULES=	zchar
ZCHECK_MODULES=	zcheck
ZCLEAN_MODULES=	rows zclean
ZCONJ_MODULES=	zconj
ZCT_MODULES=	ct
ZCV_MODULES=	ip
ZCV32_MODULES=	zcv32to64
ZCV64_MODULES=	zcv64to32
ZDC_MODULES=	rows zdc
ZDIAG_MODULES=	rows zdiag
ZDIFF_MODULES=	rows zdiff
ZDIFFD_MODULES=	rows zdiffd
ZEX_MODULES=	rows zex
ZEXPORT_MODULES=	rows zexport
ZFE_MODULES=	rows zfe
ZFLN_MODULES=	rows zfln
ZFLNF_MODULES=	rows zflnf
ZFN_MODULES=	fn rows zfn
ZFO_MODULES=	rows zfo
ZFOL_MODULES=	rows zfol
ZFORMAT_MODULES=	zformat
ZFR_MODULES=	rows zfr
ZID_MODULES=	id rows
ZIMPORT_MODULES=	rows zimport
ZIP_MODULES=	ipp rows
ZIV_MODULES=	iv rows ziv
ZIVF_MODULES=	ivf rows zivf
ZJOIN_MODULES=	rows zjoin
ZLP_MODULES=	rows zlp
ZLPF_MODULES=	rows zlpf
ZLV_MODULES=	lv rows zlv
ZMSB_MODULES=	rows zmsb
ZMSBF_MODULES=	rows zmsbf
ZMSP_MODULES=	rows zmsp
ZMSPF_MODULES=	rows zmspf
ZMU_MODULES=	mu rows
ZMVP_MODULES=	rows zmvp
ZNOC_MODULES=	znoc
ZNOR_MODULES=	znor
ZNS_MODULES=	rows zns
ZNSF_MODULES=	rows znsf
ZNTCO_MODULES=	rows zntco
ZPCO_MODULES=	rows zpco
ZPCOL_MODULES=	rows zpcol
ZPCV_MODULES=	pcv rows zpcv
ZPFL_MODULES=	zpfl
ZPOFP_MODULES=	zpofp rows
ZPR_MODULES=	pr rows
ZPRIME_MODULES=	zprime
ZPRO_MODULES=	zpro
ZPROJ_MODULES=	rows zproj
ZPS_MODULES=	zps
ZQF_MODULES=	rows zqf
ZQS_MODULES=	rows zqs
ZRAND_MODULES=	rows zrand
ZRANKS_MODULES=	rows zranks
ZRE_MODULES=	zre
ZRES_MODULES=	rows zrestrict
ZRN_MODULES=	rows zrn
ZRNF_MODULES=	rows zrnf
ZRRANKS_MODULES=	rows zrranks
ZRSUMS_MODULES=	rows zrsums
ZRSUMSF_MODULES=	rows zrsumsf
ZSAD_MODULES=	rows zsad
ZSCRIPT_MODULES=	rows zscript
ZSEL_MODULES=	zse
ZSID_MODULES=	rows zsid
ZSIGN_MODULES=	sign rows zsign
ZSIGNF_MODULES=	signf rows zsignf
ZSING_MODULES=	rows zsing
ZSIZE_MODULES=	zsize
ZSKCU_MODULES=	rows zskcu
ZSKFI_MODULES=	rows zskfi
ZSKFO_MODULES=	rows zskfo
ZSKSE_MODULES=	rows zskse
ZSKSI_MODULES=	rows zsksi
ZSKSQ_MODULES=	rows zsksq
ZSL_MODULES=	rows slave
ZSPAN_MODULES=	rows zspan
ZSPANMSP_MODULES=	rows zspanmsp
ZSS_MODULES=	rows zss
ZSUMS_MODULES=	rows zsums
ZSUMSF_MODULES=	rows zsumsf
ZSV_MODULES=	rows zsv
ZSYMB_MODULES=	rows zsymb
ZSYMSQ_MODULES=	rows zsymsq
ZTCO_MODULES=	rows ztco
ZTCOCON_MODULES=	rows ztcocon
ZTCV_MODULES=	tcv rows ztcv
ZTE_MODULES=	rows zte
ZTMU_MODULES=	rows ztmu
ZTR_MODULES=	rows tr
ZTRACE_MODULES=	rows ztrace
ZTRECOV_MODULES=	rows ztrecover
ZTSP_MODULES=	rows ztsp
ZTSPF_MODULES=	rows ztspf
ZTTR_MODULES=	rows zttr
ZVP_MODULES=	rows zvp
ZVPF_MODULES=	rows zvpf
ZWORDS_MODULES=	rows zwords

MODULES=	$(LIB_MODULES) \
	$(CONS_MODULES) \
	$(DECOMP_MODULES) \
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
	$(MAKE_MODULES) \
	$(MON_MODULES) \
	$(SIZES_MODULES) \
	$(SNS_MODULES) \
	$(SRN_MODULES) \
	$(STOP_MODULES) \
	$(ZAD_MODULES) \
	$(ZAH_MODULES) \
	$(ZBASE_MODULES) \
	$(ZCFL_MODULES) \
	$(ZCHAR_MODULES) \
	$(ZCHECK_MODULES) \
	$(ZCLEAN_MODULES) \
	$(ZCONJ_MODULES) \
	$(ZCT_MODULES) \
	$(ZCV_MODULES) \
	$(ZCV32_MODULES) \
	$(ZCV64_MODULES) \
	$(ZDC_MODULES) \
	$(ZDIAG_MODULES) \
	$(ZDIFF_MODULES) \
	$(ZDIFFD_MODULES) \
	$(ZEX_MODULES) \
	$(ZEXPORT_MODULES) \
	$(ZFE_MODULES) \
	$(ZFLN_MODULES) \
	$(ZFLNF_MODULES) \
	$(ZFN_MODULES) \
	$(ZFO_MODULES) \
	$(ZFOL_MODULES) \
	$(ZFORMAT_MODULES) \
	$(ZFR_MODULES) \
	$(ZID_MODULES) \
	$(ZIMPORT_MODULES) \
	$(ZIP_MODULES) \
	$(ZIV_MODULES) \
	$(ZIVF_MODULES) \
	$(ZJOIN_MODULES) \
	$(ZLP_MODULES) \
	$(ZLPF_MODULES) \
	$(ZLV_MODULES) \
	$(ZMSB_MODULES) \
	$(ZMSBF_MODULES) \
	$(ZMSP_MODULES) \
	$(ZMSPF_MODULES) \
	$(ZMU_MODULES) \
	$(ZMVP_MODULES) \
	$(ZNOC_MODULES) \
	$(ZNOR_MODULES) \
	$(ZNS_MODULES) \
	$(ZNSF_MODULES) \
	$(ZNTCO_MODULES) \
	$(ZPCO_MODULES) \
	$(ZPCOL_MODULES) \
	$(ZPCV_MODULES) \
	$(ZPFL_MODULES) \
	$(ZPOFP_MODULES) \
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
	$(ZSCRIPT_MODULES) \
	$(ZSEL_MODULES) \
	$(ZSID_MODULES) \
	$(ZSIGN_MODULES) \
	$(ZSIGNF_MODULES) \
	$(ZSING_MODULES) \
	$(ZSIZE_MODULES) \
	$(ZSKCU_MODULES) \
	$(ZSKFI_MODULES) \
	$(ZSKFO_MODULES) \
	$(ZSKSE_MODULES) \
	$(ZSKSI_MODULES) \
	$(ZSKSQ_MODULES) \
	$(ZSL_MODULES) \
	$(ZSPAN_MODULES) \
	$(ZSPANMSP_MODULES) \
	$(ZSS_MODULES) \
	$(ZSUMS_MODULES) \
	$(ZSUMSF_MODULES) \
	$(ZSV_MODULES) \
	$(ZSYMB_MODULES) \
	$(ZSYMSQ_MODULES) \
	$(ZTCO_MODULES) \
	$(ZTCOCON_MODULES) \
	$(ZTCV_MODULES) \
	$(ZTE_MODULES) \
	$(ZTMU_MODULES) \
	$(ZTR_MODULES) \
	$(ZTRECOV_MODULES) \
	$(ZTRACE_MODULES) \
	$(ZTSP_MODULES) \
	$(ZTSPF_MODULES) \
	$(ZTTR_MODULES) \
	$(ZVP_MODULES) \
	$(ZVPF_MODULES) \
	$(ZWORDS_MODULES)

include dirs.txt
include defines.txt

.PRECIOUS: $(GENFILES)

SRCDIR=.
# Compiler search path for headers
INCLUDES=-I. -I$(GENDIR) -I$(SRCDIR) -I $(SRCDIR)/$(OS)
# Preprocessor defined values
DEFINES=-DMEM_SIZE=200 -DCACHE_SIZE=1000

# Make search paths for source
vpath %.c $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)
vpath %.h $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)

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

# Parameterically generate the link commands
CLEAN_ITEMS:=$(DEPENDS) $(GENFILES)

REL_TARGETS:=
DEBUG_TARGETS:=
PROF_TARGETS:=
PROFNA_TARGETS:=
RELPP_TARGETS:=
RELASM_TARGETS:=

MTXLIB:=	libmtx
include libs.txt

TARGET:=CONS
include targets.txt

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

TARGET:=MAKE
TARGET_TYPE:=DEBUG
include nonlibtarget.txt

TARGET:=MON
include targets.txt

TARGET:=SIZES
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

TARGET:=ZCFL
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

TARGET:=ZCV32
include targets.txt

TARGET:=ZCV64
include targets.txt

TARGET:=ZDIAG
include targets.txt

TARGET:=ZDC
include targets.txt

TARGET:=ZDIFF
include targets.txt

TARGET:=ZDIFFD
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

TARGET:=ZFN
include targets.txt

TARGET:=ZFOL
include targets.txt

TARGET:=ZFORMAT
include targets.txt

TARGET:=ZFR
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

TARGET:=ZLPF
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

TARGET:=ZMVP
include targets.txt

TARGET:=ZNOC
include targets.txt

TARGET:=ZNOR
include targets.txt

TARGET:=ZNS
include targets.txt

TARGET:=ZNSF
include targets.txt

TARGET:=ZNTCO
include targets.txt

TARGET:=ZPCO
include targets.txt

TARGET:=ZPCOL
include targets.txt

TARGET:=ZPCV
include targets.txt

TARGET:=ZPFL
include targets.txt

TARGET:=ZPOFP
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

TARGET:=ZSCRIPT
include targets.txt

TARGET:=ZSEL
include targets.txt

TARGET:=ZSID
include targets.txt

TARGET:=ZSING
include targets.txt

TARGET:=ZSIZE
include targets.txt

TARGET:=ZSIGN
include targets.txt

TARGET:=ZSIGNF
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

TARGET:=ZSPAN
include targets.txt

TARGET:=ZSPANMSP
include targets.txt

TARGET:=ZSS
include targets.txt

TARGET:=ZSUMS
include targets.txt

TARGET:=ZSUMSF
include targets.txt

TARGET:=ZSV
include targets.txt

TARGET:=ZSYMB
include targets.txt

TARGET:=ZSYMSQ
include targets.txt

TARGET:=ZTCO
include targets.txt

TARGET:=ZTCOCON
include targets.txt

TARGET:=ZTCV
include targets.txt

TARGET:=ZTE
include targets.txt

TARGET:=ZTMU
include targets.txt

TARGET:=ZTR
include targets.txt

TARGET:=ZTTR
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

TARGET:=ZVPF
include targets.txt

TARGET:=ZWORDS
include targets.txt

debug: $(DEBUG_TARGETS)

rel: $(REL_TARGETS)
relpp: $(RELPP_TARGETS)
relasm: $(RELASM_TARGETS)

profile: $(PROF_TARGETS)

profilena: $(PROFNA_TARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
