# ===== Project =====
PROJECT := tim_trg_adc_f401
OBJDIR  := build

# ===== Toolchain =====
CC      := arm-none-eabi-gcc
AS      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# ===== MCU / CPU =====
CPU      := cortex-m4
FPU      := fpv4-sp-d16
FLOATABI := softfp
DEFS     := -DSTM32F401xE

# ===== Paths (relative to this DayXX folder) =====
COMMON_DIR  := ../common

INCLUDES := \
  -I$(COMMON_DIR)/Drivers/CMSIS/Core/Include \
  -I$(COMMON_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include

LDSCRIPT := $(COMMON_DIR)/linker/stm32f401.ld

# ===== Source search paths =====
# VPATH tells make where to look for source files
VPATH := src:$(COMMON_DIR):$(COMMON_DIR)/startup

# ===== Sources (just file names; VPATH handles the dirs) =====
SRCS_C := \
  main.c \
  system_stm32f4xx.c \
  stubs.c

SRCS_S := \
  startup_stm32f401xx.s

# ===== Flags =====
COMMON_FLAGS := -mcpu=$(CPU) -mthumb -ffunction-sections -fdata-sections \
                -Wall -Wextra $(DEFS) $(INCLUDES)

CFLAGS  := $(COMMON_FLAGS) -O2 -mfpu=$(FPU) -mfloat-abi=$(FLOATABI) \
           -std=c11 -fno-builtin -ffreestanding

ASFLAGS := -mcpu=$(CPU) -mthumb

# Bare-metal link (no libc)
LDFLAGS := -T $(LDSCRIPT) -Wl,--gc-sections -nostartfiles -nostdlib \
           -Wl,-Map=$(OBJDIR)/$(PROJECT).map
LDLIBS  := -lgcc

# ===== Objects (all go to build/) =====
OBJS_C := $(addprefix $(OBJDIR)/,$(SRCS_C:.c=.o))
OBJS_S := $(addprefix $(OBJDIR)/,$(SRCS_S:.s=.o))
OBJS   := $(OBJS_C) $(OBJS_S)

# ===== Outputs =====
ELF := $(OBJDIR)/$(PROJECT).elf
BIN := $(OBJDIR)/$(PROJECT).bin

# ===== Rules =====
.PHONY: all clean flash size erase reset

all: $(ELF) $(BIN) size

$(OBJDIR):
	mkdir -p $(OBJDIR)

# C -> .o (source found via VPATH)
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ASM -> .o (source found via VPATH)
$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(AS) $(ASFLAGS) -c $< -o $@

# Link
$(ELF): $(OBJS)
	$(CC) $(COMMON_FLAGS) -mfpu=$(FPU) -mfloat-abi=$(FLOATABI) \
	    $(OBJS) $(LDFLAGS) -o $@ $(LDLIBS)

# Binary
$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

# Size
size: $(ELF)
	$(SIZE) $<

# Flash with ST-LINK
flash: $(BIN)
	st-flash write $(BIN) 0x08000000

erase:
	st-flash erase

reset:
	st-flash reset

clean:
	rm -rf $(OBJDIR)

