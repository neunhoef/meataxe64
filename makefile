#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.10 2001/09/18 23:15:46 jon Exp $
#
all: debug rel profile profilena

GENERATED=

.PHONY: debug rel profile profilena clean full_clean

# Default arch and os, can be overridden by command line
OS=unix
ARCH=i386

AD_TARGET=	ad
DTOU_TARGET=	dtou
ID_TARGET=	id
IP_TARGET=	ip
MON_TARGET=	monst
MU_TARGET=	mu
PR_TARGET=	pr

AD_MODULES=	ad add elements endian header memory primes read rows utils write
ID_MODULES=	id elements endian header memory primes rows utils write
IP_MODULES=	elements endian header ip primes read utils write
MU_MODULES=	elements endian grease header matrix memory mu mul primes read rows utils write
MON_MODULES=	endian header mmat mop mtx primes utils write
PR_MODULES=	elements endian header memory pr primes read rows utils write
DTOU_MODULES=	dtou

MODULES=	$(AD_MODULES) $(DTOU_MODULES) $(ID_MODULES) $(IP_MODULES) $(MON_MODULES) $(MU_MODULES) $(PR_MODULES) $(DTOU_MODULES)

#
# Modify this from the command line to change where derived files go
#
BASEDIR=derived

OSARCHBASEDIR=$(BASEDIR)/$(OS)/$(ARCH)

# Keep separate bases for different OSes
# If we have different architectures, extend by $(ARCH)

# Release optimised no asserts
RELOBJDIR=$(OSARCHBASEDIR)/obj
RELBINDIR=$(OSARCHBASEDIR)/bin
# Debug, asserts
DEBUGOBJDIR=$(OSARCHBASEDIR)/debugobj
DEBUGBINDIR=$(OSARCHBASEDIR)/debugbin
# Profiling, debug, asserts
PROFOBJDIR=$(OSARCHBASEDIR)/profobj
PROFBINDIR=$(OSARCHBASEDIR)/profbin
#Profiing, debug, without asserts
PROFNAOBJDIR=$(OSARCHBASEDIR)/profnaobj
PROFNABINDIR=$(OSARCHBASEDIR)/profnabin

DEPENDDIR=$(OSARCHBASEDIR)/depends
GENDIR=$(OSARCHBASEDIR)/gen
GENFILES=$(GENERATED:%=$(GENDIR)/%)

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

RELTARGETS:=
DEBUGTARGETS:=
PROFTARGETS:=
PROFNATARGETS:=

TARGET:=AD
include targets.txt

TARGET:=DTOU
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

debug: $(DEBUGTARGETS)

rel: $(RELTARGETS)

profile: $(PROFTARGETS)

profilena: $(PROFNATARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
