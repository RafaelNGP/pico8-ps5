# pico8_loader - Makefile para ps5-payload-sdk
#
# Uso:
#   export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
#   make              # gera pico8_loader.elf
#   make upload-data  # envia pico8_dyn + pico8.dat por FTP (uma vez)
#   make deploy       # envia o loader ao Payload Manager (segundo plano, sem video)
#   make websrv       # baixa e inicia o websrv (necessario para `make run`)
#   make run          # roda como app em primeiro plano via websrv (com video)
#   make log          # baixa /data/pico8/loader.log

PS5_HOST  ?= <PS5_IP>
FTP_PORT  ?= 2121
PLDMGR    ?= 8084
WEBSRV    ?= 8080
HB_DIR    := /data/homebrew/PICO8
PICO8_DIR ?= $(HOME)/pico-8

ifdef PS5_PAYLOAD_SDK
    include $(PS5_PAYLOAD_SDK)/toolchain/prospero.mk
else
    $(error PS5_PAYLOAD_SDK nao definido. Ex: export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk)
endif

# SDL2 do PS5, compilado por ../deps/build_sdl2.sh
SDL2   := $(abspath ../deps/sdl2)
SDL2_CFLAGS := -I$(SDL2)/include/SDL2 -D_REENTRANT
SDL2_LIBS   := $(SDL2)/lib/libSDL2.a -lSceSystemService -lSceUserService \
               -lScePad -lSceAudioOut -lSceVideoOut -lSceKeyboard -lSceImeDialog

# libcurl + mbedTLS do PS5, compilados por ../deps/build_curl.sh
CURL   := $(abspath ../deps/curl)
CURL_LIBS := $(CURL)/lib/libcurl.a $(CURL)/lib/libmbedtls.a \
             $(CURL)/lib/libmbedx509.a $(CURL)/lib/libmbedcrypto.a \
             $(CURL)/lib/libeverest.a $(CURL)/lib/libp256m.a

ELF    := pico8_loader.elf
SRCS   := main.c shims_libc.c shims_sdl.c net_curl.c
CFLAGS := -Wall -Werror -O2 -g $(SDL2_CFLAGS) -I$(CURL)/include -DCURL_STATICLIB

all: $(ELF)

$(SDL2)/lib/libSDL2.a:
	../deps/build_sdl2.sh

$(CURL)/lib/libcurl.a:
	../deps/build_curl.sh

$(ELF): $(SRCS) loader.h $(SDL2)/lib/libSDL2.a $(CURL)/lib/libcurl.a
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(SDL2_LIBS) $(CURL_LIBS) -lpthread

upload-data:
	curl -sS --ftp-create-dirs -T $(PICO8_DIR)/pico8_dyn ftp://$(PS5_HOST):$(FTP_PORT)/data/pico8/
	curl -sS --ftp-create-dirs -T $(PICO8_DIR)/pico8.dat ftp://$(PS5_HOST):$(FTP_PORT)/data/pico8/
	curl -sS --ftp-create-dirs -T ../deps/cacert.pem ftp://$(PS5_HOST):$(FTP_PORT)/data/pico8/

deploy: $(ELF)
	-@curl -sS -Q "DELE /data/pico8/loader.log" ftp://$(PS5_HOST):$(FTP_PORT)/ >/dev/null 2>&1
	./deploy.sh $(PS5_HOST) $(PLDMGR) $(ELF)

WEBSRV_VER := v0.34
WEBSRV_ELF := ../tools/websrv-ps5.elf

$(WEBSRV_ELF):
	mkdir -p ../tools
	curl -fsSL -o $@ https://github.com/ps5-payload-dev/websrv/releases/download/$(WEBSRV_VER)/websrv-ps5.elf

websrv: $(WEBSRV_ELF)
	./deploy.sh $(PS5_HOST) $(PLDMGR) $(WEBSRV_ELF)

# O video do PS5 so aparece para o app em primeiro plano. O hbldr do
# websrv (ps5-payload-dev/websrv) abre um app fake e roda o ELF nele.
run: $(ELF)
	-@curl -sS -Q "DELE /data/pico8/loader.log" ftp://$(PS5_HOST):$(FTP_PORT)/ >/dev/null 2>&1
	curl -sS --ftp-create-dirs -T $(ELF) ftp://$(PS5_HOST):$(FTP_PORT)$(HB_DIR)/eboot.elf
	curl -sS -N -m 30 "http://$(PS5_HOST):$(WEBSRV)/hbldr?path=$(HB_DIR)/eboot.elf&cwd=/data/pico8&pipe=1" || true

log:
	@curl -sS ftp://$(PS5_HOST):$(FTP_PORT)/data/pico8/loader.log

clean:
	rm -f $(ELF)

.PHONY: all upload-data deploy websrv run log clean
