// SPDX-License-Identifier: Apache-2.0
// Copyright 2023-2024 Jussi Pakkanen

#pragma once

#include <drawcontext.hpp>

#include <cstdio>
#include <cstdint>

#include <vector>
#include <string_view>
#include <optional>
#include <filesystem>

namespace capypdf::internal {

class PdfGen;

struct DrawContextPopper {
    PdfGen *g;
    PdfDrawContext ctx;

    explicit DrawContextPopper(PdfGen *g,
                               PdfDocument *doc,
                               PdfColorConverter *cm,
                               CapyPDF_Draw_Context_Type dtype,
                               const PdfRectangle &rect)
        : g{g}, ctx{doc, cm, dtype, rect} {}

    DrawContextPopper() = delete;
    DrawContextPopper(const DrawContextPopper &) = delete;

    ~DrawContextPopper();
};

class PdfGen {
public:
    static rvoe<std::unique_ptr<PdfGen>> construct(const std::filesystem::path &ofname,
                                                   const DocumentMetadata &d);
    PdfGen(PdfGen &&o) = default;
    ~PdfGen();

    rvoe<NoReturnValue> write();

    rvoe<RasterImage> load_image(const std::filesystem::path &fname);
    rvoe<CapyPDF_ImageId> embed_jpg(const std::filesystem::path &fname,
                                    const ImagePDFProperties &props);
    rvoe<CapyPDF_EmbeddedFileId> embed_file(const std::filesystem::path &fname) {
        return pdoc.embed_file(fname);
    }
    rvoe<CapyPDF_FontId> load_font(const std::filesystem::path &fname) {
        return pdoc.load_font(ft.get(), fname);
    };

    rvoe<RasterImage> convert_image_to_cs(RasterImage image,
                                          CapyPDF_DeviceColorspace cs,
                                          CapyPDF_Rendering_Intent ri) const;
    rvoe<CapyPDF_ImageId> add_image(RasterImage ri, const ImagePDFProperties &params);

    ImageSize get_image_info(CapyPDF_ImageId img_id) { return pdoc.get(img_id).s; }

    rvoe<CapyPDF_SeparationId> create_separation(const asciistring &name,
                                                 CapyPDF_DeviceColorspace cs,
                                                 CapyPDF_FunctionId fid) {
        return pdoc.create_separation(name, cs, fid);
    }

    rvoe<CapyPDF_GraphicsStateId> add_graphics_state(const GraphicsState &state) {
        return pdoc.add_graphics_state(state);
    }

    rvoe<CapyPDF_FunctionId> add_function(const PdfFunction &func) {
        return pdoc.add_function(func);
    }
    rvoe<CapyPDF_FunctionId> add_function(const FunctionType3 &func) {
        return pdoc.add_function(func);
    }

    rvoe<CapyPDF_ShadingId> add_shading(const PdfShading &shade) { return pdoc.add_shading(shade); }

    rvoe<CapyPDF_LabColorSpaceId> add_lab_colorspace(const LabColorSpace &lab) {
        return pdoc.add_lab_colorspace(lab);
    }

    rvoe<CapyPDF_IccColorSpaceId> load_icc_file(const std::filesystem::path &fname) {
        return pdoc.load_icc_file(fname);
    }
    rvoe<CapyPDF_IccColorSpaceId> add_icc_profile(const std::string_view &bytes,
                                                  uint32_t num_channels) {
        return pdoc.add_icc_profile(bytes, num_channels);
    }

    rvoe<CapyPDF_FormWidgetId> create_form_checkbox(PdfBox loc,
                                                    CapyPDF_FormXObjectId onstate,
                                                    CapyPDF_FormXObjectId offstate,
                                                    std::string_view partial_name) {
        return pdoc.create_form_checkbox(loc, onstate, offstate, partial_name);
    }

    rvoe<CapyPDF_AnnotationId> create_annotation(const Annotation &a) {
        return pdoc.create_annotation(a);
    }

    DrawContextPopper guarded_page_context();
    PdfDrawContext *new_page_draw_context();

