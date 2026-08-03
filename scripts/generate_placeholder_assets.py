# -*- coding: utf-8 -*-
"""Generate placeholder art assets for 《星环遗章》 tech demo.

Output: assets/placeholder/
  backgrounds/  12 scene backgrounds (1280x720, vertical gradient + scene name)
  portraits/    21 character busts (600x900, transparent, silhouette + face emotion)
  enemies/      7 enemy silhouettes (400x400, transparent)
  ui/           UI kit (9-slice panel, buttons 3-state, bars, frames, icons, logo)

All textures are CC0-style placeholder art generated locally (no external assets).
"""

import math
import os

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "assets", "placeholder")
FONT_PATH = os.path.join(ROOT, "assets", "fonts", "NotoSansSC.ttf")


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------

def font(size):
    return ImageFont.truetype(FONT_PATH, size)


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def shade(c, f):
    """Scale an RGB color by factor f (toward black)."""
    return tuple(max(0, min(255, int(v * f))) for v in c)


def lighten(c, f):
    """Blend an RGB color toward white by factor f."""
    return tuple(int(v + (255 - v) * f) for v in c)


def vertical_gradient(size, top, bottom):
    w, h = size
    img = Image.new("RGB", (w, h))
    px = img.load()
    for y in range(h):
        c = lerp(top, bottom, y / max(h - 1, 1))
        for x in range(w):
            px[x, y] = c
    return img


def glow(img, cx, cy, radius, color, alpha=60):
    """Add a soft radial glow to an RGB image."""
    d = ImageDraw.Draw(img, "RGBA")
    steps = 12
    for i in range(steps, 0, -1):
        r = radius * i / steps
        a = int(alpha * (steps - i + 1) / steps)
        d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=color + (a,))


def text_center(img, text, fnt, color, cy, alpha=255):
    """Draw text horizontally centered, vertically at cy (anchor style)."""
    d = ImageDraw.Draw(img, "RGBA")
    bbox = d.textbbox((0, 0), text, font=fnt)
    w = bbox[2] - bbox[0]
    x = (img.width - w) // 2 - bbox[0]
    y = cy - bbox[1]
    d.text((x, y), text, font=fnt, fill=color + (alpha,))


def star_points(cx, cy, outer, inner, n=5, rot=-math.pi / 2):
    pts = []
    for i in range(n * 2):
        r = outer if i % 2 == 0 else inner
        a = rot + i * math.pi / n
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


# ---------------------------------------------------------------------------
# backgrounds
# ---------------------------------------------------------------------------

BACKGROUNDS = [
    # (filename, chinese name, top color, bottom color)
    ("bg_library_hall",   "图书馆大厅",   (86, 66, 44),  (32, 24, 16)),
    ("bg_library_fire",   "失火夜·图书馆", (120, 44, 28), (24, 12, 14)),
    ("bg_forest_1",       "灰雾森林·入口", (92, 104, 78), (26, 34, 26)),
    ("bg_forest_2",       "灰雾森林·木屋", (58, 72, 54), (16, 22, 18)),
    ("bg_city_1",         "锈蚀之城·城门", (122, 82, 46), (34, 28, 26)),
    ("bg_city_2",         "锈蚀之城·钟楼", (96, 78, 62), (28, 26, 28)),
    ("bg_throne_1",       "静默王座·议会", (88, 72, 108), (26, 20, 38)),
    ("bg_throne_2",       "静默王座·王座", (72, 56, 96), (18, 14, 30)),
    ("bg_ring_top",       "星环顶端",     (30, 44, 84),  (8, 10, 24)),
    ("bg_hallway",        "书之回廊",     (44, 62, 78),  (14, 20, 30)),
    ("bg_ending_1",       "结局·合上书",  (128, 96, 52), (40, 28, 16)),
    ("bg_ending_2",       "结局·同行",    (110, 118, 128), (36, 42, 52)),
]


