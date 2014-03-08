###############################################################################
#                             Legend of Pandora Makefile [Linux x86] Ver. 1.0 #
###############################################################################

##### These will carry over when going back into the Makefile #################
ifndef TOPDIR
export TOPDIR		:= $(shell cd ../../ && pwd)
export TOPSRC		= $(TOPDIR)/source
export TARGETDIR	= $(TOPDIR)/bin/$(PLATFORM)/$(ARCH)/$(BITSIZE)
export OBJSDIR		= $(TOPDIR)/obj/$(PLATFORM)/$(ARCH)/$(BITSIZE)/gcc/$(GCCVER)/$(BUILD)
endif

##### These directories are relative to the one you instantiate make from #####
SOURCEDIR	= source ../common/source

##### Rename this if you want #################################################
PROJECT := legend-of-pandora
##### Depending on the build target, you may have a D (debug) or P (profile) ##
##### character appended to the executable, plus the verison of GCC used to  ##
##### create the executable                                                  ##  
TARGET := $(PROJECT)
OUTFILE	=	$(TARGETDIR)/$(TARGET)_$(GCCVER)

##### notdir will get the last item after the directory delimeter, in this   ##
##### case it is the directory we are in: $(CURDIR)                          ##
PLATFORM		:= $(notdir $(CURDIR))
BUILD_PLATFORM	:= LINUX

##### By default, a 32-bit x86 executable will be created, BUILD_ARCH is     ##
##### used by the build flags, whereas ARCH is for directories created       ##
BUILD_ARCH		:= X86
ARCH			:= x86
BITSIZE			:= 32

GCCVER	= $(shell $(CC) -dumpversion)

BUILD_TYPE := debug

32BIT ?= False
64BIT ?= False

##### Get the machine type to determine which set of libraries to use #########
UNAME			= $(shell uname)
UNAME_MACHINE	= $(shell uname -m)

ifeq ($(UNAME), Linux)
	CFLAGS_EXT	=
	SYSIPATH	=
	LINKFLAGS	=
##### Check for forced 32-bit or 64-bit builds ################################
ifneq ($(64BIT), False)
	ARCH		=	x86
	BITSIZE		=	64
else
ifneq ($(32BIT), False)
	ARCH		= x86
	BITSIZE		= 32
else
##### Nothing forced.  Use what the machine reports ###########################
ifeq ($(UNAME_MACHINE), x86_64)
	ARCH		=	x86
	BITSIZE		=	64
endif
ifeq ($(UNAME_MACHINE), i686)
	ARCH		= x86
	BITSIZE		= 32
endif
endif
endif
endif

CC = gcc
CPP = g++

##### -c will just compile the C file and generate an object file instead of ##
##### trying to link all the source files which is done at the end           ##
##### -m allows you to specify the bit size to compile for, which can be     ##
##### useful for compiling a 32-bit executable from a 64-bit compiler        ##
##### The -D* flags are preprocessor definitions (which are checked as:      ##
##### #ifdef <CHARACTERS AFTER -D>                                           ##
##### ...                                                                    ##
##### #endif                                                                 ##
CFLAGS	=	-c -m$(BITSIZE) -DLOP_BUILD_$(BUILD_DEF) \
				-DLOP_PLATFORM_$(BUILD_PLATFORM) \
				-DLOP_PLATFORM_$(BUILD_PLATFORM)_$(BUILD_ARCH) \
				-DLOP_PLATFORM_$(BUILD_PLATFORM)_$(BUILD_ARCH)_$(BITSIZE) \
				-DLOP_BITSIZE_$(BITSIZE) -DLOP_ARCH_$(BUILD_ARCH) 
SYSIPATH	=
INCPATH		=	-I$(TOPSRC)/include
SYSLPATH	=	
LIBPATH		=	
SYSLIBS		=	-lSDL_mixer
LINKFLAGS	=

##### Debug #######
debug:		BUILD		= Debug
debug:		BUILD_TYPE	= debug
debug:		BUILD_DEF	= DEBUG
debug:		TARGET := $(TARGET)D
debug:		CFLAGS += -g -ggdb -Wall -D_DEBUG $(CFLAGS_EXT)
debug:		LIBS = 
debug:		$(TARGET)

##### Release #####
release:	BUILD		= Release
release:	BUILD_TYPE	= release
release:	BUILD_DEF	= RELEASE
release:	TARGET := $(TARGET)
release:	CFLAGS += -O3 $(CFLAGS_EXT)
release:	LIBS =
release:	LINKFLAGS += -Wl,-S
release:	$(TARGET)

##### Profile #####
profile:	BUILD		= Profile
profile:	BUILD_TYPE	= profile
profile:	BUILD_DEF	= PROFILE
profile:	TARGET := $(TARGET)P
profile:	CFLAGS += -O3 -g -ggdb -Wall -D_DEBUG $(CFLAGS_EXT)
profile:	LIBS =
profile:	$(TARGET)

##### Build the object files while not in the Obj directory ###################
ifneq ($(OBJSDIR), $(CURDIR))

