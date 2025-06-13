#include "text_layout_helper.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/ports/SkFontMgr_data.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Decorations.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "variant.h"
#include <cstddef>

namespace {
// Helper functions to convert between our enums and Skia enums
skia::textlayout::TextAlign to_skia_text_align(camellia::text_layout_helper::paragraph_style::text_align align) {
    switch (align) {
    case camellia::text_layout_helper::paragraph_style::text_align::LEFT:
        return skia::textlayout::TextAlign::kLeft;
    case camellia::text_layout_helper::paragraph_style::text_align::RIGHT:
        return skia::textlayout::TextAlign::kRight;
    case camellia::text_layout_helper::paragraph_style::text_align::CENTER:
        return skia::textlayout::TextAlign::kCenter;
    case camellia::text_layout_helper::paragraph_style::text_align::JUSTIFY:
        return skia::textlayout::TextAlign::kJustify;
    case camellia::text_layout_helper::paragraph_style::text_align::START:
        return skia::textlayout::TextAlign::kStart;
    case camellia::text_layout_helper::paragraph_style::text_align::END:
        return skia::textlayout::TextAlign::kEnd;
    default:
        return skia::textlayout::TextAlign::kLeft;
    }
}

camellia::text_layout_helper::paragraph_style::text_align from_skia_text_align(skia::textlayout::TextAlign align) {
    switch (align) {
    case skia::textlayout::TextAlign::kLeft:
        return camellia::text_layout_helper::paragraph_style::text_align::LEFT;
    case skia::textlayout::TextAlign::kRight:
        return camellia::text_layout_helper::paragraph_style::text_align::RIGHT;
    case skia::textlayout::TextAlign::kCenter:
        return camellia::text_layout_helper::paragraph_style::text_align::CENTER;
    case skia::textlayout::TextAlign::kJustify:
        return camellia::text_layout_helper::paragraph_style::text_align::JUSTIFY;
    case skia::textlayout::TextAlign::kStart:
        return camellia::text_layout_helper::paragraph_style::text_align::START;
    case skia::textlayout::TextAlign::kEnd:
        return camellia::text_layout_helper::paragraph_style::text_align::END;
    default:
        return camellia::text_layout_helper::paragraph_style::text_align::LEFT;
    }
}

skia::textlayout::TextDirection to_skia_text_direction(camellia::text_layout_helper::paragraph_style::text_direction direction) {
    switch (direction) {
    case camellia::text_layout_helper::paragraph_style::text_direction::LTR:
        return skia::textlayout::TextDirection::kLtr;
    case camellia::text_layout_helper::paragraph_style::text_direction::RTL:
        return skia::textlayout::TextDirection::kRtl;
    default:
        return skia::textlayout::TextDirection::kLtr;
    }
}

camellia::text_layout_helper::paragraph_style::text_direction from_skia_text_direction(skia::textlayout::TextDirection direction) {
    switch (direction) {
    case skia::textlayout::TextDirection::kLtr:
        return camellia::text_layout_helper::paragraph_style::text_direction::LTR;
    case skia::textlayout::TextDirection::kRtl:
        return camellia::text_layout_helper::paragraph_style::text_direction::RTL;
    default:
        return camellia::text_layout_helper::paragraph_style::text_direction::LTR;
    }
}

SkFontStyle::Weight to_skia_font_weight(camellia::text_layout_helper::text_style::font_weight weight) {
    return static_cast<SkFontStyle::Weight>(static_cast<int>(weight));
}

SkFontStyle::Slant to_skia_font_slant(camellia::text_layout_helper::text_style::font_style style) {
    switch (style) {
    case camellia::text_layout_helper::text_style::font_style::NORMAL:
        return SkFontStyle::kUpright_Slant;
    case camellia::text_layout_helper::text_style::font_style::ITALIC:
        return SkFontStyle::kItalic_Slant;
    default:
        return SkFontStyle::kUpright_Slant;
    }
}

camellia::text_layout_helper::text_style::font_style from_skia_font_slant(SkFontStyle::Slant slant) {
    switch (slant) {
    case SkFontStyle::kUpright_Slant:
        return camellia::text_layout_helper::text_style::font_style::NORMAL;
    case SkFontStyle::kItalic_Slant:
    case SkFontStyle::kOblique_Slant:
        return camellia::text_layout_helper::text_style::font_style::ITALIC;
    default:
        return camellia::text_layout_helper::text_style::font_style::NORMAL;
    }
}

skia::textlayout::TextDecoration to_skia_text_decoration(camellia::text_layout_helper::text_style::text_decoration decoration) {
    skia::textlayout::TextDecoration result = skia::textlayout::TextDecoration::kNoDecoration;
    int decor_int = static_cast<int>(decoration);

    if (decor_int & static_cast<int>(camellia::text_layout_helper::text_style::text_decoration::UNDERLINE)) {
        result = static_cast<skia::textlayout::TextDecoration>(static_cast<int>(result) | static_cast<int>(skia::textlayout::TextDecoration::kUnderline));
    }
    if (decor_int & static_cast<int>(camellia::text_layout_helper::text_style::text_decoration::OVERLINE)) {
        result = static_cast<skia::textlayout::TextDecoration>(static_cast<int>(result) | static_cast<int>(skia::textlayout::TextDecoration::kOverline));
    }
    if (decor_int & static_cast<int>(camellia::text_layout_helper::text_style::text_decoration::LINE_THROUGH)) {
        result = static_cast<skia::textlayout::TextDecoration>(static_cast<int>(result) | static_cast<int>(skia::textlayout::TextDecoration::kLineThrough));
    }

    return result;
}

skia::textlayout::TextDecorationStyle to_skia_decoration_style(camellia::text_layout_helper::text_style::decoration_style style) {
    switch (style) {
    case camellia::text_layout_helper::text_style::decoration_style::SOLID:
        return skia::textlayout::TextDecorationStyle::kSolid;
    case camellia::text_layout_helper::text_style::decoration_style::DOUBLE:
        return skia::textlayout::TextDecorationStyle::kDouble;
    case camellia::text_layout_helper::text_style::decoration_style::DOTTED:
        return skia::textlayout::TextDecorationStyle::kDotted;
    case camellia::text_layout_helper::text_style::decoration_style::DASHED:
        return skia::textlayout::TextDecorationStyle::kDashed;
    case camellia::text_layout_helper::text_style::decoration_style::WAVY:
        return skia::textlayout::TextDecorationStyle::kWavy;
    default:
        return skia::textlayout::TextDecorationStyle::kSolid;
    }
}

camellia::text_layout_helper::text_style::decoration_style from_skia_decoration_style(skia::textlayout::TextDecorationStyle style) {
    switch (style) {
    case skia::textlayout::TextDecorationStyle::kSolid:
        return camellia::text_layout_helper::text_style::decoration_style::SOLID;
    case skia::textlayout::TextDecorationStyle::kDouble:
        return camellia::text_layout_helper::text_style::decoration_style::DOUBLE;
    case skia::textlayout::TextDecorationStyle::kDotted:
        return camellia::text_layout_helper::text_style::decoration_style::DOTTED;
    case skia::textlayout::TextDecorationStyle::kDashed:
        return camellia::text_layout_helper::text_style::decoration_style::DASHED;
    case skia::textlayout::TextDecorationStyle::kWavy:
        return camellia::text_layout_helper::text_style::decoration_style::WAVY;
    default:
        return camellia::text_layout_helper::text_style::decoration_style::SOLID;
    }
}

SkFont::Edging to_skia_edging(camellia::text_layout_helper::font::edging edge) {
    switch (edge) {
    case camellia::text_layout_helper::font::edging::ALIAS:
        return SkFont::Edging::kAlias;
    case camellia::text_layout_helper::font::edging::ANTI_ALIAS:
        return SkFont::Edging::kAntiAlias;
    case camellia::text_layout_helper::font::edging::SUBPIXEL_ANTI_ALIAS:
        return SkFont::Edging::kSubpixelAntiAlias;
    default:
        return SkFont::Edging::kAntiAlias;
    }
}

camellia::text_layout_helper::font::edging from_skia_edging(SkFont::Edging edge) {
    switch (edge) {
    case SkFont::Edging::kAlias:
        return camellia::text_layout_helper::font::edging::ALIAS;
    case SkFont::Edging::kAntiAlias:
        return camellia::text_layout_helper::font::edging::ANTI_ALIAS;
    case SkFont::Edging::kSubpixelAntiAlias:
        return camellia::text_layout_helper::font::edging::SUBPIXEL_ANTI_ALIAS;
    default:
        return camellia::text_layout_helper::font::edging::ANTI_ALIAS;
    }
}
} // namespace

