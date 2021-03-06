# ----- Tools ------------------------------------------------------------------

GCC     = $(TOOLCHAIN_PREFIX)gcc
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
SIZE    = $(TOOLCHAIN_PREFIX)size

MKDIR   = mkdir -p
RM      = rm -rf

# ----- Directories and files --------------------------------------------------

ifndef PROJECT_NAME
PROJECT_NAME = $(notdir $(PWD))
endif

__OUTPUT_DIR = _out

__BASE_FILES       = $(addprefix $(__OUTPUT_DIR),$(basename $(realpath $(SOURCE_FILES))))
__DEPENDENCY_FILES = $(addsuffix .d,$(__BASE_FILES))
__OBJECT_FILES     = $(addsuffix .o,$(__BASE_FILES))

__BIN = $(__OUTPUT_DIR)/$(PROJECT_NAME).bin
__ELF = $(__OUTPUT_DIR)/$(PROJECT_NAME).elf
__HEX = $(__OUTPUT_DIR)/$(PROJECT_NAME).hex
__MAP = $(__OUTPUT_DIR)/$(PROJECT_NAME).map

VPATH += /

# ----- Flags ------------------------------------------------------------------

__COMMON_CFLAGS += -fdata-sections
__COMMON_CFLAGS += -ffunction-sections
__COMMON_CFLAGS += -fno-asynchronous-unwind-tables
__COMMON_CFLAGS += -fno-common
__COMMON_CFLAGS += -fno-exceptions
__COMMON_CFLAGS += -fno-unwind-tables
__COMMON_CFLAGS += -nostdlib
__COMMON_CFLAGS += -Wall
__COMMON_CFLAGS += -Wextra

CFLAGS +=

CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics

CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))
CPPFLAGS += -MD
CPPFLAGS += -MP

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(__MAP)
LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))

LDLIBS += $(addprefix -l,$(LIBRARIES))

# ----- Rules ------------------------------------------------------------------

.PHONY: all clean

all: $(__ELF) $(__BIN) $(__HEX)
	@echo # New line for better reading
	$(SIZE) $<
	@echo # Another new line for even better reading

clean:
	$(RM) $(__OUTPUT_DIR)

# Output

%.elf: $(__OBJECT_FILES)
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.bin: %.elf
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O ihex $< $@

# Assembler

$(__OUTPUT_DIR)/%.o: %.s
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

$(__OUTPUT_DIR)/%.o: %.S
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

# C

$(__OUTPUT_DIR)/%.o: %.c
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(__COMMON_CFLAGS) $(CPPFLAGS) -c -o $@ $<

# C++

$(__OUTPUT_DIR)/%.o: %.cpp
	@echo $(subst $(dir $(PWD)),/[...]/,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(CXXFLAGS) $(__COMMON_CFLAGS) $(CPPFLAGS) -c -o $@ $<

# ----- Dependencies -----------------------------------------------------------

-include $(__DEPENDENCY_FILES)
