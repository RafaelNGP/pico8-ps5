#!/usr/bin/env python3
"""Gera o icon0.png (512x512) do app na home.

Usa o lexaloffle-pico8.png da instalacao do proprio usuario, ampliado sem
suavizacao, para que nenhum arquivo da Lexaloffle entre no repositorio.
Sem ele, desenha um icone generico.

Uso: make_icon.py <saida.png> [lexaloffle-pico8.png]
"""
import os
import sys

from PIL import Image, ImageDraw

SIZE = 512
BG = (29, 43, 83)  # azul escuro da paleta do PICO-8


def from_user_logo(path):
    logo = Image.open(path).convert("RGBA")
    scale = SIZE // max(logo.size)
    logo = logo.resize((logo.width * scale, logo.height * scale), Image.NEAREST)
    icon = Image.new("RGB", (SIZE, SIZE), BG)
    icon.paste(logo, ((SIZE - logo.width) // 2, (SIZE - logo.height) // 2), logo)
    return icon


def generic():
    icon = Image.new("RGB", (SIZE, SIZE), BG)
    draw = ImageDraw.Draw(icon)
    # Faixa com as 8 cores vivas da paleta do PICO-8.
    palette = [(255, 0, 77), (255, 163, 0), (255, 236, 39), (0, 228, 54),
               (41, 173, 255), (131, 118, 156), (255, 119, 168), (255, 204, 170)]
    w = SIZE // len(palette)
    for i, c in enumerate(palette):
        draw.rectangle([i * w, 400, (i + 1) * w - 1, 440], fill=c)
    draw.text((SIZE // 2, 230), "P8", fill=(255, 241, 232), anchor="mm",
              font_size=220)
    return icon


def main():
    out = sys.argv[1]
    logo = sys.argv[2] if len(sys.argv) > 2 else ""
    icon = from_user_logo(logo) if logo and os.path.exists(logo) else generic()
    icon.save(out)


if __name__ == "__main__":
    main()
