#!/usr/bin/env python3

# SPDX-License-Identifier: Apache-2.0
# Copyright 2023-2024 Jussi Pakkanen

import pathlib, os, sys, json

os.environ['CAPYPDF_SO_OVERRIDE'] = 'build/src'
source_root = pathlib.Path(__file__).parent / '..'
sys.path.append(str(source_root / 'python'))

try:
    import capypdf
except Exception:
    print('You might need to edit the search paths at the top of this file to get it to find the dependencies.')
    raise

def cm2pt(pts):
    return pts*28.346;


class PrinterTest:
    def __init__(self, ofilename):
        self.ofilename = ofilename
        self.fontfile = '/usr/share/fonts/truetype/noto/NotoSerif-Regular.ttf'
        self.titlesize = 20
        self.textsize = 10
        opts = capypdf.DocumentMetadata()
        props = capypdf.PageProperties()
        self.w = 595.0
        self.h = 841.89
        props.set_pagebox(capypdf.PageBox.Media, 0, 0, self.w, self.h)
        opts.set_title('Printer test document')
        opts.set_author('CapyPDF developers')
        opts.set_default_page_properties(props)
        self.pdfgen = capypdf.Generator(self.ofilename, opts)
        self.basefont = self.pdfgen.load_font(self.fontfile)

    def render_centered(self, ctx, text, font, pointsize, x, y):
        text_w = self.pdfgen.text_width(text, font, pointsize)
        ctx.render_text(text, font, pointsize, x -text_w/2, y)

    def create(self):
        with self.pdfgen.page_draw_context() as ctx:
            self.render_centered(ctx, "CapyPDF printer test page", self.basefont, 20, self.w/2, 800)
            with ctx.push_gstate():
                ctx.translate(10, 700)
                self.draw_grays(ctx)
            with ctx.push_gstate():
                ctx.translate(10, 600)
                self.draw_richblacks(ctx)

    def draw_unit_circle(self, ctx):
        control = 0.5523 / 2;
        ctx.cmd_m(0, 0.5);
        ctx.cmd_c(control, 0.5, 0.5, control, 0.5, 0);
        ctx.cmd_c(0.5, -control, control, -0.5, 0, -0.5);
        ctx.cmd_c(-control, -0.5, -0.5, -control, -0.5, 0);
        ctx.cmd_c(-0.5, control, -control, 0.5, 0, 0.5);
        ctx.cmd_f()

    def draw_circle(self, ctx, scale, offx, offy):
        with ctx.push_gstate():
            ctx.translate(offx, offy)
            ctx.scale(scale, scale)
            self.draw_unit_circle(ctx)

    def draw_grays(self, ctx):
        ctx.cmd_g(0.5)
        deltax = 100
        self.draw_gray_and_text(ctx, 50, 80, 50, "G gray")
        ctx.cmd_k(0.5, 0.5, 0.5, 0)
        self.draw_gray_and_text(ctx, 50, 80 + deltax, 50, "CMY gray")
        ctx.cmd_k(0.5, 0.5, 0.5, 0.5)
        self.draw_gray_and_text(ctx, 50, 80 + 2*deltax, 50, "CMYK gray")
        ctx.cmd_k(0.0, 0.0, 0.0, 0.5)
        self.draw_gray_and_text(ctx, 50, 80 + 3*deltax, 50, "K gray")
        ctx.cmd_rg(0.5, 0.5, 0.5)
        self.draw_gray_and_text(ctx, 50, 80 + 4*deltax, 50, "RGB gray")

    def draw_richblacks(self, ctx):
        ctx.cmd_k(0.1, 0, 0, 1.0)
        deltax = 100
        self.draw_gray_and_text(ctx, 50, 80, 50, "C rich black")
        ctx.cmd_k(0.0, 0.1, 0.0, 1.0)
        self.draw_gray_and_text(ctx, 50, 80 + deltax, 50, "M rich black")
        ctx.cmd_k(0.0, 0.0, 0.1, 1.0)
        self.draw_gray_and_text(ctx, 50, 80 + 2*deltax, 50, "Y rich black")
        ctx.cmd_k(0.1, 0.1, 0.1, 1.0)
        self.draw_gray_and_text(ctx, 50, 80 + 3*deltax, 50, "All rich black")

    def draw_gray_and_text(self, ctx, s, transx, transy, text):
        self.draw_circle(ctx, s, transx, transy)
        ctx.cmd_g(0)
        self.render_centered(ctx, text, self.basefont, 10, transx, transy - 40)

    def finish(self):
        self.pdfgen.write()

if __name__ == '__main__':
    p = PrinterTest(    'capy_printer_test.pdf')
    p.create()
    p.finish()
