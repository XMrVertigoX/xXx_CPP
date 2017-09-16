# ----- Tools -----------------------------------------------------------------

GCC     = $(TOOLCHAIN_PREFIX)gcc
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
SIZE    = $(TOOLCHAIN_PREFIX)size

MKDIR   = mkdir -p
RM      = rm -rf

__ECHO  = echo $(subst $(dir $(PWD)),/[...]/,$@)

# ----- Directories and files -------------------------------------------------

OUTPUT_NAME = $(notdir $(PWD))
OUTPUT_DIR  = _out

ELF_FILE = $(OUTPUT_DIR)/$(OUTPUT_NAME).elf

BASE_FILES       = $(addprefix $(OUTPUT_DIR),$(basename $(realpath $(SOURCE_FILES))))
DEPENDENCY_FILES = $(addsuffix .d,$(BASE_FILES))
OBJECT_FILES     = $(addsuffix .o,$(BASE_FILES))

VPATH += /

# ----- Flags -----------------------------------------------------------------

COMMON_CFLAGS += -fdata-sections
COMMON_CFLAGS += -ffunction-sections
COMMON_CFLAGS += -fno-exceptions
COMMON_CFLAGS += -fno-unwind-tables
COMMON_CFLAGS += -fno-asynchronous-unwind-tables
COMMON_CFLAGS += -fno-common
COMMON_CFLAGS += -nostdlib

# Warnings
COMMON_CFLAGS += -Wall
COMMON_CFLAGS += -Wextra

CFLAGS += $(COMMON_CFLAGS)

CXXFLAGS += $(COMMON_CFLAGS)
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics

CPPFLAGS += -MD
CPPFLAGS += -MP
CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(OUTPUT_NAME).map
LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))
LDFLAGS += -Wl,--start-group $(addprefix -l,$(LIBRARIES)) -Wl,--end-group

# ----- Rules -----------------------------------------------------------------

.PHONY: all clean

all: $(ELF_FILE)
	@echo # New line for better reading
	$(SIZE) $<
	@echo # Another new line for even better reading

clean:
	$(RM) $(OUTPUT_DIR)

# Output

%.elf: $(OBJECT_FILES)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(LDFLAGS) -o $@ $^

%.bin: $(ELF_FILE)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O binary $< $@

%.hex: $(ELF_FILE)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O ihex $< $@

# Directories

$(OUTPUT_DIR):
	$(MKDIR) $@

# Assembler

$(OUTPUT_DIR)/%.o: %.s $(MAKEFILE_LIST)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: %.S $(MAKEFILE_LIST)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

# C

$(OUTPUT_DIR)/%.o: %.c $(MAKEFILE_LIST)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# C++

$(OUTPUT_DIR)/%.o: %.cpp $(MAKEFILE_LIST)
	$(__ECHO)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

-include $(DEPENDENCY_FILES)