def gen_backgrounds():
    d = os.path.join(OUT, "backgrounds")
    os.makedirs(d, exist_ok=True)
    for name, cn, top, bottom in BACKGROUNDS:
        img = vertical_gradient((1280, 720), top, bottom)
        # vignette: darker bottom corners feel
        glow(img, 640, 200, 420, lighten(top, 0.35), alpha=26)
        glow(img, 640, 620, 380, shade(bottom, 0.4), alpha=40)
        # stars for night scenes
        if name in ("bg_ring_top", "bg_hallway"):
            dd = ImageDraw.Draw(img, "RGBA")
            import random
            rnd = random.Random(hash(name) & 0xFFFF)
            for _ in range(90):
                x, y = rnd.randrange(0, 1280), rnd.randrange(0, 420)
                r = rnd.uniform(0.6, 1.8)
                dd.ellipse([x - r, y - r, x + r, y + r], fill=(230, 235, 255, 200))
        # scene name
        fnt = font(56)
        text_center(img, cn, fnt, (255, 255, 255), 336, alpha=90)
        fnt2 = font(22)
        text_center(img, "占位背景 · Placeholder", fnt2, (255, 255, 255), 408, alpha=70)
        img.save(os.path.join(d, name + ".png"))
        print("background:", name)


# ---------------------------------------------------------------------------
# portraits (silhouette + face emotion)
# ---------------------------------------------------------------------------

# face emotions drawn on the silhouette head
def draw_face(draw, cx, cy, emotion, ink):
    """cx, cy = head center; ink = dark color for eyes/mouth."""
    def dot(x, y, r=7):
        draw.ellipse([x - r, y - r, x + r, y + r], fill=ink)

    def line(x0, y0, x1, y1, w=4):
        draw.line([x0, y0, x1, y1], fill=ink, width=w)

    def arc(box, start, end, w=4):
        draw.arc(box, start, end, fill=ink, width=w)

    e = 26  # eye offset
    if emotion == "normal":
        dot(cx - e, cy - 8); dot(cx + e, cy - 8)
        line(cx - 14, cy + 30, cx + 14, cy + 30)
    elif emotion == "smile":
        dot(cx - e, cy - 10); dot(cx + e, cy - 10)
        arc([cx - 16, cy + 16, cx + 16, cy + 42], 0, 180, 5)
    elif emotion == "serious":
        dot(cx - e, cy - 6); dot(cx + e, cy - 6)
        line(cx - 30, cy - 26, cx - 12, cy - 18, 5)
        line(cx + 30, cy - 26, cx + 12, cy - 18, 5)
        line(cx - 12, cy + 28, cx + 12, cy + 28, 5)
    elif emotion == "surprised":
        draw.ellipse([cx - e - 9, cy - 18, cx - e + 9, cy + 2], outline=ink, width=5)
        draw.ellipse([cx + e - 9, cy - 18, cx + e + 9, cy + 2], outline=ink, width=5)
        draw.ellipse([cx - 7, cy + 22, cx + 7, cy + 38], fill=ink)
    elif emotion == "sad":
        arc([cx - e - 12, cy - 22, cx - e + 12, cy - 2], 0, 180, 5)
        arc([cx + e - 12, cy - 22, cx + e + 12, cy - 2], 0, 180, 5)
        arc([cx - 16, cy + 24, cx + 16, cy + 44], 180, 360, 5)
    elif emotion == "tears":
        draw.ellipse([cx - e - 9, cy - 18, cx - e + 9, cy + 2], outline=ink, width=5)
        draw.ellipse([cx + e - 9, cy - 18, cx + e + 9, cy + 2], outline=ink, width=5)
        draw.ellipse([cx - e - 5, cy + 16, cx - e + 3, cy + 30], fill=(120, 170, 255))
        draw.ellipse([cx + e - 3, cy + 16, cx + e + 5, cy + 30], fill=(120, 170, 255))
        arc([cx - 14, cy + 26, cx + 14, cy + 44], 180, 360, 5)
    elif emotion == "angry":
        line(cx - 34, cy - 22, cx - 12, cy - 14, 5)
        line(cx + 34, cy - 22, cx + 12, cy - 14, 5)
        dot(cx - e, cy - 4, 6); dot(cx + e, cy - 4, 6)
        line(cx - 12, cy + 28, cx + 12, cy + 28, 5)
    elif emotion == "proud":
        arc([cx - e - 14, cy - 26, cx - e + 14, cy + 2], 180, 360, 5)
        arc([cx + e - 14, cy - 26, cx + e + 14, cy + 2], 180, 360, 5)
        arc([cx - 16, cy + 14, cx + 16, cy + 40], 0, 180, 5)
        line(cx - 30, cy - 34, cx - 14, cy - 26, 5)
        line(cx + 30, cy - 34, cx + 14, cy - 26, 5)
    else:
        dot(cx - e, cy - 8); dot(cx + e, cy - 8)
        line(cx - 14, cy + 30, cx + 14, cy + 30)


