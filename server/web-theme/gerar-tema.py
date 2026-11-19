"""Gera o tema do Drive para o ownCloud Web 7.0.3 (pasta dbpixel/): logos em SVG com "Drive" em curvas,
fundo do login, ícone da aba e o theme.json com as cores do produto (manual 3.9 e 9.2).

Uso: python -I gerar-tema.py   (precisa do fontTools; as fontes vêm de src/gui/fonts)
"""
import json
import pathlib
import shutil
import sys

HERE = pathlib.Path(__file__).parent
ROOT = HERE.parent.parent
sys.path.insert(0, str(pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else HERE))
from fontTools.pens.svgPathPen import SVGPathPen  # noqa: E402
from fontTools.pens.transformPen import TransformPen  # noqa: E402
from fontTools.ttLib import TTFont  # noqa: E402

OUT = HERE / "dbpixel"
ASSETS = OUT / "assets"
ASSETS.mkdir(parents=True, exist_ok=True)

NAVY, STEEL, BTN, TEXT, ONDARK, SOFT = "#041F46", "#6F8BB8", "#47689A", "#43638F", "#9DB6DC", "#EBF0F7"
INK, MUTED = "#0A1222", "#5E6573"


def text_path(text, ttf, size, x, baseline):
    """Desenha o texto como um caminho SVG (sem depender da fonte no navegador)."""
    font = TTFont(str(ttf))
    gs = font.getGlyphSet()
    cmap = font.getBestCmap()
    scale = size / font["head"].unitsPerEm
    pen = SVGPathPen(gs)
    cursor = x
    for ch in text:
        g = cmap[ord(ch)]
        tp = TransformPen(pen, (scale, 0, 0, -scale, cursor, baseline))
        gs[g].draw(tp)
        cursor += gs[g].width * scale
    return pen.getCommands(), cursor


TILE = '<polygon points="0,0 18.7,0 24,5.3 24,24 5.3,24 0,18.7" fill="{c}"/><path d="M7.4 16.6h9.2a3.3 3.3 0 0 0 .2-6.6 4.7 4.7 0 0 0-9 1.1 2.8 2.8 0 0 0-.4 5.5z" fill="#FFFFFF"/>'
M800 = ROOT / "src/gui/fonts/Montserrat-800.ttf"


def logo(name_color, tile_color=STEEL):
    # quadrado de 24 unidades e o nome com 16 de corpo, como a marca do app (DriveMark)
    d, end = text_path("Drive", M800, 16.5, 32, 18)
    w = round(end + 1, 1)
    return (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} 24" width="{w * 2}" height="48" role="img" aria-label="Drive">'
            f'<g>{TILE.format(c=tile_color)}</g><path d="{d}" fill="{name_color}"/></svg>')


