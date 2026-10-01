# mmap_probe - Makefile para ps5-payload-sdk
#
# Uso:
#   export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
#   make                             # gera mmap_probe.elf
#   make test                        # envia pro elfldr na porta 9021

PS5_HOST ?= <PS5_IP>
PS5_PORT ?= 9021

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK nao definido. Ex: export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk)
endif

ELF    := mmap_probe.elf
CFLAGS := -Wall -Werror -O2 -g

all: $(ELF)

# prospero-clang compila e linka (crt1 + libc + libkernel) num passo so.
$(ELF): main.c
	$(CC) $(CFLAGS) -o $@ $^

test: $(ELF)
	$(PS5_DEPLOY) -h $(PS5_HOST) -p $(PS5_PORT) $^

clean:
	rm -f $(ELF)

.PHONY: all test clean
