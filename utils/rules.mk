# ----- Tools -----------------------------------------------------------------

GCC     = $(TOOLCHAIN_PREFIX)gcc
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
SIZE    = $(TOOLCHAIN_PREFIX)size

MKDIR   = mkdir -p
RM      = rm -rf

# ----- Directories and files -------------------------------------------------

ifndef PROJECT_NAME
PROJECT_NAME = PROJECT_NAME_not_set
endif

BINARY     = $(PROJECT_NAME).bin
EXECUTABLE = $(PROJECT_NAME).elf
HEXARY     = $(PROJECT_NAME).hex
MAPFILE    = $(PROJECT_NAME).map

ifndef OUTPUT_DIR
OUTPUT_DIR = _out
endif

vpath %.bin $(OUTPUT_DIR)
vpath %.elf $(OUTPUT_DIR)
vpath %.hex $(OUTPUT_DIR)

# ----- Flags -----------------------------------------------------------------

ASMFLAGS +=

COMMON_CFLAGS += -fdata-sections
COMMON_CFLAGS += -ffunction-sections
COMMON_CFLAGS += -fno-builtin
COMMON_CFLAGS += -fno-exceptions
COMMON_CFLAGS += -fno-strict-aliasing
COMMON_CFLAGS += -fno-unwind-tables
COMMON_CFLAGS += -nostdlib
COMMON_CFLAGS += -Wall
COMMON_CFLAGS += -Wextra

CFLAGS +=

CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics

CPPFLAGS += -MD
CPPFLAGS += -MP
CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))

ifeq ($(TOOLCHAIN_PREFIX), arm-none-eabi-)
LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
endif

LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(MAPFILE)
LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))

_LIBFLAGS = $(addprefix -l,$(LIBRARIES))

# ----- Sources and objects ---------------------------------------------------

_SOURCE_FILES = $(sort $(realpath $(filter %.c %.cpp %.s %.S,$(SOURCE_FILES))))
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

$(OUTPUT_DIR)/$(EXECUTABLE): $(_OBJECT_FILES)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(LDFLAGS) $^ $(_LIBFLAGS) -o $@

$(OUTPUT_DIR)/$(BINARY): $(EXECUTABLE)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O binary $< $@

$(OUTPUT_DIR)/$(HEXARY): $(EXECUTABLE)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O ihex $< $@

# Assembler

$(OUTPUT_DIR)/%.o: /%.s $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo $(shell realpath --relative-to=".." "$<")

$(OUTPUT_DIR)/%.o: /%.S $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo $(shell realpath --relative-to=".." "$<")

# C

$(OUTPUT_DIR)/%.o: /%.c $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(COMMON_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo $(shell realpath --relative-to=".." "$<")

# C++

$(OUTPUT_DIR)/%.o: /%.cpp $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(COMMON_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo $(shell realpath --relative-to=".." "$<")

-include $(_DEPENDENCY_FILES)
