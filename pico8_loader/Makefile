# pico8_loader - build do app nativo (../pico8_app) e atalhos para o PS5
#
# Uso:
#   make              # gera o app nativo (eboot.bin) em ../pico8_app/build
#   make release      # zip para instalar sem compilar (../release/)
#   make clean
#
# Atalhos por FTP (precisam do IP: PS5_HOST=<ip>, no ambiente ou num
# local.mk fora do git com "PS5_HOST := <ip>"):
#   make install-app  # envia o app para /data/homebrew/<TITLE_ID>; o
#                     # ShadowMountPlus registra o icone em ate ~15 s
#   make upload-data  # envia pico8_dyn e pico8.dat para <TITLE_ID>/pico8/
#   make log          # mostra /data/pico8/loader.log
#
# O SDL2 e a libcurl usam o ps5-payload-sdk de PS5_PAYLOAD_SDK
# (padrao /opt/ps5-payload-sdk).

-include local.mk
FTP_PORT  ?= 2121
PICO8_DIR ?= $(HOME)/pico-8
# O Title ID do app eh o do param.json; a pasta no PS5 leva o mesmo nome.
TITLE_ID  := $(shell python3 -c 'import json; print(json.load(open("../pico8_app/sce_sys/param.json"))["titleId"])')
HB_DIR    := /data/homebrew/$(TITLE_ID)
FTP        = ftp://$(PS5_HOST):$(FTP_PORT)

APP     := ../pico8_app
APP_DIR := $(APP)/build/$(TITLE_ID)
BP      := ../deps/boilerplate

# SDL2 e libcurl + mbedTLS do PS5, linkados no eboot (pico8_app/build.env).
SDL2 := ../deps/sdl2
CURL := ../deps/curl

all: app

$(SDL2)/lib/libSDL2.a:
	../deps/build_sdl2.sh

$(CURL)/lib/libcurl.a:
	../deps/build_curl.sh

# Certificados raiz para o HTTPS do Splore (o build da libcurl tambem baixa).
../deps/cacert.pem:
	curl -fsSLo $@ https://curl.se/ca/cacert.pem

# A tabela de imports do app (app_imports.c) vem do proprio eboot linkado:
# gera depois do build e, se mudou, builda de novo.
app: $(SDL2)/lib/libSDL2.a $(CURL)/lib/libcurl.a
	../deps/build_app.sh pico8_app
	cp $(APP)/src/app_imports.c $(APP)/build/app_imports.prev
	python3 ../deps/gen_app_imports.py $(BP)/build/llvm-pie.elf \
	    $(BP)/.deps/native/ps5-payload-sdk/target/lib $(APP)/src/app_imports.c
	cmp -s $(APP)/src/app_imports.c $(APP)/build/app_imports.prev || \
	    ../deps/build_app.sh pico8_app

# Pacote para quem nao vai compilar. A pasta <TITLE_ID>/ vai inteira para
# /data/homebrew/: o app (icone generico, sem nada da Lexaloffle), o
# cacert.pem e a subpasta pico8/ onde o usuario poe o pico8_dyn e o pico8.dat.
VERSION    := $(shell git describe --tags --always --dirty)
COMMIT     := $(shell git rev-parse --short HEAD)
SOURCE_URL ?= $(or $(shell git remote get-url origin 2>/dev/null | sed 's/\.git$$//'),(no public repository yet))
REL_NAME   := pico8-ps5-$(VERSION)
REL        := ../release/$(REL_NAME)

release: ../deps/cacert.pem
	APP_ICON_LOGO= $(MAKE) app
	rm -rf $(REL) $(REL).zip
	mkdir -p $(REL)/$(TITLE_ID)/pico8 $(REL)/licenses
	cp -r $(APP_DIR)/. $(REL)/$(TITLE_ID)/
	rm -rf $(REL)/$(TITLE_ID)/assets
	cp ../deps/cacert.pem $(REL)/$(TITLE_ID)/
	cp $(APP)/release/PUT-YOUR-FILES-HERE.txt $(REL)/$(TITLE_ID)/pico8/
	sed -e 's|@VERSION@|$(VERSION)|' -e 's|@COMMIT@|$(COMMIT)|' -e 's|@TITLE_ID@|$(TITLE_ID)|g' \
	    -e 's|@SOURCE@|$(SOURCE_URL)|' $(APP)/release/README.txt > $(REL)/README.txt
	cp ../deps/boilerplate/LICENSE                  $(REL)/licenses/boilerplate-GPL-3.0.txt
	cp ../deps/boilerplate/THIRD_PARTY_NOTICES.md   $(REL)/licenses/boilerplate-THIRD_PARTY_NOTICES.md
	cp ../deps/SDL/LICENSE.txt                      $(REL)/licenses/SDL2-zlib.txt
	cp ../deps/curl-*/COPYING                       $(REL)/licenses/curl.txt
	cp ../deps/mbedtls-*/LICENSE                    $(REL)/licenses/mbedtls-Apache-2.0.txt
	sed -n '2,/opensource.org\/license\/mit-0/p' ../deps/dlmalloc/malloc.c \
	    > $(REL)/licenses/dlmalloc-MIT-0.txt
	printf 'cacert.pem: Mozilla root certificates (MPL-2.0), from https://curl.se/ca/cacert.pem\n' \
	    > $(REL)/licenses/cacert.txt
	cd ../release && zip -qr $(REL_NAME).zip $(REL_NAME)
	@echo "pacote: release/$(REL_NAME).zip"

# Alvos que falam com o PS5 precisam do IP.
need-host:
	@test -n "$(PS5_HOST)" || { echo "defina PS5_HOST=<ip do PS5> (ou crie pico8_loader/local.mk)" >&2; exit 1; }

APP_FILES := eboot.bin sandbox-elevator.elf sce_module/libc.prx \
             sce_sys/param.json sce_sys/icon0.png

install-app: app ../deps/cacert.pem need-host
	-@curl -sS -Q "DELE /data/pico8/loader.log" $(FTP)/ >/dev/null 2>&1
	for f in $(APP_FILES); do \
	    curl -sS --ftp-create-dirs -T $(APP_DIR)/$$f $(FTP)$(HB_DIR)/$$f || exit 1; \
	done
	curl -sS -T ../deps/cacert.pem $(FTP)$(HB_DIR)/cacert.pem

# Os arquivos do PICO-8 do usuario ficam dentro da pasta do app
# (pico8_loader/paths.c), como os de firmware no ProsperoEden.
upload-data: need-host
	curl -sS --ftp-create-dirs -T $(PICO8_DIR)/pico8_dyn $(FTP)$(HB_DIR)/pico8/
	curl -sS --ftp-create-dirs -T $(PICO8_DIR)/pico8.dat $(FTP)$(HB_DIR)/pico8/

log: need-host
	@curl -sS $(FTP)/data/pico8/loader.log

clean:
	rm -rf $(APP)/build

.PHONY: all app release need-host install-app upload-data log clean