(ASSETS / "logo.svg").write_text(logo("#FFFFFF"), encoding="utf-8")          # barra do topo (fundo navy)
(ASSETS / "logo-escuro.svg").write_text(logo(INK), encoding="utf-8")         # sobre fundo claro (login do web)
(ASSETS / "nuvem.svg").write_text(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" width="96" height="96">{TILE.format(c="#C9D4E5")}</svg>', encoding="utf-8")
# fundo do login: navy com os cortes chanfrados da marca
(ASSETS / "fundo-login.svg").write_text(
    f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1600 1000" preserveAspectRatio="xMidYMid slice"><rect width="1600" height="1000" fill="{NAVY}"/>'
    f'<polygon points="1180,0 1600,0 1600,420" fill="{STEEL}" opacity=".9"/><polygon points="0,700 0,1000 300,1000" fill="#0B2A57"/></svg>', encoding="utf-8")
shutil.copy(ROOT / "branding/theme/colored/64-drive-icon.png", ASSETS / "favicon.png")
# fontes só com os caracteres do português (Latin-1 + pontuação), em WOFF: ~10x menores que os TTF do app
from fontTools import subset  # noqa: E402
for f in ("Montserrat-600", "Montserrat-700", "Montserrat-800", "NunitoSans-400", "NunitoSans-600", "NunitoSans-700"):
    (ASSETS / f"{f}.ttf").unlink(missing_ok=True)
    opts = subset.Options()
    opts.flavor = "woff"
    opts.layout_features = ["kern", "liga", "calt", "tnum"]
    font = subset.load_font(str(ROOT / f"src/gui/fonts/{f}.ttf"), opts)
    s = subset.Subsetter(opts)
    s.populate(unicodes=list(range(0x20, 0x7F)) + list(range(0xA0, 0x100)) + [0x2013, 0x2014, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2026, 0x20AC])
    s.subset(font)
    subset.save_font(font, str(ASSETS / f"{f}.woff"), opts)

BASE = "themes/dbpixel/assets"
light = {
    "background-accentuate": "rgba(111, 139, 184, 0.12)",
    "background-default": "#FFFFFF", "background-highlight": SOFT, "background-muted": "#F7F8FA",
    "background-secondary": "#FFFFFF", "background-hover": "#F1F4F9", "border": "#E6E8EC",
    "input-bg": "#FFFFFF", "input-border": "#D0D5DD", "input-text-default": INK, "input-text-muted": MUTED,
    "swatch-brand-default": NAVY, "swatch-brand-hover": "#0B2A57", "swatch-brand-contrast": "#FFFFFF",
    "swatch-danger-default": "#B42318", "swatch-danger-hover": "#912018", "swatch-danger-muted": "#F3B7B2", "swatch-danger-contrast": "#FFFFFF",
    "swatch-inverse-default": "#FFFFFF", "swatch-inverse-hover": "#FFFFFF", "swatch-inverse-muted": "#C9D4E5",
    "swatch-passive-default": MUTED, "swatch-passive-hover": "#3A404C", "swatch-passive-muted": "#98A2B3", "swatch-passive-contrast": "#FFFFFF",
    "swatch-primary-default": BTN, "swatch-primary-hover": "#3F5F8F", "swatch-primary-muted": TEXT, "swatch-primary-gradient": BTN, "swatch-primary-contrast": "#FFFFFF",
    "swatch-success-default": "#146C36", "swatch-success-hover": "#0F5A2C", "swatch-success-muted": "#A6D9B8", "swatch-success-contrast": "#FFFFFF",
    "swatch-warning-default": "#B84000", "swatch-warning-hover": "#9A3600", "swatch-warning-muted": "rgba(184, 64, 0, .5)", "swatch-warning-contrast": "#FFFFFF",
    "text-default": INK, "text-inverse": "#FFFFFF", "text-muted": MUTED,
    "icon-folder": STEEL, "icon-archive": "#B54708", "icon-image": "#7A5AF8", "icon-spreadsheet": "#1D7A46", "icon-document": "#1570EF",
    "icon-video": "#C11574", "icon-audio": "#0E9384", "icon-presentation": "#D9480F", "icon-pdf": "#D92D20",
}
dark = dict(light, **{
    "background-accentuate": "rgba(157, 182, 220, 0.16)", "background-default": "#0B1220", "background-highlight": "#1B2A44",
    "background-muted": "#111A2B", "background-secondary": "#152036", "background-hover": "#18243A", "border": "#243149",
    "input-bg": "#111A2B", "input-border": "#33415C", "input-text-default": "#E8ECF2", "input-text-muted": "#A3A9B5",
    "swatch-brand-default": "#060B16", "swatch-brand-hover": "#111A2B",
    "swatch-primary-default": ONDARK, "swatch-primary-hover": "#B5C9E6", "swatch-primary-muted": ONDARK, "swatch-primary-gradient": ONDARK, "swatch-primary-contrast": "#0A1222",
    "swatch-passive-default": "#A3A9B5", "swatch-passive-muted": "#6B7385", "text-default": "#E8ECF2", "text-inverse": "#0A1222", "text-muted": "#A3A9B5",
    "icon-folder": ONDARK,
})


def variant(colors):
    return {
        "general": {"name": "Drive", "slogan": "Drive · um produto DBPixel"},
        "logo": {"topbar": f"{BASE}/logo.svg", "favicon": f"{BASE}/favicon.png", "login": f"{BASE}/logo-escuro.svg", "notFound": f"{BASE}/nuvem.svg"},
        "loginPage": {"autoRedirect": True, "backgroundImg": f"{BASE}/fundo-login.svg"},
        "designTokens": {"colorPalette": colors},
    }


theme = {"common": {"name": "Drive", "slogan": "Drive · um produto DBPixel", "logo": f"{BASE}/logo.svg"},
         "ios": {}, "web": {"default": variant(light), "default-dark": variant(dark)}}
(OUT / "theme.json").write_text(json.dumps(theme, ensure_ascii=False, indent=2), encoding="utf-8")
print("tema gerado em", OUT)
