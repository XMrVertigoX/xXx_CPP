# ----- Toolchain -------------------------------------------------------------

TOOLCHAIN_PREFIX = arm-none-eabi-

GCC     = $(TOOLCHAIN_PREFIX)gcc
GDB     = $(TOOLCHAIN_PREFIX)gdb
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy
SIZE    = $(TOOLCHAIN_PREFIX)size

# ----- Tools -----------------------------------------------------------------

MKDIR = mkdir -p
RM = rm -rf

# ----- Directories and files -------------------------------------------------

BINARY     = $(PROJECT_NAME).bin
EXECUTABLE = $(PROJECT_NAME).elf
MAPFILE    = $(PROJECT_NAME).map

PARENT_DIR = $(dir $(CURDIR))
OUTPUT_DIR = _out

VPATH += $(OUTPUT_DIR)

# ----- Flags -----------------------------------------------------------------

CPPFLAGS += $(addprefix -D,$(SYMBOLS))
CPPFLAGS += $(addprefix -I,$(realpath $(INCLUDE_DIRS)))
CPPFLAGS += -MD
CPPFLAGS += -MP

LDFLAGS += $(addprefix -L,$(realpath $(LIBRARY_DIRS)))
LDFLAGS += -Wl,-Map=$(OUTPUT_DIR)/$(MAPFILE)

_LIBFLAGS = $(addprefix -l,$(LIBRARIES))
# _LIBFLAGS = -Wl,--start-group $(addprefix -l, $(LIBRARIES)) -Wl,--end-group

# ----- Sourced and objects ---------------------------------------------------

_ASM_SOURCE_FILES  = $(sort $(realpath $(filter %.s %.S,$(SOURCE_FILES))))
_ASM_OBJECT_FILES += $(patsubst %.s,%.o,$(_ASM_SOURCE_FILES))
_ASM_OBJECT_FILES += $(patsubst %.S,%.o,$(_ASM_SOURCE_FILES))

_C_SOURCE_FILES    = $(sort $(realpath $(filter %.c,$(SOURCE_FILES))))
_C_OBJECT_FILES   += $(patsubst %.c,%.o,$(_C_SOURCE_FILES))

_CXX_SOURCE_FILES  = $(sort $(realpath $(filter %.cpp,$(SOURCE_FILES))))
_CXX_OBJECT_FILES += $(patsubst %.cpp,%.o,$(_CXX_SOURCE_FILES))

_OBJECT_FILES     += $(addprefix $(OUTPUT_DIR),$(_ASM_OBJECT_FILES))
_OBJECT_FILES     += $(addprefix $(OUTPUT_DIR),$(_C_OBJECT_FILES))
_OBJECT_FILES     += $(addprefix $(OUTPUT_DIR),$(_CXX_OBJECT_FILES))

_DEPENDENCY_FILES  = $(patsubst %.o,%.d,$(_OBJECT_FILES))

# ----- Rules -----------------------------------------------------------------

.PHONY: all clean download

all: $(EXECUTABLE) $(BINARY)
	@echo # New line for better reading
	$(SIZE) $<
	@echo # Another new line for even better reading

clean:
	$(RM) $(OUTPUT_DIR)

download: $(EXECUTABLE)
	$(GDB) -q -x download.gdb $<

$(OUTPUT_DIR)/$(EXECUTABLE): $(_OBJECT_FILES)
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(LDFLAGS) $^ $(_LIBFLAGS) -o $@

$(OUTPUT_DIR)/$(BINARY): $(EXECUTABLE)
	$(MKDIR) $(dir $@)
	$(OBJCOPY) -O binary $< $@

$(OUTPUT_DIR)/%.o: /%.c
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(COMMON_CFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "$(subst $(PARENT_DIR),,$<)"

$(OUTPUT_DIR)/%.o: /%.cpp
	$(MKDIR) $(dir $@)
	$(GCC) $(GCCFLAGS) $(COMMON_CFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	@echo "$(subst $(PARENT_DIR),,$<)"

-include $(_DEPENDENCY_FILES)