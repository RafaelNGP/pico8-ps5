# mmap_probe - Makefile para ps5-payload-sdk
#
# Uso:
#   export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
#   make                 # gera mmap_probe.elf
#   make test PS5_HOST=<ip-do-ps5>   # envia pro elfldr na porta 9021

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK nao definido. Ex: export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk)
endif

ELF     := mmap_probe.elf
CFLAGS  := -Wall -O2 -g
SRCS    := main.c
OBJS    := $(SRCS:.c=.o)

PS5_HOST ?= ps5
PS5_PORT ?= 9021

all: $(ELF)

$(ELF): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Envia o ELF direto pro elf loader (porta 9021 do Relapse/etaHEN).
test: $(ELF)
	nc -q0 $(PS5_HOST) $(PS5_PORT) < $(ELF)

clean:
	rm -f $(OBJS) $(ELF)

.PHONY: all test clean
