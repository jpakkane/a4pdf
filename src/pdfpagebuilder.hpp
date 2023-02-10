/*
 * Copyright 2022 Jussi Pakkanen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <pdfcommon.hpp>
#include <pdfcolorconverter.hpp>
#include <pdfdocument.hpp>
#include <string>
#include <unordered_set>
#include <vector>
#include <optional>

template<> struct std::hash<A4PDF::FontSubset> {
    size_t operator()(A4PDF::FontSubset const &s) const noexcept {
        const size_t x = (size_t)s.fid.id;
        const size_t y = s.subset_id;
        return (x << 32) + y;
    }
};

namespace A4PDF {

struct GraphicsState {
    std::optional<RenderingIntent> intent;
    std::optional<A4PDF_Blend_Mode> blend_mode;
};

struct GsEntries {
    std::string name;
    GraphicsState state;
};

class PdfPageBuilder {

public:
    PdfPageBuilder(PdfDocument *g, PdfColorConverter *cm);
    ~PdfPageBuilder();
    void finalize();

    PdfPageBuilder() = delete;
    PdfPageBuilder(const PdfPageBuilder &) = delete;
    // All methods that begin with cmd_ map directly to the PDF primitive with the same name.
    void cmd_q(); // Save
    void cmd_Q(); // Restore
    void cmd_re(double x, double y, double w, double h);
    void cmd_f();
    void cmd_S();
    void cmd_s();
    void cmd_h();
    void cmd_B();
    void cmd_Bstar();
    void cmd_m(double x, double y);
    void cmd_l(double x, double y);
    void cmd_w(double w);
    void cmd_c(double x1, double y1, double x2, double y2, double x3, double y3);
    void cmd_cm(double m1, double m2, double m3, double m4, double m5, double m6);

    void cmd_cs(std::string_view cspace_name);
    void cmd_scn(double value);
    void cmd_CS(std::string_view cspace_name);
    void cmd_SCN(double value);

    // Stroke.
    void cmd_RG(double r, double g, double b);
    void cmd_G(double gray);
    void cmd_K(double c, double m, double y, double k);

    // Nonstroke
    void cmd_rg(double r, double g, double b);
    void cmd_g(double gray);
    void cmd_k(double c, double m, double y, double k);

    void cmd_gs(std::string_view gs_name);

    void set_stroke_color(const DeviceRGBColor &c);
    void set_nonstroke_color(const DeviceRGBColor &c);
    void set_nonstroke_color(const DeviceGrayColor &c);
    void set_separation_stroke_color(SeparationId id, LimitDouble value);
    void set_separation_nonstroke_color(SeparationId id, LimitDouble value);
    void set_all_stroke_color();
    void draw_image(ImageId obj_num);
    void scale(double xscale, double yscale);
    void translate(double xtran, double ytran);
    void rotate(double angle);
    void render_utf8_text(std::string_view text, FontId fid, double pointsize, double x, double y);
    void render_raw_glyph(uint32_t glyph, FontId fid, double pointsize, double x, double y);
    void render_ascii_text_builtin(
        const char *ascii_text, A4PDF_Builtin_Fonts font_id, double pointsize, double x, double y);

    void add_graphics_state(std::string_view name, const GraphicsState &gs);

    void draw_unit_circle();
    void draw_unit_box();

    void clear();

private:
    void build_resource_dict();
    void setup_initial_cs();

    PdfDocument *doc;
    PdfColorConverter *cm;
    std::string resources;
    std::string commands;
    std::back_insert_iterator<std::string> cmd_appender;
    std::unordered_set<int32_t> used_images;
    std::unordered_set<FontSubset> used_subset_fonts;
    std::unordered_set<int32_t> used_fonts;
    std::unordered_set<int32_t> used_colorspaces;
    std::vector<GsEntries> gstates;
    bool is_finalized = false;
    bool uses_all_colorspace = false;
};

} // namespace A4PDF
