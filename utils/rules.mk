# ----- Tools -----------------------------------------------------------------

GCC     = $(TOOLCHAIN_PREFIX)gcc
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
SIZE    = $(TOOLCHAIN_PREFIX)size

ECHO    = echo $(subst $(dir $(PWD)),/[...]/,$@)
MKDIR   = mkdir -p $(dir $@)
RM      = rm -rf

# ----- Directories and files -------------------------------------------------

ifdef PROJECT_NAME
OUTPUT_NAME = $(PROJECT_NAME)
else
OUTPUT_NAME = $(notdir $(PWD))
endif

OUTPUT_DIR  = _out

__ELF = $(OUTPUT_DIR)/$(OUTPUT_NAME).elf

BASE_FILES       = $(addprefix $(OUTPUT_DIR),$(basename $(realpath $(SOURCE_FILES))))
DEPENDENCY_FILES = $(addsuffix .d,$(BASE_FILES))
OBJECT_FILES     = $(addsuffix .o,$(BASE_FILES))

VPATH += /

# ----- Flags -----------------------------------------------------------------

__CFLAGS += -fdata-sections
__CFLAGS += -ffunction-sections
__CFLAGS += -fno-exceptions
__CFLAGS += -fno-unwind-tables
__CFLAGS += -fno-asynchronous-unwind-tables
__CFLAGS += -fno-common
__CFLAGS += -nostdlib

# Warnings
__CFLAGS += -Wall
# __CFLAGS += -Wextra

CFLAGS += $(__CFLAGS)

CXXFLAGS += $(__CFLAGS)
CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics

CPPFLAGS += -MD
CPPFLAGS += -MP
CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(OUTPUT_NAME).map
LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))

LDLIBS += $(addprefix -l,$(LIBRARIES))

# ----- Rules -----------------------------------------------------------------

.PHONY: all clean

all: $(__ELF)
	@echo # New line for better reading
	$(SIZE) $<
	@echo # Another new line for even better reading

clean:
	$(RM) $(OUTPUT_DIR)

# Output

%.elf: $(OBJECT_FILES)
	$(ECHO) && $(MKDIR) && $(GCC) $(GCCFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.bin: $(__ELF)
	$(ECHO) && $(MKDIR) && $(OBJCOPY) -O binary $< $@

%.hex: $(__ELF)
	$(ECHO) && $(MKDIR) && $(OBJCOPY) -O ihex $< $@

# Assembler

$(OUTPUT_DIR)/%.o: %.s $(MAKEFILE_LIST)
	$(ECHO) && $(MKDIR) && $(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: %.S $(MAKEFILE_LIST)
	$(ECHO) && $(MKDIR) && $(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<

# C

$(OUTPUT_DIR)/%.o: %.c $(MAKEFILE_LIST)
	$(ECHO) && $(MKDIR) && $(GCC) $(GCCFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# C++

$(OUTPUT_DIR)/%.o: %.cpp $(MAKEFILE_LIST)
	$(ECHO) && $(MKDIR) && $(GCC) $(GCCFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

-include $(DEPENDENCY_FILES)
