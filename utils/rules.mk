# ----- Tools -----------------------------------------------------------------

GCC     = $(TOOLCHAIN_PREFIX)gcc
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
SIZE    = $(TOOLCHAIN_PREFIX)size

MKDIR   = mkdir -p
RM      = rm -rf

# ----- Directories and files -------------------------------------------------

ifndef PROJECT_NAME
PROJECT_NAME = PROJECT_NAME_NOT_SET
endif

OUTPUT_DIR = _out

BINARY     = $(OUTPUT_DIR)/$(PROJECT_NAME).bin
EXECUTABLE = $(OUTPUT_DIR)/$(PROJECT_NAME).elf
HEXARY     = $(OUTPUT_DIR)/$(PROJECT_NAME).hex

# ----- Flags -----------------------------------------------------------------

# ASMFLAGS +=

COMMON_CFLAGS += -fdata-sections
COMMON_CFLAGS += -ffunction-sections
COMMON_CFLAGS += -fno-exceptions
COMMON_CFLAGS += -fno-unwind-tables
COMMON_CFLAGS += -fno-asynchronous-unwind-tables
COMMON_CFLAGS += -fno-common
COMMON_CFLAGS += -nostdlib
# Warnings
COMMON_CFLAGS += -Wall
# COMMON_CFLAGS += -Wextra

CFLAGS += $(COMMON_CFLAGS)

CXXFLAGS += $(COMMON_CFLAGS)
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics

CPPFLAGS += -MD
CPPFLAGS += -MP
CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(PROJECT_NAME).map
LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))
LDFLAGS += -Wl,--start-group $(addprefix -l,$(LIBRARIES)) -Wl,--end-group

# ----- Sources and objects ---------------------------------------------------

_SOURCE_FILES = $(realpath $(SOURCE_FILES))
_OUTPUT_FILES = $(addprefix $(OUTPUT_DIR),$(basename $(_SOURCE_FILES)))

_DEPENDENCY_FILES = $(addsuffix .d,$(_OUTPUT_FILES))
_OBJECT_FILES     = $(addsuffix .o,$(_OUTPUT_FILES))

# ----- Rules -----------------------------------------------------------------

.PHONY: all clean

all: $(EXECUTABLE) $(BINARY) $(HEXARY)
	@echo # New line for better reading
	$(SIZE) $<
	@echo # Another new line for even better reading

clean:
	$(RM) $(OUTPUT_DIR)

# Output

$(EXECUTABLE): $(_OBJECT_FILES)
	echo $@
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(LDFLAGS) -o $@ $+

$(BINARY): $(EXECUTABLE)
	echo $@
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O binary $< $@

$(HEXARY): $(EXECUTABLE)
	echo $@
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O ihex $< $@

# Assembler

$(OUTPUT_DIR)/%.o: /%.s $(MAKEFILE_LIST)
	echo $(subst $(dir $(PWD)),/.../,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: /%.S $(MAKEFILE_LIST)
	echo $(subst $(dir $(PWD)),/.../,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

# C

$(OUTPUT_DIR)/%.o: /%.c $(MAKEFILE_LIST)
	echo $(subst $(dir $(PWD)),/.../,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# C++

$(OUTPUT_DIR)/%.o: /%.cpp $(MAKEFILE_LIST)
	echo $(subst $(dir $(PWD)),/.../,$@)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

-include $(_DEPENDENCY_FILES)
