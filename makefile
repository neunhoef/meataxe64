#
# meataxe makefile for building on multiple targets
#
# $Id: makefile,v 1.1 2001/08/28 21:39:44 jon Exp $
#
all: debug rel profile profilena

GENERATED=

.PHONY: debug rel profile profilena clean full_clean

# Default arch and os, can be overridden by command line
OS= unix
ARCH= i386

IP_TARGET=	ip
DTOU_TARGET=	dtou

TARGETS=	$(IP_TARGET) $(DTOU_TARGET)

IP_MODULES=	elements endian header ip primes read utils write 
DTOU_MODULES=	dtou

MODULES=	$(IP_MODULES) $(DTOU_MODULES)

#
# Modify this from the command line to change where derived files go
#
BASEDIR= derived

OSARCHBASEDIR= $(BASEDIR)/$(OS)/$(ARCH)

# Keep separate bases for different OSes
# If we have different architectures, extend by $(ARCH)

# Release optimised no asserts
RELOBJDIR= $(OSARCHBASEDIR)/obj
RELBINDIR= $(OSARCHBASEDIR)/bin
# Debug, asserts
DEBUGOBJDIR= $(OSARCHBASEDIR)/debugobj
DEBUGBINDIR= $(OSARCHBASEDIR)/debugbin
# Profiling, debug, asserts
PROFOBJDIR= $(OSARCHBASEDIR)/profobj
PROFBINDIR= $(OSARCHBASEDIR)/profbin
#Profiing, debug, without asserts
PROFNAOBJDIR= $(OSARCHBASEDIR)/profnaobj
PROFNABINDIR= $(OSARCHBASEDIR)/profnabin

DEPENDDIR= $(OSARCHBASEDIR)/depends
GENDIR= $(OSARCHBASEDIR)/gen
GENFILES= $(GENERATED:%=$(GENDIR)/%)

.PRECIOUS: $(GENFILES)

SRCDIR= .
# Compiler search path for headers
INCLUDES= -I. -I$(GENDIR) -I$(SRCDIR) -I $(SRCDIR)/$(OS)

# Get OS specific portion
# Can similarly have arch specific portion
include $(OS)/GNUmake
# Get implicit rules
include rules.txt
# Get dependencies. Don't include when we make clean or full_clean. Avoid the empty set
DEPENDS= $(MODULES:%=$(DEPENDDIR)/%.$(DEPENDEXT))
ifeq "$(findstring clean,$(MAKECMDGOALS))" ""
ifneq "$(strip $(DEPENDS))" ""
-include $(DEPENDS)
endif
endif

# Make search paths for source
vpath %.c . $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)
vpath %.h . $(SRCDIR) $(SRCDIR)/$(OS) $(GENDIR)

# Parameterically generate the link commands
CLEAN_ITEMS:= $(DEPENDS) $(GENFILES)

RELTARGETS:=
DEBUGTARGETS:=
PROFTARGETS:=
PROFNATARGETS:=

TARGET:=IP
TARGET_TYPE:=REL
include targets.txt

TARGET_TYPE:=DEBUG
include targets.txt

TARGET_TYPE:=PROF
include targets.txt

TARGET_TYPE:=PROFNA
include targets.txt

TARGET:=DTOU
TARGET_TYPE:=REL
include targets.txt

TARGET_TYPE:=DEBUG
include targets.txt

TARGET_TYPE:=PROF
include targets.txt

TARGET_TYPE:=PROFNA
include targets.txt

debug: $(DEBUGTARGETS)

rel: $(RELTARGETS)

profile: $(PROFTARGETS)

profilena: $(PROFNATARGETS)

clean:
	rm -rf $(CLEAN_ITEMS)

full_clean:
	rm -rf $(OSARCHBASEDIR)
