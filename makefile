# Output control
Q := @
ECHO := @echo

# Cross-compilation setup
GCCSDK_ROOT := $(HOME)/gccsdk
GCCSDK_CROSS := $(GCCSDK_ROOT)/cross/bin

# Default to native build
TARGET_OS ?= native

# Target-specific variables
ifeq ($(TARGET_OS),riscos)
    # RISC OS Toolchain
    CC := $(GCCSDK_CROSS)/arm-unknown-riscos-gcc
    LD := $(CC)
    OUTPUT_BINARY := !RunImageELF
    FINAL_BINARY := !RunImage
    CCFLAGS := -std=gnu99 -mlibscl -mhard-float -mthrowback -Wall -O2 \
               -fno-strict-aliasing -mpoke-function-name -D__riscos__
    LDFLAGS := -L$(GCCSDK_ROOT)/env/lib -lOSLibH32 -lSFLib32
else
    # Native POSIX build
    CC := gcc
    LD := gcc
    OUTPUT_BINARY := client
    FINAL_BINARY := $(OUTPUT_BINARY)
    CCFLAGS := -std=gnu99 -Wall -O2
    LDFLAGS :=
endif

# Directories
SRCDIR := c
HDRDIR := h
OBJDIR := o
PLATFORMDIR := $(if $(filter riscos,$(TARGET_OS)),riscos,posix)

# Source files
# SRCS := $(wildcard $(SRCDIR)/*.c)
# OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS)) \
#         $(OBJDIR)/socket_impl.o

# Explicit list of main source files (excluding platform-specific ones)
MAIN_SRCS := $(filter-out $(SRCDIR)/socket_utils_wrapper.c, $(wildcard $(SRCDIR)/*.c))

# Platform-specific implementation
PLATFORM_IMPL := $(PLATFORMDIR)/socket_utils_$(if $(filter riscos,$(TARGET_OS)),riscos,posix).c

# Object files
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(MAIN_SRCS)) \
        $(OBJDIR)/socket_utils_wrapper.o \
        $(OBJDIR)/socket_utils_$(if $(filter riscos,$(TARGET_OS)),riscos,posix).o


# Include paths
INCLUDES := -I. -I$(HDRDIR) -I$(PLATFORMDIR)

# Ensure directories exist
$(shell mkdir -p $(OBJDIR))

.PHONY: all clean riscos native

all: $(TARGET_OS)

riscos:
	@$(MAKE) TARGET_OS=riscos all_target

native:
	@$(MAKE) TARGET_OS=native all_target

all_target: $(FINAL_BINARY)

# NEW RULE: Link object files into OUTPUT_BINARY
$(OUTPUT_BINARY): $(OBJS)
	@echo "Linking $(OUTPUT_BINARY) from:" $(OBJS)
	$(LD) $(CCFLAGS) $(LDFLAGS) -o $@ $(OBJS)

# Existing conversion rule
$(FINAL_BINARY): $(OUTPUT_BINARY)
ifeq ($(TARGET_OS),riscos)
	$(GCCSDK_CROSS)/elf2aif $(OUTPUT_BINARY) $(FINAL_BINARY) 2> aif_error.log || \
	(if grep -q "'$(OUTPUT_BINARY)' is not an ELF file" aif_error.log; then \
		mv -f $(OUTPUT_BINARY) $(FINAL_BINARY); \
	else cat aif_error.log; false; fi)
else
	@echo "Native build complete"
endif

# Compilation rules
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(Q)$(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@
	$(ECHO) "Compiled: $< -> $@"

$(OBJDIR)/socket_utils_wrapper.o: c/socket_utils_wrapper.c
	$(Q)$(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@
	$(ECHO) "Compiled: $< -> $@"

$(OBJDIR)/socket_utils_riscos.o: riscos/socket_utils_riscos.c
	$(Q)$(CC) -c $(CCFLAGS) $(INCLUDES) -D__riscos__ $< -o $@
	$(ECHO) "Compiled: $< -> $@"

$(OUTPUT_BINARY): $(OBJS)
	$(ECHO) "Linking: $(OUTPUT_BINARY)"
	$(Q)$(LD) $(CCFLAGS) $(LDFLAGS) -o $@ $(OBJS)

$(FINAL_BINARY): $(OUTPUT_BINARY)
ifeq ($(TARGET_OS),riscos)
	$(ECHO) "Converting to AIF..."
	$(Q)$(GCCSDK_CROSS)/elf2aif $(OUTPUT_BINARY) $(FINAL_BINARY) 2> aif_error.log || \
	(if grep -q "'$(OUTPUT_BINARY)' is not an ELF file" aif_error.log; then \
		mv -f $(OUTPUT_BINARY) $(FINAL_BINARY); \
	else cat aif_error.log; false; fi)
else
	$(ECHO) "Build complete: $(FINAL_BINARY)"
endif

clean:
	rm -rf $(OBJDIR)/*.o $(OUTPUT_BINARY) $(FINAL_BINARY) aif_error.log