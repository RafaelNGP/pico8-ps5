# pico8_loader - Makefile para ps5-payload-sdk
#
# Uso:
#   export PS5_PAYLOAD_SDK=/opt/ps5-payload-sdk
#   make upload-data  # envia pico8_dyn e pico8.dat para PICO8/pico8/ (uma vez)
#   make app          # gera o app nativo (eboot.bin) em ../pico8_app/build
#   make install-app  # envia o app para /data/homebrew/PICO8; o ShadowMountPlus
#                     # registra o icone PICO-8 na home em ate ~15 s
#   make log          # baixa /data/pico8/loader.log
#   make release      # zip para instalar sem compilar (../release/)
#
# Desenvolvimento (loader como payload, sem o app):
#   make              # gera pico8_loader.elf
#   make websrv       # baixa e inicia o websrv (necessario para `make run`)
#   make run          # roda em primeiro plano via hbldr do websrv (com video)

PS5_HOST  ?= <PS5_IP>
FTP_PORT  ?= 2121
PLDMGR    ?= 8084
WEBSRV    ?= 8080
HB_DIR    := /data/homebrew/PICO8
DEV_ELF   := /data/pico8/pico8_loader.elf
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
SRCS   := main.c shims_libc.c shims_sdl.c net_curl.c p8_alloc.c paths.c
CFLAGS := -Wall -Werror -O2 -g -I../deps $(SDL2_CFLAGS) -I$(CURL)/include -DCURL_STATICLIB

all: $(ELF)

$(SDL2)/lib/libSDL2.a:
	../deps/build_sdl2.sh

$(CURL)/lib/libcurl.a:
	../deps/build_curl.sh

$(ELF): $(SRCS) loader.h ../deps/dlmalloc/malloc.c $(SDL2)/lib/libSDL2.a $(CURL)/lib/libcurl.a
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(SDL2_LIBS) $(CURL_LIBS) -lpthread

# Certificados raiz para o HTTPS do Splore (o build da libcurl tambem baixa).
../deps/cacert.pem:
	curl -fsSLo $@ https://curl.se/ca/cacert.pem

# Os arquivos do PICO-8 do usuario ficam dentro da pasta do app
# (pico8_loader/paths.c), como os de firmware no ProsperoEden.
upload-data:
	curl -sS --ftp-create-dirs -T $(PICO8_DIR)/pico8_dyn ftp://$(PS5_HOST):$(FTP_PORT)$(HB_DIR)/pico8/
	curl -sS --ftp-create-dirs -T $(PICO8_DIR)/pico8.dat ftp://$(PS5_HOST):$(FTP_PORT)$(HB_DIR)/pico8/

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
	curl -sS --ftp-create-dirs -T $(ELF) ftp://$(PS5_HOST):$(FTP_PORT)$(DEV_ELF)
	curl -sS -N -m 30 "http://$(PS5_HOST):$(WEBSRV)/hbldr?path=$(DEV_ELF)&cwd=/data/pico8&pipe=1" || true

# App nativo: o loader inteiro como eboot.bin do PPSA99808 (../pico8_app).
APP     := ../pico8_app
APP_DIR := $(APP)/build/PPSA99808
BP      := ../deps/boilerplate
FTP     := ftp://$(PS5_HOST):$(FTP_PORT)

# A tabela de imports do app (app_imports.c) vem do proprio eboot linkado:
# gera depois do build e, se mudou, builda de novo.
app: $(SDL2)/lib/libSDL2.a $(CURL)/lib/libcurl.a
	../deps/build_app.sh pico8_app
	cp $(APP)/src/app_imports.c $(APP)/build/app_imports.prev
	python3 ../deps/gen_app_imports.py $(BP)/build/llvm-pie.elf \
	    $(BP)/.deps/native/ps5-payload-sdk/target/lib $(APP)/src/app_imports.c
	cmp -s $(APP)/src/app_imports.c $(APP)/build/app_imports.prev || \
	    ../deps/build_app.sh pico8_app

APP_FILES := eboot.bin sandbox-elevator.elf sce_module/libc.prx \
             sce_sys/param.json sce_sys/icon0.png

install-app: app ../deps/cacert.pem
	-@curl -sS -Q "DELE /data/pico8/loader.log" $(FTP)/ >/dev/null 2>&1
	for f in $(APP_FILES); do \
	    curl -sS --ftp-create-dirs -T $(APP_DIR)/$$f $(FTP)$(HB_DIR)/$$f || exit 1; \
	done
	curl -sS -T ../deps/cacert.pem $(FTP)$(HB_DIR)/cacert.pem

# Pacote para quem nao vai compilar. A pasta PICO8/ vai inteira para o
# PS5: o app (icone generico, sem nada da Lexaloffle), o cacert.pem e a
# subpasta pico8/ onde o usuario poe o pico8_dyn e o pico8.dat.
VERSION    := $(shell git describe --tags --always --dirty)
COMMIT     := $(shell git rev-parse --short HEAD)
SOURCE_URL ?= $(shell git remote get-url origin 2>/dev/null || echo "(ainda sem repositorio publico)")
REL_NAME   := pico8-ps5-$(VERSION)
REL        := ../release/$(REL_NAME)

release: ../deps/cacert.pem
	APP_ICON_LOGO= $(MAKE) app
	rm -rf $(REL) $(REL).zip
	mkdir -p $(REL)/PICO8/pico8 $(REL)/licencas
	cp -r $(APP_DIR)/. $(REL)/PICO8/
	rm -rf $(REL)/PICO8/assets
	cp ../deps/cacert.pem $(REL)/PICO8/
	cp $(APP)/release/COLOQUE-AQUI.txt $(REL)/PICO8/pico8/
	sed -e 's|@VERSION@|$(VERSION)|' -e 's|@COMMIT@|$(COMMIT)|' \
	    -e 's|@SOURCE@|$(SOURCE_URL)|' $(APP)/release/LEIA-ME.txt > $(REL)/LEIA-ME.txt
	cp ../deps/boilerplate/LICENSE                  $(REL)/licencas/boilerplate-GPL-3.0.txt
	cp ../deps/boilerplate/THIRD_PARTY_NOTICES.md   $(REL)/licencas/boilerplate-THIRD_PARTY_NOTICES.md
	cp ../deps/SDL/LICENSE.txt                      $(REL)/licencas/SDL2-zlib.txt
	cp ../deps/curl-*/COPYING                       $(REL)/licencas/curl.txt
	cp ../deps/mbedtls-*/LICENSE                    $(REL)/licencas/mbedtls-Apache-2.0.txt
	sed -n '2,/opensource.org\/license\/mit-0/p' ../deps/dlmalloc/malloc.c \
	    > $(REL)/licencas/dlmalloc-MIT-0.txt
	printf 'cacert.pem: certificados raiz da Mozilla (MPL-2.0), via https://curl.se/ca/cacert.pem\n' \
	    > $(REL)/licencas/cacert.txt
	cd ../release && zip -qr $(REL_NAME).zip $(REL_NAME)
	@echo "pacote: release/$(REL_NAME).zip"

log:
	@curl -sS ftp://$(PS5_HOST):$(FTP_PORT)/data/pico8/loader.log

clean:
	rm -f $(ELF)

.PHONY: all upload-data deploy websrv run app install-app release log clean
