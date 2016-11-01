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

ifndef OUTPUT_NAME
OUTPUT_NAME = $(PROJECT_NAME)
endif

BINARY     = $(OUTPUT_NAME).bin
EXECUTABLE = $(OUTPUT_NAME).elf
MAPFILE    = $(OUTPUT_NAME).map

OUTPUT_DIR = _out

vpath %.elf $(OUTPUT_DIR)
vpath %.bin $(OUTPUT_DIR)

# ----- Flags -----------------------------------------------------------------

GCCFLAGS += -fdiagnostics-color=always

# ASMFLAGS +=

COMMON_CFLAGS += -fdata-sections
COMMON_CFLAGS += -ffunction-sections
COMMON_CFLAGS += -fno-builtin
COMMON_CFLAGS += -fno-exceptions
COMMON_CFLAGS += -fno-unwind-tables
COMMON_CFLAGS += -nostdlib

CXXFLAGS += -fno-rtti
CXXFLAGS += -fno-threadsafe-statics

CPPFLAGS += -MD
CPPFLAGS += -MP
CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))

LDFLAGS += --specs=nano.specs
LDFLAGS += --specs=nosys.specs
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(MAPFILE)
LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))

_LIBFLAGS = $(addprefix -l,$(LIBRARIES))

# ----- Sources and objects ---------------------------------------------------

_SOURCE_FILES = $(sort $(realpath $(filter %.c %.cpp %.s,$(SOURCE_FILES))))
_OUTPUT_FILES = $(addprefix $(OUTPUT_DIR),$(basename $(_SOURCE_FILES)))

_DEPENDENCY_FILES = $(addsuffix .d,$(_OUTPUT_FILES))
_OBJECT_FILES     = $(addsuffix .o,$(_OUTPUT_FILES))

# ----- Rules -----------------------------------------------------------------

.PHONY: all clean

all: $(EXECUTABLE) $(BINARY)
	@echo # New line for better reading
	$(SIZE) $<
	@echo # Another new line for even better reading

clean:
	$(RM) $(OUTPUT_DIR)

$(OUTPUT_DIR)/$(EXECUTABLE): $(_OBJECT_FILES)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(LDFLAGS) $^ $(_LIBFLAGS) -o $@

$(OUTPUT_DIR)/$(BINARY): $(EXECUTABLE)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O binary $< $@

$(OUTPUT_DIR)/%.o: /%.s $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(ASMFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "$<"

$(OUTPUT_DIR)/%.o: /%.c $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(COMMON_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "$<"

$(OUTPUT_DIR)/%.o: /%.cpp $(MAKEFILE_LIST)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(COMMON_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "$<"

-include $(_DEPENDENCY_FILES)