namespace camellia::text_layout_helper {
// paragraph_style implementation
paragraph_style::paragraph_style(skia::textlayout::ParagraphStyle paragraph_style) : _paragraph_style(std::move(paragraph_style)) {}

paragraph_style &paragraph_style::operator=(skia::textlayout::ParagraphStyle paragraph_style) {
    _paragraph_style = std::move(paragraph_style);
    return *this;
}

void paragraph_style::set_text_align(text_align align) { _paragraph_style.setTextAlign(to_skia_text_align(align)); }

paragraph_style::text_align paragraph_style::get_text_align() const { return from_skia_text_align(_paragraph_style.getTextAlign()); }

void paragraph_style::set_text_direction(text_direction direction) { _paragraph_style.setTextDirection(to_skia_text_direction(direction)); }

paragraph_style::text_direction paragraph_style::get_text_direction() const { return from_skia_text_direction(_paragraph_style.getTextDirection()); }

void paragraph_style::set_line_height(number_t height) { _paragraph_style.setHeight(height); }

number_t paragraph_style::get_line_height() const { return _paragraph_style.getHeight(); }

void paragraph_style::set_max_lines(integer_t max_lines) { _paragraph_style.setMaxLines(max_lines); }

integer_t paragraph_style::get_max_lines() const { return static_cast<integer_t>(_paragraph_style.getMaxLines()); }

void paragraph_style::set_ellipsis(const text_t &ellipsis) { _paragraph_style.setEllipsis(SkString(ellipsis.c_str())); }

text_t paragraph_style::get_ellipsis() const { return text_t(_paragraph_style.getEllipsis().c_str()); }

void paragraph_style::set_text_overflow(text_overflow overflow) {
    // Note: Skia doesn't have a direct text overflow enum,
    // so we map this to ellipsis behavior
    switch (overflow) {
    case text_overflow::ELLIPSIS:
        if (get_ellipsis().empty()) {
            set_ellipsis("...");
        }
        break;
    case text_overflow::CLIP:
    case text_overflow::FADE:
        set_ellipsis("");
        break;
    }
}

paragraph_style::text_overflow paragraph_style::get_text_overflow() const {
    // Simple heuristic: if ellipsis is set, assume ELLIPSIS overflow
    return get_ellipsis().empty() ? text_overflow::CLIP : text_overflow::ELLIPSIS;
}

// text_style implementation
void text_style::set_font_size(number_t size) { _text_style.setFontSize(size); }

number_t text_style::get_font_size() const { return _text_style.getFontSize(); }

void text_style::set_font_weight(font_weight weight) {
    auto current_style = _text_style.getFontStyle();
    SkFontStyle new_style(to_skia_font_weight(weight), current_style.width(), current_style.slant());
    _text_style.setFontStyle(new_style);
}

text_style::font_weight text_style::get_font_weight() const { return static_cast<text_style::font_weight>(_text_style.getFontStyle().weight()); }

void text_style::set_font_style(font_style style) {
    auto current_style = _text_style.getFontStyle();
    SkFontStyle new_style(current_style.weight(), current_style.width(), to_skia_font_slant(style));
    _text_style.setFontStyle(new_style);
}

text_style::font_style text_style::get_font_style() const { return from_skia_font_slant(_text_style.getFontStyle().slant()); }

void text_style::set_font_family(const text_t &family) {
    std::vector<SkString> families = {SkString(family.c_str())};
    _text_style.setFontFamilies(families);
}

text_t text_style::get_font_family() const {
    auto families = _text_style.getFontFamilies();
    if (!families.empty()) {
        return text_t(families[0].c_str());
    }
    return "";
}

void text_style::set_color(integer_t color) { _text_style.setColor(static_cast<SkColor>(color)); }

integer_t text_style::get_color() const { return static_cast<integer_t>(_text_style.getColor()); }

void text_style::set_background_color(integer_t color) {
    SkPaint paint;
    paint.setColor(static_cast<SkColor>(color));
    _text_style.setBackgroundColor(paint);
}

integer_t text_style::get_background_color() const {
    // Note: This is a simplified approach since Skia stores background as a paint
    if (_text_style.hasBackground()) {
        return static_cast<integer_t>(_text_style.getBackground().getColor());
    }
    return 0x00000000; // Transparent
}

void text_style::set_decoration(text_decoration decoration) { _text_style.setDecoration(static_cast<skia::textlayout::TextDecoration>(decoration)); }

text_style::text_decoration text_style::get_decoration() const { return static_cast<text_style::text_decoration>(_text_style.getDecoration().fType); }

void text_style::set_decoration_color(integer_t color) { _text_style.setDecorationColor(static_cast<SkColor>(color)); }

integer_t text_style::get_decoration_color() const { return static_cast<integer_t>(_text_style.getDecorationColor()); }

void text_style::set_decoration_style(decoration_style style) { _text_style.setDecorationStyle(to_skia_decoration_style(style)); }

text_style::decoration_style text_style::get_decoration_style() const { return from_skia_decoration_style(_text_style.getDecorationStyle()); }

void text_style::set_decoration_thickness(number_t thickness) { _text_style.setDecorationThicknessMultiplier(thickness); }

number_t text_style::get_decoration_thickness() const { return _text_style.getDecorationThicknessMultiplier(); }

void text_style::set_letter_spacing(number_t spacing) { _text_style.setLetterSpacing(spacing); }

number_t text_style::get_letter_spacing() const { return _text_style.getLetterSpacing(); }

void text_style::set_word_spacing(number_t spacing) { _text_style.setWordSpacing(spacing); }

number_t text_style::get_word_spacing() const { return _text_style.getWordSpacing(); }

text_style::text_style(skia::textlayout::TextStyle text_style) : _text_style(std::move(text_style)) {}

text_style &text_style::operator=(skia::textlayout::TextStyle text_style) {
    _text_style = std::move(text_style);
    return *this;
}
text_paragraph::text_paragraph(std::unique_ptr<skia::textlayout::Paragraph> p_paragraph) : _p_paragraph(std::move(p_paragraph)) {}

void text_paragraph::layout(number_t width) { _p_paragraph->layout(width); }

void text_paragraph::visit_foreground(visit_foreground_cb callback) {
    auto *impl = static_cast<skia::textlayout::ParagraphImpl *>(_p_paragraph.get());

    int lineNumber = 0;
    for (auto &line : impl->lines()) {
        line.iterateThroughVisualRuns(false, [&](const skia::textlayout::Run *run, SkScalar runOffsetInLine, skia::textlayout::TextRange textRange,
                                                 SkScalar *runWidthInLine) {
            *runWidthInLine = line.iterateThroughSingleRunByStyles(
                skia::textlayout::TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, skia::textlayout::StyleType::kForeground,
                [&](skia::textlayout::TextRange textRange, const skia::textlayout::TextStyle &style, const skia::textlayout::TextLine::ClipContext &context) {
                    SkScalar correctedBaseline = SkScalarFloorToScalar(line.baseline() + style.getBaselineShift() + 0.5);

                    foreground_visit_info info;
                    info._line_number = lineNumber;
                    info._start = textRange.start;
                    info._end = textRange.end;
                    info._offset_x = line.offset().fX + context.fTextShift;
                    info._offset_y = line.offset().fY + correctedBaseline;
                    info._glyphs = &run->glyphs()[context.pos];
                    info._positions = &run->positions()[context.pos];
                    info._glyph_count = context.size;
                    info._font = font(run->font());
                    info._style = text_style(style);
                    callback(info);
                });
            return true;
        });
        lineNumber += 1;
    }
}

void text_paragraph::visit_background(visit_background_cb callback) {
    auto *impl = static_cast<skia::textlayout::ParagraphImpl *>(_p_paragraph.get());

    int lineNumber = 0;
    for (auto &line : impl->lines()) {
        line.iterateThroughVisualRuns(false, [&](const skia::textlayout::Run *run, SkScalar runOffsetInLine, skia::textlayout::TextRange textRange,
                                                 SkScalar *runWidthInLine) {
            *runWidthInLine = line.iterateThroughSingleRunByStyles(
                skia::textlayout::TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, skia::textlayout::StyleType::kForeground,
                [&](skia::textlayout::TextRange textRange, const skia::textlayout::TextStyle &style, const skia::textlayout::TextLine::ClipContext &context) {
                    if (style.hasBackground()) {
                        shape_visit_info info;
                        info._line_number = lineNumber;
                        info._start = textRange.start;
                        info._end = textRange.end;

                        info._painter.drawRect(context.clip.makeOffset(line.offset()), style.getBackgroundPaintOrID());

                        callback(info);
                    }
                });
            return true;
        });
        lineNumber += 1;
    }
}

void text_paragraph::visit_decoration(visit_decoration_cb callback) {
    auto *impl = static_cast<skia::textlayout::ParagraphImpl *>(_p_paragraph.get());

    int lineNumber = 0;
    for (auto &line : impl->lines()) {
        line.iterateThroughVisualRuns(false, [&](const skia::textlayout::Run *run, SkScalar runOffsetInLine, skia::textlayout::TextRange textRange,
                                                 SkScalar *runWidthInLine) {
            *runWidthInLine = line.iterateThroughSingleRunByStyles(
                skia::textlayout::TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, skia::textlayout::StyleType::kForeground,
                [&](skia::textlayout::TextRange textRange, const skia::textlayout::TextStyle &style, const skia::textlayout::TextLine::ClipContext &context) {
                    shape_visit_info info;
                    info._line_number = lineNumber;
                    info._start = textRange.start;
                    info._end = textRange.end;

                    info._painter.translate(line.offset().fX, line.offset().fY + style.getBaselineShift());
                    skia::textlayout::Decorations decorations;
                    SkScalar correctedBaseline = SkScalarFloorToScalar(-line.sizes().rawAscent() + style.getBaselineShift() + 0.5);
                    decorations.paint(&info._painter, style, context, correctedBaseline);

                    callback(info);
                });
            return true;
        });
        lineNumber += 1;
    }
}

text_layout_engine::text_layout_engine(const std::vector<bytes_t> &font_data_vector) : _p_font_collection(sk_make_sp<skia::textlayout::FontCollection>()) {
    _font_data_vector.reserve(font_data_vector.size());
    for (const auto &font_data : font_data_vector) {
        _font_data_vector.push_back(SkData::MakeWithCopy(font_data.data(), font_data.size()));
    }
    _p_font_mgr = SkFontMgr_New_Custom_Data(SkSpan<sk_sp<SkData>>(_font_data_vector.data(), _font_data_vector.size()));
    _p_font_collection->setDefaultFontManager(_p_font_mgr);
}

text_paragraph text_layout_engine::create_paragraph(const text_t &text, const paragraph_style &paragraph_style) {
    auto builder = skia::textlayout::ParagraphBuilder::make(paragraph_style._paragraph_style, _p_font_collection);
    builder->addText(text.c_str(), text.size());
    return text_paragraph(builder->Build());
}

text_paragraph text_layout_engine::create_paragraph(const std::vector<text_segment> &segments, const paragraph_style &paragraph_style) {
    auto builder = skia::textlayout::ParagraphBuilder::make(paragraph_style._paragraph_style, _p_font_collection);

    for (const auto &segment : segments) {
        builder->pushStyle(segment.style._text_style);
        builder->addText(segment.text.c_str(), segment.text.size());
        builder->pop();
    }

    return text_paragraph(builder->Build());
}

font text_layout_engine::create_font() const {
    SkFont sk_font;
    return font(sk_font);
}

font text_layout_engine::create_font(number_t size) const {
    SkFont sk_font;
    sk_font.setSize(size);
    return font(sk_font);
}

void interceptor_painter::drawRect(const SkRect &rect, const SkPaintOrID &paint) {
    auto info = std::make_unique<rect_deco_info>();
    info->xywh = vector4(_offset_x + rect.x(), _offset_y + rect.y(), rect.width(), rect.height());
    info->color = static_cast<integer_t>(std::get<SkPaint>(paint).getColor());
    info->filled = false;
    _shapes.push_back(std::move(info));
}

void interceptor_painter::drawFilledRect(const SkRect &rect, const DecorationStyle &decorStyle) {
    auto info = std::make_unique<rect_deco_info>();
    info->xywh = vector4(_offset_x + rect.x(), _offset_y + rect.y(), rect.width(), rect.height());
    info->color = static_cast<integer_t>(decorStyle.getColor());
    info->filled = true;
    _shapes.push_back(std::move(info));
}

void interceptor_painter::drawPath(const SkPath &path, const DecorationStyle &decorStyle) {
    auto info = std::make_unique<path_deco_info>();
    // info->path = path;
    info->color = static_cast<integer_t>(decorStyle.getColor());
    _shapes.push_back(std::move(info));
}

void interceptor_painter::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const DecorationStyle &decorStyle) {
    auto info = std::make_unique<line_deco_info>();
    info->start = vector2(_offset_x + x0, _offset_y + y0);
    info->end = vector2(_offset_x + x1, _offset_y + y1);
    info->color = static_cast<integer_t>(decorStyle.getColor());
    _shapes.push_back(std::move(info));
}