def draw_bust(draw, color, dark, accent, y_top=150):
    """Generic humanoid bust silhouette: head + shoulders, facing forward."""
    cx = 300
    # shoulders / torso
    torso = [cx - 250, y_top + 240, cx + 250, 920]
    draw.rounded_rectangle(torso, radius=110, fill=color)
    # neck
    draw.rounded_rectangle([cx - 42, y_top + 190, cx + 42, y_top + 250], radius=20, fill=dark)
    # head
    head_cy = y_top + 120
    draw.ellipse([cx - 120, head_cy - 130, cx + 120, head_cy + 130], fill=dark)
    # face plate (lighter skin tone)
    draw.ellipse([cx - 92, head_cy - 100, cx + 92, head_cy + 96], fill=accent)
    # collar accent
    draw.rounded_rectangle([cx - 110, y_top + 300, cx + 110, y_top + 330], radius=12, fill=dark)
    return cx, head_cy


PORTRAITS = [
    # (name, chinese, emotion list, main color)
    ("yinchuan",  "尹川",   ["normal", "smile", "serious", "surprised"],     (46, 74, 98)),
    ("suyan",     "苏言",   ["normal", "smile", "sad", "surprised", "serious", "tears"], (122, 179, 213)),
    ("jin",       "烬",     ["normal", "serious", "angry"],                  (138, 80, 70)),
    ("qixia",     "栖霞",   ["normal", "proud", "panic", "sad"],             (150, 110, 46)),
    ("shenyanqiu","沈砚秋", ["normal", "serious"],                            (122, 132, 142)),
]

EMOTION_CN = {
    "normal": "普通", "smile": "微笑", "serious": "严肃", "surprised": "惊讶",
    "sad": "落寞", "tears": "含泪", "angry": "愤怒", "proud": "得意", "panic": "慌张",
}


