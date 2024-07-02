# Makefile to build doom64

TARGET_STRING := doom64.elf
TARGET := $(TARGET_STRING)

# Preprocessor definitions
DEFINES := _FINALROM=1 NDEBUG=1 F3DEX_GBI_2=1

SRC_DIRS :=

# Whether to hide commands or not
VERBOSE ?= 1
ifeq ($(VERBOSE),0)
  V := @
endif

# Whether to colorize build messages
COLOR ?= 1

#==============================================================================#
# Target Executable and Sources                                                #
#==============================================================================#
# BUILD_DIR is the location where all build artifacts are placed
BUILD_DIR := build

# Directories containing source files
SRC_DIRS += .

C_FILES           := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))

# Object files
O_FILES := $(foreach file,$(C_FILES),./$(file:.c=.o))

CFLAGS=${KOS_CFLAGS} -std=c11

# tools
PRINT = printf

ifeq ($(COLOR),1)
NO_COL  := \033[0m
RED     := \033[0;31m
GREEN   := \033[0;32m
BLUE    := \033[0;34m
YELLOW  := \033[0;33m
BLINK   := \033[33;5m
endif

# Common build print status function
define print
  @$(PRINT) "$(GREEN)$(1) $(YELLOW)$(2)$(GREEN) -> $(BLUE)$(3)$(NO_COL)\n"
endef

#KOS_CFLAGS += -O3

#==============================================================================#
# Main Targets                                                                 #
#==============================================================================#

all: $(TARGET)

$(TARGET): $(O_FILES)
	${KOS_CC} ${KOS_CFLAGS} ${KOS_LDFLAGS} -o $@ ${KOS_START} $(O_FILES) -lkosfat -loggvorbisplay -lvorbis -logg ${KOS_LIBS} -lm

clean:
	$(RM) $(O_FILES)

ALL_DIRS := $(BUILD_DIR) $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))


print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true

include Makefile.kos