// font implementation
font::font(const SkFont &sk_font) : _font(sk_font) {}

font &font::operator=(const SkFont &sk_font) {
    _font = sk_font;
    return *this;
}

const SkFont *font::get_sk_font() const { return &_font; }

void font::set_size(number_t size) { _font.setSize(size); }

number_t font::get_size() const { return _font.getSize(); }

void font::set_scale_x(number_t scale) { _font.setScaleX(scale); }

number_t font::get_scale_x() const { return _font.getScaleX(); }

void font::set_skew_x(number_t skew) { _font.setSkewX(skew); }

number_t font::get_skew_x() const { return _font.getSkewX(); }

void font::set_spacing(number_t spacing) {
    // Note: SkFont doesn't have direct spacing methods
    // This would typically be handled by the layout engine
}

number_t font::get_spacing() const {
    // Note: SkFont doesn't have direct spacing methods
    return 0.0f;
}

void font::set_embolden(boolean_t embolden) { _font.setEmbolden(embolden); }

boolean_t font::get_embolden() const { return _font.isEmbolden(); }

void font::set_baseline_snap(boolean_t snap) { _font.setBaselineSnap(snap); }

boolean_t font::get_baseline_snap() const { return _font.isBaselineSnap(); }

void font::set_linear_metrics(boolean_t linear) { _font.setLinearMetrics(linear); }