    DrawContextPopper guarded_form_xobject(const PdfRectangle &rect) {
        return DrawContextPopper(this, &this->pdoc, &pdoc.cm, CAPY_DC_FORM_XOBJECT, rect);
    }
    PdfDrawContext *new_form_xobject(const PdfRectangle &rect) {
        return new PdfDrawContext(&this->pdoc, &pdoc.cm, CAPY_DC_FORM_XOBJECT, rect);
    }

    PdfDrawContext *new_transparency_group(const PdfRectangle &bbox) {
        return new PdfDrawContext(&this->pdoc, &pdoc.cm, CAPY_DC_TRANSPARENCY_GROUP, bbox);
    }

    PdfDrawContext new_color_pattern_builder(const PdfRectangle &rect);
    PdfDrawContext *new_color_pattern(const PdfRectangle &rect);

    rvoe<PageId> add_page(PdfDrawContext &ctx);
    rvoe<CapyPDF_FormXObjectId> add_form_xobject(PdfDrawContext &ctx);
    rvoe<CapyPDF_PatternId> add_shading_pattern(const ShadingPattern &shp);
    rvoe<CapyPDF_PatternId> add_tiling_pattern(PdfDrawContext &cp);
    rvoe<CapyPDF_TransparencyGroupId> add_transparency_group(PdfDrawContext &ctx) {
        return pdoc.add_transparency_group(ctx);
    }

    rvoe<CapyPDF_OutlineId> add_outline(const Outline &o) { return pdoc.add_outline(o); }

    rvoe<CapyPDF_StructureItemId> add_structure_item(const CapyPDF_StructureType stype,
                                                     std::optional<CapyPDF_StructureItemId> parent,
                                                     std::optional<StructItemExtraData> extra) {
        return pdoc.add_structure_item(stype, parent, std::move(extra));
    }

    rvoe<CapyPDF_StructureItemId> add_structure_item(const CapyPDF_RoleId role,
                                                     std::optional<CapyPDF_StructureItemId> parent,
                                                     std::optional<StructItemExtraData> extra) {
        return pdoc.add_structure_item(role, parent, std::move(extra));
    }

    rvoe<CapyPDF_OptionalContentGroupId> add_optional_content_group(const OptionalContentGroup &g) {
        return pdoc.add_optional_content_group(g);
    }

    int32_t num_pages() const { return (int32_t)pdoc.pages.size(); }

    std::optional<double>
    glyph_advance(CapyPDF_FontId fid, double pointsize, uint32_t codepoint) const {
        return pdoc.glyph_advance(fid, pointsize, codepoint);
    }

    rvoe<double> utf8_text_width(const u8string &txt, CapyPDF_FontId fid, double pointsize) const;

    rvoe<CapyPDF_RoleId> add_rolemap_entry(std::string name, CapyPDF_StructureType builtin_type) {
        return pdoc.add_rolemap_entry(std::move(name), builtin_type);
    }

private:
    PdfGen(std::filesystem::path ofilename,
           std::unique_ptr<FT_LibraryRec_, FT_Error (*)(FT_LibraryRec_ *)> ft,
           PdfDocument pdoc)
        : ofilename(std::move(ofilename)), ft(std::move(ft)), pdoc(std::move(pdoc)) {}

    std::filesystem::path ofilename;
    std::unique_ptr<FT_LibraryRec_, FT_Error (*)(FT_LibraryRec_ *)> ft;
    PdfDocument pdoc;
};

struct GenPopper {
    std::unique_ptr<PdfGen> g;
    GenPopper(const std::filesystem::path &ofname, const DocumentMetadata &d) : g() {
        auto rc = PdfGen::construct(ofname, d);
        if(!rc) {
            fprintf(stderr, "%s\n", error_text(rc.error()));
            std::abort();
        }
        g = std::move(rc.value());
    }
    ~GenPopper() {
        auto rc = g->write();
        if(!rc) {
            fprintf(stderr, "%s\n", error_text(rc.error()));
            std::abort();
        }
    }
};

} // namespace capypdf::internal