VERSIONINFO:
	@printf "%s" "------------------------- Generating Version Information ---"
	@printf "%s\n" "-------------------"
	@mkdir -p include
	@sh ../gitversion.sh ./include/gitversion.h $(PROJECT)
	@printf "%s" "------------------------------------------------------------"
	@printf "%s\n" "-------------------"

TARGETDIR:
	@mkdir -p $(TARGETDIR)

OBJSDIR:
	@mkdir -p $(OBJSDIR)


##### This will store all the *.c files found in the $(SOURCEDIR), excluding ##
##### the full path, instead just getting the file name                      ##
CFILES	:= $(foreach dir,$(SOURCEDIR),$(notdir $(wildcard $(dir)/*.c)))
export VPATH	:= $(foreach dir,$(SOURCEDIR),$(CURDIR)/$(dir))
##### The previously obtained *.c files are converted to their object        ##
##### counterpart names, with main.c becoming main.o, for example            ##
export OBJS		:= $(CFILES:.c=.o)

$(TARGET): OBJSDIR TARGETDIR VERSIONINFO
	@/usr/bin/time -f "%E" -o $(OBJSDIR)/compiletime $(MAKE) \
	--no-print-directory -C $(OBJSDIR) -f $(TOPSRC)/$(PLATFORM)/Makefile \
	$(BUILD_TYPE)
	@printf "\n%s" "----------------------------------------------------------"
	@printf "%s\n" "---------------------"
	@printf "Compile time: "
	@cat $(OBJSDIR)/compiletime
	@printf "%s" "------------------------------------------------------------"
	@printf "%s\n" "-------------------"

else

##### In the intermediate build directory #####################################

$(TARGET): $(OBJS)
	@printf "\n\n%s" "------------------------- MESSAGES FROM THE COMPILER ---"
	@printf "%s\n" "-----------------------"
	@cat *.cmsgs
	@printf "%s" "------------------------------------------------------------"
	@printf "%s\n\n" "-------------------"
	@printf "Creating: $(OUTFILE)... "
	@$(CC) -o $(OUTFILE) $(LINKFLAGS) -m$(BITSIZE) $(OBJS) $(SYSLPATH) \
	$(LIBPATH)  `sdl-config --libs` $(LIBS) $(SYSLIBS) 2> lmsgs;\
	RETVAL=$$?;\
	if env | grep -q ^TERM=; then\
		COLOURSUPPORT=`tput colors`;\
		if [ $$COLOURSUPPORT -gt -1 ]; then\
			cat lmsgs | sed ''/error/s//`printf "\033[31merror\033[0m"`/'' > lmsgs;\
			cat lmsgs | sed ''/warning/s//`printf "\034[31mwarning\033[0m"`/'' > lmsgs;\
		fi;\
	fi;\
	if [[ $$RETVAL == 0 ]]; then\
		printf "[OK]\n";\
	else\
		printf "\n\n%s" "------------------------------------ ERROR --------";\
		printf "%s\n" "----------------------------";\
		cat lmsgs;\
		printf "%s" "-------------------------------------------------------";\
		printf "%s\n" "------------------------";\
		exit 1;\
	fi
	@printf "Copying content to $(TARGETDIR)\n"
	@cp -rv $(TOPDIR)/content/* $(TARGETDIR)/

%.o: %.c
	@printf "Compiling: $<... "
	@$(CC) -MMD -MP -MF $*.d $(CFLAGS) `sdl-config --cflags` $(SYSIPATH) $(INCPATH) \
	$< -o $@ 2> $*.cmsgs;\
	RETVAL=$$?;\
	cat $*.cmsgs | sed 's|$(TOPDIR)||g' > $*.cmsgstmp;\
	if env | grep -q ^TERM=; then\
		COLOURSUPPORT=`tput colors`;\
		if [ $$COLOURSUPPORT -gt -1 ]; then\
			cat $*.cmsgstmp | sed ''/error/s//`printf "\033[31merror\033[0m"`/'' > $*.cmsgs;\
			cat $*.cmsgs | sed ''/warning/s//`printf "\033[34mwarning\033[0m"`/'' > $*.cmsgs;\
		fi;\
	fi;\
	if [[ $$RETVAL == 0 ]]; then\
		printf "[OK]\n";\
	else\
		printf "\n\n%s" "--------------------------------- ERROR -----------";\
		printf "%s\n" "----------------------------";\
		cat $*.cmsgs;\
		printf "%s" "-------------------------------------------------------";\
		printf "%s\n\n" "------------------------";\
		exit 1;\
	fi

-include $(OBJSDIR)/*.d

endif

.PHONY: clean completeclean
clean:
	@printf "Removing files from: $(TARGETDIR)... "
	@rm -f $(TARGETDIR)/*
	@printf "[OK]\n"
	@printf "Removing files from: $(OBJSDIR)... "
	@rm -rf $(OBJSDIR)/*
	@printf "[OK]\n"

completeclean:
	@printf "Removing files from: $(TOPDIR)/Bin... "
	@rm -rf $(TOPDIR)/Bin
	@printf "[OK]\n"
	@printf "Removing files from: $(TOPDIR)/Obj... "
	@rm -rf $(TOPDIR)/Obj
	@printf "[OK]\n"