boolean_t font::get_linear_metrics() const { return _font.isLinearMetrics(); }

void font::set_subpixel(boolean_t subpixel) { _font.setSubpixel(subpixel); }

boolean_t font::get_subpixel() const { return _font.isSubpixel(); }

void font::set_embedded_bitmaps(boolean_t embedded) { _font.setEmbeddedBitmaps(embedded); }

boolean_t font::get_embedded_bitmaps() const { return _font.isEmbeddedBitmaps(); }

void font::set_force_auto_hinting(boolean_t force) { _font.setForceAutoHinting(force); }

boolean_t font::get_force_auto_hinting() const { return _font.isForceAutoHinting(); }

void font::set_hinting(hinting hint) {
    // Simplified hinting implementation using basic SkFont methods
    switch (hint) {
    case hinting::NONE:
        _font.setHinting(SkFontHinting::kNone);
        break;
    case hinting::SLIGHT:
        _font.setHinting(SkFontHinting::kSlight);
        break;
    case hinting::NORMAL:
        _font.setHinting(SkFontHinting::kNormal);
        break;
    case hinting::FULL:
        _font.setHinting(SkFontHinting::kFull);
        break;
    }
}

font::hinting font::get_hinting() const {
    switch (_font.getHinting()) {
    case SkFontHinting::kNone:
        return hinting::NONE;
    case SkFontHinting::kSlight:
        return hinting::SLIGHT;
    case SkFontHinting::kNormal:
        return hinting::NORMAL;
    case SkFontHinting::kFull:
        return hinting::FULL;
    default:
        return hinting::NORMAL;
    }
}

void font::set_edging(edging edge) { _font.setEdging(to_skia_edging(edge)); }

font::edging font::get_edging() const { return from_skia_edging(_font.getEdging()); }
} // namespace camellia::text_layout_helper