def gen_portraits():
    d = os.path.join(OUT, "portraits")
    os.makedirs(d, exist_ok=True)
    for name, cn, emotions, color in PORTRAITS:
        for emo in emotions:
            img = Image.new("RGBA", (600, 900), (0, 0, 0, 0))
            draw = ImageDraw.Draw(img)
            dark = shade(color, 0.45)
            accent = lighten(color, 0.55)
            cx, head_cy = draw_bust(draw, color, dark, accent)
            draw_face(draw, cx, head_cy, emo, (36, 30, 34))
            # name plate at bottom
            draw.rounded_rectangle([150, 790, 450, 862], radius=18, fill=(20, 22, 28, 190))
            fnt = font(34)
            text_center(img, cn, fnt, (255, 255, 255), 820)
            fnt2 = font(22)
            text_center(img, EMOTION_CN.get(emo, emo), fnt2, (200, 210, 225), 818, alpha=200)
            img.save(os.path.join(d, "%s_%s.png" % (name, emo)))
            print("portrait:", name, emo)
    # faceless (boss) two forms
    for fname, label, color in (("faceless_false", "无面者·伪", (58, 44, 74)),
                                ("faceless_true",  "无面者·真", (30, 26, 44))):
        img = Image.new("RGBA", (600, 900), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rounded_rectangle([60, 320, 540, 900], radius=90, fill=color)
        draw.rounded_rectangle([220, 200, 380, 360], radius=46, fill=shade(color, 0.7))
        draw.rectangle([232, 268, 368, 340], fill=(238, 240, 248))  # blank page face
        draw.rounded_rectangle([150, 790, 450, 862], radius=18, fill=(20, 22, 28, 190))
        fnt = font(34)
        text_center(img, label, fnt, (255, 255, 255), 820)
        img.save(os.path.join(d, fname + ".png"))
        print("portrait:", fname)


# ---------------------------------------------------------------------------
# enemies (400x400 silhouettes)
# ---------------------------------------------------------------------------

def enemy_plate(img, draw, label, color):
    draw.rounded_rectangle([90, 348, 310, 396], radius=14, fill=(20, 22, 28, 190))
    fnt = font(28)
    text_center(img, label, fnt, (255, 255, 255), 370)


def gen_enemies():
    d = os.path.join(OUT, "enemies")
    os.makedirs(d, exist_ok=True)

    def new():
        return Image.new("RGBA", (400, 400), (0, 0, 0, 0))

    # erosion moth: torn paper sheet
    img = new(); draw = ImageDraw.Draw(img)
    pts = [(70, 200), (140, 110), (230, 90), (320, 150), (340, 250), (280, 330), (150, 340), (80, 280)]
    draw.polygon(pts, fill=(150, 140, 160))
    draw.ellipse([170, 170, 230, 230], fill=(30, 20, 40))
    enemy_plate(img, draw, "蚀页魔", (150, 140, 160))
    img.save(os.path.join(d, "erosion_moth.png")); print("enemy: erosion_moth")

    # mist wolf
    img = new(); draw = ImageDraw.Draw(img)
    draw.polygon([(60, 230), (130, 200), (250, 205), (330, 235), (300, 290), (190, 305), (90, 290)], fill=(92, 102, 112))
    draw.polygon([(120, 180), (140, 110), (170, 165)], fill=(92, 102, 112))   # ear
    draw.polygon([(240, 175), (260, 105), (285, 160)], fill=(92, 102, 112))
    draw.polygon([(330, 235), (380, 225), (345, 262)], fill=(92, 102, 112))   # snout
    draw.ellipse([262, 214, 282, 234], fill=(210, 60, 60))                    # eye
    enemy_plate(img, draw, "雾影狼", (92, 102, 112))
    img.save(os.path.join(d, "mist_wolf.png")); print("enemy: mist_wolf")

    # rust golem
    img = new(); draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([90, 130, 310, 330], radius=34, fill=(120, 88, 62))
    draw.rounded_rectangle([140, 60, 260, 150], radius=24, fill=(96, 70, 50))
    draw.rectangle([150, 95, 178, 115], fill=(30, 22, 16))
    draw.rectangle([222, 95, 250, 115], fill=(30, 22, 16))
    draw.ellipse([150, 180, 182, 212], outline=(60, 44, 32), width=8)
    draw.ellipse([218, 180, 250, 212], outline=(60, 44, 32), width=8)
    enemy_plate(img, draw, "锈铁傀儡", (120, 88, 62))
    img.save(os.path.join(d, "rust_golem.png")); print("enemy: rust_golem")

    # silent priest: tall robed figure + peaked cap
    img = new(); draw = ImageDraw.Draw(img)
    draw.polygon([(160, 60), (200, 20), (240, 60)], fill=(90, 82, 108))
    draw.ellipse([148, 55, 252, 150], fill=(90, 82, 108))
    draw.polygon([(120, 140), (280, 140), (310, 380), (90, 380)], fill=(76, 68, 94))
    draw.ellipse([176, 92, 224, 140], fill=(230, 228, 240))
    draw.ellipse([183, 102, 197, 116], fill=(24, 18, 34))
    draw.ellipse([203, 102, 217, 116], fill=(24, 18, 34))
    enemy_plate(img, draw, "静默主教", (90, 82, 108))
    img.save(os.path.join(d, "silent_priest.png")); print("enemy: silent_priest")

    # rust commander: broad shoulders, pauldrons
    img = new(); draw = ImageDraw.Draw(img)
    draw.ellipse([130, 40, 270, 180], fill=(110, 78, 56))
    draw.rounded_rectangle([60, 160, 340, 380], radius=56, fill=(126, 92, 66))
    draw.rounded_rectangle([40, 150, 130, 230], radius=26, fill=(88, 62, 44))   # pauldrons
    draw.rounded_rectangle([270, 150, 360, 230], radius=26, fill=(88, 62, 44))
    draw.rectangle([150, 82, 178, 106], fill=(26, 18, 12))
    draw.rectangle([222, 82, 250, 106], fill=(26, 18, 12))
    draw.line([150, 140, 250, 140], fill=(26, 18, 12), width=6)
    enemy_plate(img, draw, "锈铁战团长", (126, 92, 66))
    img.save(os.path.join(d, "rust_commander.png")); print("enemy: rust_commander")

    # faceless boss forms reuse portrait style at enemy size
    for fname, label, color in (("faceless_false", "无面者·伪", (52, 40, 66)),
                                ("faceless_true",  "无面者·真", (24, 20, 38))):
        img = new(); draw = ImageDraw.Draw(img)
        draw.rounded_rectangle([60, 110, 340, 380], radius=60, fill=color)
        draw.rounded_rectangle([170, 60, 230, 150], radius=30, fill=shade(color, 0.7))
        draw.rectangle([178, 100, 222, 148], fill=(238, 240, 248))
        enemy_plate(img, draw, label, color)
        img.save(os.path.join(d, fname + ".png")); print("enemy:", fname)


# ---------------------------------------------------------------------------
# UI kit
# ---------------------------------------------------------------------------

def gen_ui():
    d = os.path.join(OUT, "ui")
    os.makedirs(d, exist_ok=True)

    # 9-slice panel 64x64 (dark translucent + gold rim)
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([3, 3, 60, 60], radius=10, fill=(22, 26, 34, 200),
                           outline=(196, 168, 108), width=3)
    img.save(os.path.join(d, "panel_9slice.png")); print("ui: panel_9slice")

    # buttons 3-state, two sizes
    def gen_button(fname, w, h, base, edge):
        img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        grad = Image.new("RGB", (w, h))
        px = grad.load()
        for y in range(h):
            c = lerp(lighten(base, 0.18), shade(base, 0.55), y / max(h - 1, 1))
            for x in range(w):
                px[x, y] = c
        mask = Image.new("L", (w, h), 0)
        md = ImageDraw.Draw(mask)
        md.rounded_rectangle([0, 0, w - 1, h - 1], radius=h // 2, fill=255)
        img.paste(grad, (0, 0), mask)
        draw = ImageDraw.Draw(img)
        draw.rounded_rectangle([0, 0, w - 1, h - 1], radius=h // 2,
                               outline=edge, width=3)
        draw.line([4, 4, w - 5, 4], fill=lighten(base, 0.45), width=2)
        img.save(os.path.join(d, fname)); print("ui:", fname)

    gen_button("btn_main_normal.png", 220, 56, (58, 84, 122), (150, 190, 230))
    gen_button("btn_main_hover.png", 220, 56, (74, 104, 148), (190, 225, 255))
    gen_button("btn_main_pressed.png", 220, 56, (38, 58, 88), (110, 150, 190))
    gen_button("btn_icon_normal.png", 64, 64, (50, 72, 104), (140, 176, 214))
    gen_button("btn_icon_hover.png", 64, 64, (66, 92, 130), (180, 214, 248))
    gen_button("btn_icon_pressed.png", 64, 64, (34, 50, 76), (100, 136, 176))

    # bars
    def gen_bar(fname, fill_color):
        img = Image.new("RGBA", (320, 24), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rounded_rectangle([0, 0, 319, 23], radius=10, fill=fill_color)
        draw.rounded_rectangle([1, 1, 318, 22], radius=9, outline=(255, 255, 255, 90), width=2)
        img.save(os.path.join(d, fname)); print("ui:", fname)

    gen_bar("hpbar_bg.png", (30, 26, 30, 230))
    gen_bar("hpbar_fill.png", (92, 168, 96, 255))
    gen_bar("spbar_bg.png", (26, 30, 34, 230))
    gen_bar("spbar_fill.png", (88, 148, 210, 255))

    # frames
    img = Image.new("RGBA", (96, 96), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([2, 2, 93, 93], radius=16, outline=(220, 222, 228, 220), width=4)
    img.save(os.path.join(d, "portrait_frame.png")); print("ui: portrait_frame")

    img = Image.new("RGBA", (128, 48), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rounded_rectangle([1, 1, 126, 46], radius=10, fill=(24, 28, 38, 170),
                           outline=(170, 150, 100, 200), width=2)
    img.save(os.path.join(d, "action_slot.png")); print("ui: action_slot")

    # status icons (48x48, circle base + glyph)
    status = [
        ("icon_status_atk_up",   "↑", (180, 120, 80)),
        ("icon_status_atk_dn",   "↓", (140, 90, 90)),
        ("icon_status_shield",   "盾", (110, 140, 180)),
        ("icon_status_charge",   "蓄", (190, 160, 90)),
        ("icon_status_seal",     "封", (150, 110, 170)),
        ("icon_status_spd_up",   "速", (110, 170, 150)),
        ("icon_status_spd_dn",   "滞", (120, 120, 140)),
        ("icon_status_poison",   "毒", (120, 160, 80)),
    ]
    for fname, glyph, color in status:
        img = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.ellipse([2, 2, 45, 45], fill=color + (230,))
        draw.ellipse([2, 2, 45, 45], outline=(255, 255, 255, 160), width=2)
        fnt = font(24)
        text_center(img, glyph, fnt, (255, 255, 255), 24, alpha=255)
        img.save(os.path.join(d, fname + ".png")); print("ui:", fname)

    # skill icons (64x64, attribute color ring + glyph letter)
    skills = [
        ("icon_skill_light_a", "光", (240, 214, 120)),
        ("icon_skill_light_b", "辉", (240, 214, 120)),
        ("icon_skill_light_c", "愈", (232, 226, 190)),
        ("icon_skill_water_a", "水", (110, 170, 220)),
        ("icon_skill_water_b", "潮", (110, 170, 220)),
        ("icon_skill_water_c", "缓", (130, 190, 230)),
        ("icon_skill_fire_a",  "火", (220, 110, 70)),
        ("icon_skill_fire_b",  "破", (220, 110, 70)),
        ("icon_skill_fire_c",  "壁", (190, 100, 80)),
        ("icon_skill_thunder_a", "雷", (230, 200, 80)),
        ("icon_skill_thunder_b", "疾", (230, 200, 80)),
        ("icon_skill_thunder_c", "盾", (210, 180, 100)),
    ]
    for fname, glyph, color in skills:
        img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rounded_rectangle([2, 2, 61, 61], radius=12, fill=shade(color, 0.25) + (235,),
                               outline=color + (255,), width=3)
        fnt = font(32)
        text_center(img, glyph, fnt, (255, 255, 255), 32, alpha=255)
        img.save(os.path.join(d, fname + ".png")); print("ui:", fname)

    # item icons
    items = [
        ("icon_item_herb",   "药", (120, 170, 90)),
        ("icon_item_dew",    "露", (100, 150, 210)),
        ("icon_item_clear",  "清", (160, 190, 130)),
        ("icon_item_fragment", "忆", (200, 170, 110)),
    ]
    for fname, glyph, color in items:
        img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
        draw = ImageDraw.Draw(img)
        draw.rounded_rectangle([2, 2, 61, 61], radius=12, fill=color + (235,),
                               outline=(255, 255, 255, 170), width=2)
        fnt = font(32)
        text_center(img, glyph, fnt, (255, 255, 255), 32, alpha=255)
        img.save(os.path.join(d, fname + ".png")); print("ui:", fname)

    # next-page arrow (48x32, rounded triangle)
    img = Image.new("RGBA", (48, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.polygon([(6, 4), (42, 16), (6, 28)], fill=(240, 244, 250, 240))
    img.save(os.path.join(d, "arrow_next.png")); print("ui: arrow_next")

    # title logo (512x512, ring + stars + title text)
    img = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([96, 96, 416, 416], outline=(200, 190, 160, 255), width=10)
    draw.ellipse([130, 130, 382, 382], outline=(160, 150, 130, 120), width=4)
    for i in range(8):
        a = i * math.pi / 4
        x = 256 + 250 * math.cos(a)
        y = 256 + 250 * math.sin(a)
        r = 8 if i % 2 == 0 else 5
        draw.ellipse([x - r, y - r, x + r, y + r], fill=(232, 222, 190, 255))
    fnt = font(64)
    text_center(img, "星环遗章", fnt, (240, 232, 210), 256, alpha=255)
    img.save(os.path.join(d, "logo_ring.png")); print("ui: logo_ring")


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main():
    os.makedirs(OUT, exist_ok=True)
    gen_backgrounds()
    gen_portraits()
    gen_enemies()
    gen_ui()
    print("ALL DONE ->", OUT)


if __name__ == "__main__":
    main()
