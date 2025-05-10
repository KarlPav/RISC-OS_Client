# Setup RISCOS GCC environment
TARGET_ENV = . ~/gccsdk/env/ro-path

# Compiler and linker
CC := gcc
LD := gcc

# Setup some standard compiler flags.
#RISCOS flags:
# -mpoke-function-name enables function names to exist in the object file (poke function name)
# -mthrowback is used to enable throwback error reporting
# -mlibscl is used to link against the SCLib library (Shared C Library) - Creates a RISCOS executable (AIF Type)

#ARM flags:
# -mhard-float is used to enable hardware floating point support
# -mfloat-abi=hard is used to specify the floating point ABI (Application Binary Interface)
# -mfpu= sets which floating point unit to use. If using libscl, hardfp options are fpa only, soft options are neon or vfp

#UnixLib flags:
# -lunixlib is used to link against the UnixLib library - Creates a UnixLib executable (ELF Type)

#GCC flags:
# -Wall enables all compiler warnings
# -O2 enables optimization level 2
# -fno-strict-aliasing disables strict aliasing optimizations (Allows more flexible casting)
# -static links the executable statically
# -dynamic enables dynamic linking (opposite of static)
# -std=gnu99 enables GNU99 standard for C

CCFLAGS += -std=gnu99 -mlibscl -mhard-float -mthrowback -Wall -O2 -fno-strict-aliasing -mpoke-function-name -static
#CXXFLAGS += -Wall -O2 #-mfloat-abi=hard #-static
LDFLAGS +=

# Set target
# TARGET := !RunImage
TARGET := !RunImageELF
AIF := !RunImage

# Relative paths to our source, header and object file directories.
SRCDIR := $(CURDIR)/c
HDRDIR := $(CURDIR)/h
OBJDIR := $(CURDIR)/o

# The C++ Headers and libraries live here.
INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include -I$(HDRDIR)
LINKS := -L$(GCCSDK_INSTALL_ENV)/lib -lOSLibH32 -lSFLib32 -lunixlib

# Ensure the directory exists
$(shell mkdir -p $(SRCDIR))
$(shell mkdir -p $(HDRDIR))
$(shell mkdir -p $(OBJDIR))

# List of source files and corresponding object files and the target
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

# Debugging: Print the source and object files
$(info Source files: $(SRCS))
$(info Object files: $(OBJS))

# Phony targets
.PHONY: all clean

# Default target
# all: $(TARGET)
all: $(AIF)

# Compile the C++ sources into ELF object files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@$(TARGET_ENV) && $(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@

# Link the ELF object files into an RISCOS ELF executable.
$(TARGET): $(OBJS)
	@$(TARGET_ENV) && $(CC) $(CCFLAGS) $(LINKS) $(LDFLAGS) -o $(TARGET) $(OBJS)

# # Convert ELF to AIF - Must use -static
# $(AIF): $(TARGET)
# 	/home/piOS/gccsdk/cross/bin/elf2aif $(TARGET) $(AIF)
# 	rm -f $(TARGET)

# Convert ELF to AIF - Must use -static
$(AIF): $(TARGET)
	@echo "Converting ELF to AIF..."
	@/home/piOS/gccsdk/cross/bin/elf2aif $(TARGET) $(AIF) 2> aif_error.log || \
	( \
		if grep -q "'$(TARGET)' is not an ELF file" aif_error.log; then \
			echo "$(TARGET) is already an AIF binary � renaming to $(AIF)"; \
			mv -f $(TARGET) $(AIF); \
		else \
			cat aif_error.log; \
			false; \
		fi \
	)

# Clean the build directory
clean:
	@$(TARGET_ENV) && rm -f $(OBJDIR)/*.o $(TARGET)
