#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CuinZip 官方图标生成器(原创设计)
================================

设计概念:"拉链 Z"(Zipper-Z)——CuinZip 之名取 "Zip",以拉链齿构成的 Z 字
作为品牌标记,蓝色渐变圆角方块为应用标志底板;文件关联图标为白色文档 +
蓝色拉链 Z。全部素材由本脚本程序化生成,不使用任何上游(NanaZip/7-Zip)
CC BY-ND 图标素材。

用法:python Assets/GenerateCuinZipAssets.py
依赖:Pillow

生成内容:
- Assets/CuinZip.ico / CuinZipPreview.ico(应用图标,13 种尺寸)
- Assets/CuinZipSfx.ico / CuinZipPreviewSfx.ico(SFX 自解压图标)
- Assets/CuinZip.png / CuinZipPreview.png(README 用 256px 标志)
- Assets/CuinZipPackageAssets/** 与 Assets/CuinZipPreviewPackageAssets/**
  (MSIX 全尺寸资产,370 个文件/目录)
"""

import os
from PIL import Image, ImageDraw

SS = 4  # 超采样抗锯齿倍数

# ---------------------------------------------------------------- 品牌色
GRAD_TOP = (78, 158, 246)     # 渐变顶 #4E9EF6
GRAD_BOTTOM = (24, 82, 194)   # 渐变底 #1852C2
MARK_BLUE = (27, 92, 196)     # 文档图标上的 Z 标 #1B5CC4
MARK_BLUE_LIGHT = (127, 179, 240)  # lightunplated 变体
DOC_BODY = (255, 255, 255)
DOC_BORDER = (203, 212, 224)
DOC_FOLD = (233, 238, 245)

TILE_SIZES = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 90, 96, 128, 256]
ICO_SIZES = [(s, s) for s in TILE_SIZES]


def _s(v):
    """按超采样倍数放大坐标。"""
    return int(round(v * SS))


def vertical_gradient(size, top, bottom):
    """对角线渐变(左上 -> 右下),size 为逻辑像素。

    渐变平滑无高频细节,先在 256px 分辨率生成再上采样,避免大尺寸逐像素循环。
    """
    gen_size = min(_s(size), 256 * SS)
    img = Image.new("RGBA", (gen_size, gen_size))
    px = img.load()
    for y in range(gen_size):
        ty = y / max(gen_size - 1, 1)
        for x in range(gen_size):
            t = (ty + x / max(gen_size - 1, 1)) / 2.0
            px[x, y] = (
                int(top[0] + (bottom[0] - top[0]) * t),
                int(top[1] + (bottom[1] - top[1]) * t),
                int(top[2] + (bottom[2] - top[2]) * t),
                255,
            )
    if gen_size != _s(size):
        img = img.resize((_s(size), _s(size)), Image.BILINEAR)
    return img


def squircle_mask(size, radius_ratio=0.225):
    """圆角方块蒙版(全幅)。"""
    img = Image.new("L", (_s(size), _s(size)), 0)
    d = ImageDraw.Draw(img)
    d.rounded_rectangle(
        [0, 0, _s(size) - 1, _s(size) - 1],
        radius=_s(size * radius_ratio),
        fill=255,
    )
    return img


def rounded_mask(size, radius_ratio):
    img = Image.new("L", (_s(size), _s(size)), 0)
    d = ImageDraw.Draw(img)
    d.rounded_rectangle(
        [0, 0, _s(size) - 1, _s(size) - 1],
        radius=_s(size * radius_ratio),
        fill=255,
    )
    return img


def draw_zipper_z(img, box, color, with_teeth=True, with_slider=True):
    """在 img 上绘制拉链 Z 标记。box=(x0,y0,x1,y1) 逻辑像素。

    Z 的对角线由 5 段白色(或指定颜色)短划构成,段间留隙形成拉链齿;
    尺寸足够时在右上端加滑块。小尺寸自动退化为实心 Z。
    """
    x0, y0, x1, y1 = [_s(v) for v in box]
    w, h = x1 - x0, y1 - y0
    t = int(h * 0.215)            # 笔画粗细
    d = ImageDraw.Draw(img)

    def bar(ya, yb):
        d.rounded_rectangle([x0, ya, x1, yb], radius=t // 2, fill=color)

    bar(y0, y0 + t)               # 上横
    bar(y1 - t, y1)               # 下横

    # 对角线段(右上 -> 左下)
    px0, py0 = x1 - int(w * 0.16), y0 + int(t * 0.95)
    px1, py1 = x0 + int(w * 0.16), y1 - int(t * 0.95)
    lw = int(t * 0.80)

    if not with_teeth:
        d.line([px0, py0, px1, py1], fill=color, width=lw)
        # 圆头
        r = lw // 2
        for cx, cy in ((px0, py0), (px1, py1)):
            d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=color)
    else:
        segs = 5
        gap_ratio = 0.11          # 每个齿隙占对角线长度比例
        seg_ratio = (1.0 - (segs - 1) * gap_ratio) / segs
        pts = []
        for i in range(segs):
            a = i * (seg_ratio + gap_ratio)
            b = a + seg_ratio
            pts.append((
                (px0 + (px1 - px0) * a, py0 + (py1 - py0) * a),
                (px0 + (px1 - px0) * b, py0 + (py1 - py0) * b),
            ))
        for (ax, ay), (bx, by) in pts:
            d.line([ax, ay, bx, by], fill=color, width=lw)
            r = lw // 2
            for cx, cy in ((ax, ay), (bx, by)):
                d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=color)

    if with_slider:
        # 滑块:横跨对角线的圆角块(右上端)
        import math
        ux, uy = px0 + (px1 - px0) * 0.10, py0 + (py1 - py0) * 0.10
        ang = math.atan2(py1 - py0, px1 - px0)
        ca, sa = math.cos(ang), math.sin(ang)
        # 局部坐标 -> 全局(沿对角线方向为长边)
        L, W = t * 0.95, t * 1.28
        corners = [(-L / 2, -W / 2), (L / 2, -W / 2), (L / 2, W / 2), (-L / 2, W / 2)]
        poly = [(ux + cx * ca - cy * sa, uy + cx * sa + cy * ca) for cx, cy in corners]
        d.polygon(poly, fill=color)
        # 拉环头:垂直对角线的细长圆角块
        tabL, tabW = t * 1.15, t * 0.30
        tx, ty = ux, uy + (W / 2 + tabL / 2) * (1 if py1 > py0 else -1)
        poly2 = []
        for cx, cy in [(-tabW / 2, -tabL / 2), (tabW / 2, -tabL / 2),
                       (tabW / 2, tabL / 2), (-tabW / 2, tabL / 2)]:
            poly2.append((tx + cx * ca - cy * sa, ty + cx * sa + cy * ca))
        d.polygon(poly2, fill=color)


def app_tile(size, radius_ratio=0.225, fill_ratio=1.0):
    """应用标志:渐变圆角方块 + 白色拉链 Z。fill_ratio<1 时方块居中留边。"""
    canvas = Image.new("RGBA", (_s(size), _s(size)), (0, 0, 0, 0))
    tile_px = size * fill_ratio
    tile = vertical_gradient(int(tile_px), GRAD_TOP, GRAD_BOTTOM)
    mask = squircle_mask(int(tile_px), radius_ratio)
    off = int((size - tile_px) / 2 * SS)
    canvas.paste(tile, (off, off), mask)
    # 左上柔和高光
    hl = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
    hd = ImageDraw.Draw(hl)
    hd.ellipse(
        [-_s(size) * 0.35, -_s(size) * 0.45, _s(size) * 0.55, _s(size) * 0.45],
        fill=(255, 255, 255, 26),
    )
    canvas = Image.alpha_composite(canvas, hl)
    # Z 标记(留 21% 内边距)
    m = size * 0.21 * fill_ratio + (size - tile_px) / 2
    box = (m, m, size - m, size - m)
    teeth = size >= 40
    slider = size >= 64
    draw_zipper_z(canvas, box, (255, 255, 255, 255), teeth, slider)
    return canvas.resize((size, size), Image.LANCZOS)


def mark_only(size, color, radius_ratio=0.0):
    """仅 Z 标记(无底板),用于 unplated 变体。"""
    canvas = Image.new("RGBA", (_s(size), _s(size)), (0, 0, 0, 0))
    m = size * 0.19
    box = (m, m, size - m, size - m)
    draw_zipper_z(canvas, box, color, size >= 40, size >= 64)
    return canvas.resize((size, size), Image.LANCZOS)


def archive_icon(size):
    """文件关联图标:小尺寸用实心标志块,大尺寸用白色文档 + 蓝色拉链 Z。"""
    if size <= 40:
        return app_tile(size)
    canvas = Image.new("RGBA", (_s(size), _s(size)), (0, 0, 0, 0))
    d = ImageDraw.Draw(canvas)
    s = _s(size)
    pad = _s(size * 0.045)
    fold = _s(size * 0.26)
    bw = max(_s(size * 0.022), SS)  # 边框粗细
    # 文档主体
    d.rounded_rectangle(
        [pad, pad, s - pad, s - pad],
        radius=_s(size * 0.075),
        fill=DOC_BODY + (255,),
        outline=DOC_BORDER + (255,),
        width=bw,
    )
    # 折角
    d.polygon(
        [(s - pad - fold, pad), (s - pad, pad + fold), (s - pad - fold, pad + fold)],
        fill=DOC_FOLD + (255,),
    )
    d.line(
        [(s - pad - fold, pad), (s - pad, pad + fold), (s - pad, pad + fold)],
        fill=DOC_BORDER + (255,),
        width=bw,
    )
    # 蓝色拉链 Z(居中偏上)
    zw, zh = size * (0.50 if size < 72 else 0.46), size * 0.30
    cx, cy = size / 2, size * 0.54
    box = (cx - zw / 2, cy - zh / 2, cx + zw / 2, cy + zh / 2)
    draw_zipper_z(canvas, box, MARK_BLUE + (255,), size >= 48, size >= 72)
    return canvas.resize((size, size), Image.LANCZOS)


def monochrome_tile(size, color, radius_ratio=0.225):
    """纯色圆角方块(contrast 变体)。"""
    canvas = Image.new("RGBA", (_s(size), _s(size)), (0, 0, 0, 0))
    tile = Image.new("RGBA", (_s(size), _s(size)), color)
    canvas.paste(tile, (0, 0), squircle_mask(size, radius_ratio))
    return canvas.resize((size, size), Image.LANCZOS)


def save(img, path):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    img.save(path)


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    assets = os.path.join(root, "Assets")

    # ---- 应用 / SFX 图标(ico)与 README 标志(png)
    for name in ("CuinZip", "CuinZipPreview"):
        master = app_tile(256)
        save(master, os.path.join(assets, name + ".png"))
        master.save(os.path.join(assets, name + ".ico"), sizes=ICO_SIZES)
        master.save(os.path.join(assets, name + "Sfx.ico"), sizes=ICO_SIZES)

    # ---- MSIX 包资产(Preview 与 Release 两套内容一致)
    pkg_dirs = ["CuinZipPackageAssets", "CuinZipPreviewPackageAssets"]
    scales = [100, 125, 150, 200, 400]
    tile_bases = {
        "Square44x44Logo": 44,
        "Square150x150Logo": 150,
        "SmallTile": 71,
        "LargeTile": 310,
        "StoreLogo": 50,
    }
    target_sizes = [16, 20, 24, 30, 32, 36, 40, 48, 60, 64, 72, 80, 90, 96,
                    108, 120, 128, 144, 160, 192, 216, 256, 288, 320, 384,
                    512, 768, 1024]

    for pkg in pkg_dirs:
        base = os.path.join(assets, pkg)

        # ArchiveFile:文件关联图标
        for s in target_sizes:
            save(archive_icon(s), os.path.join(base, "ArchiveFile.targetsize-%d.png" % s))

        # Square44x44Logo:targetsize 系列(含 altform / contrast 变体)
        for s in target_sizes:
            p = os.path.join(base, "Square44x44Logo.targetsize-%d" % s)
            save(app_tile(s), p + ".png")
            save(monochrome_tile(s, (0, 0, 0, 255)), p + "_contrast-black.png")
            save(monochrome_tile(s, (255, 255, 255, 255)), p + "_contrast-white.png")
            save(mark_only(s, MARK_BLUE + (255,)), p + "_altform-unplated.png")
            save(mark_only(s, MARK_BLUE_LIGHT + (255,)), p + "_altform-lightunplated.png")
            for suffix, color in (
                ("_contrast-black", (0, 0, 0, 255)),
                ("_contrast-white", (255, 255, 255, 255)),
            ):
                save(mark_only(s, color), p + "_altform-unplated" + suffix + ".png")
                save(mark_only(s, color), p + "_altform-lightunplated" + suffix + ".png")

        # 磁贴 / 徽标:scale 系列
        for name, b in tile_bases.items():
            for sc in scales:
                px = int(round(b * sc / 100))
                p = os.path.join(base, "%s.scale-%d" % (name, sc))
                if name == "LargeTile":
                    save(app_tile(px, fill_ratio=0.72), p + ".png")
                elif name == "StoreLogo":
                    save(app_tile(px), p + ".png")
                else:
                    save(app_tile(px), p + ".png")
                save(monochrome_tile(px, (0, 0, 0, 255)), p + "_contrast-black.png")
                save(monochrome_tile(px, (255, 255, 255, 255)), p + "_contrast-white.png")

        # Wide310x150Logo:左侧居中标志
        for sc in scales:
            w = int(round(310 * sc / 100))
            h = int(round(150 * sc / 100))
            canvas = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            tile = app_tile(int(h * 0.80))
            canvas.paste(tile, (int(w * 0.5 - h * 0.40), int(h * 0.10)), tile)
            p = os.path.join(base, "Wide310x150Logo.scale-%d" % sc)
            save(canvas, p + ".png")
            mono = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            sq = monochrome_tile(int(h * 0.80), (0, 0, 0, 255))
            mono.paste(sq, (int(w * 0.5 - h * 0.40), int(h * 0.10)), sq)
            save(mono, p + "_contrast-black.png")
            sqw = monochrome_tile(int(h * 0.80), (255, 255, 255, 255))
            monow = Image.new("RGBA", (w, h), (0, 0, 0, 0))
            monow.paste(sqw, (int(w * 0.5 - h * 0.40), int(h * 0.10)), sqw)
            save(monow, p + "_contrast-white.png")

    print("CuinZip assets generated in", assets)


if __name__ == "__main__":
    main()
