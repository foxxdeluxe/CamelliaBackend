#include "text_layout_helper.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/ports/SkFontMgr_data.h"
#include "include/private/base/SkPoint_impl.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphBuilder.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Decorations.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "variant.h"
#include <cstddef>
#include <stdexcept>

namespace camellia::text_layout_helper {

// PIMPL Implementation classes
class interceptor_painter_impl : public skia::textlayout::ParagraphPainter {
public:
    void drawRect(const SkRect &rect, const SkPaintOrID &paint) override;
    void drawFilledRect(const SkRect &rect, const DecorationStyle &decorStyle) override;
    void drawPath(const SkPath &path, const DecorationStyle &decorStyle) override;
    void drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const DecorationStyle &decorStyle) override;

    // Not intercepting these functions
    void drawTextShadow(const sk_sp<SkTextBlob> &blob, SkScalar x, SkScalar y, SkColor color, SkScalar blurSigma) override {};
    void drawTextBlob(const sk_sp<SkTextBlob> &blob, SkScalar x, SkScalar y, const SkPaintOrID &paint) override {};
    void save() override {};
    void restore() override {};

    void clipRect(const SkRect &rect) override {};
    void translate(SkScalar dx, SkScalar dy) override {
        _offset_x += dx;
        _offset_y += dy;
    };

    [[nodiscard]] const std::vector<std::unique_ptr<deco_info>> &get_shapes() const { return _shapes; }

private:
    number_t _offset_x{0.0F};
    number_t _offset_y{0.0F};
    std::vector<std::unique_ptr<deco_info>> _shapes;
};

class paragraph_style_impl {
public:
    skia::textlayout::ParagraphStyle _paragraph_style;

    paragraph_style_impl() = default;
    paragraph_style_impl(const paragraph_style_impl &other) = default;
    paragraph_style_impl &operator=(const paragraph_style_impl &other) = default;
    paragraph_style_impl(paragraph_style_impl &&other) noexcept = default;
    paragraph_style_impl &operator=(paragraph_style_impl &&other) noexcept = default;
    ~paragraph_style_impl() = default;

    explicit paragraph_style_impl(skia::textlayout::ParagraphStyle paragraph_style) : _paragraph_style(std::move(paragraph_style)) {}
};

class text_style_impl {
public:
    skia::textlayout::TextStyle _text_style;

    text_style_impl() = default;
    text_style_impl(const text_style_impl &other) = default;
    text_style_impl &operator=(const text_style_impl &other) = default;
    text_style_impl(text_style_impl &&other) noexcept = default;
    text_style_impl &operator=(text_style_impl &&other) noexcept = default;
    ~text_style_impl() = default;

    explicit text_style_impl(skia::textlayout::TextStyle text_style) : _text_style(std::move(text_style)) {}
};

class font_impl {
public:
    SkFont _font;

    font_impl() = default;
    font_impl(const font_impl &other) = default;
    font_impl &operator=(const font_impl &other) = default;
    font_impl(font_impl &&other) noexcept = default;
    font_impl &operator=(font_impl &&other) noexcept = default;
    ~font_impl() = default;

    explicit font_impl(const SkFont &sk_font) : _font(sk_font) {}
};

class text_paragraph_impl {
public:
    std::unique_ptr<skia::textlayout::Paragraph> _p_paragraph;

    text_paragraph_impl() = default;
    text_paragraph_impl(const text_paragraph_impl &other) = delete;
    text_paragraph_impl &operator=(const text_paragraph_impl &other) = delete;
    text_paragraph_impl(text_paragraph_impl &&other) noexcept = default;
    text_paragraph_impl &operator=(text_paragraph_impl &&other) noexcept = default;
    ~text_paragraph_impl() = default;

    explicit text_paragraph_impl(std::unique_ptr<skia::textlayout::Paragraph> p_paragraph) : _p_paragraph(std::move(p_paragraph)) {}
};

class text_layout_engine_impl {
public:
    sk_sp<SkFontMgr> _p_font_mgr;
    std::vector<sk_sp<SkData>> _font_data_vector;
    sk_sp<skia::textlayout::FontCollection> _p_font_collection;

    text_layout_engine_impl() = default;
    text_layout_engine_impl(const text_layout_engine_impl &other) = delete;
    text_layout_engine_impl &operator=(const text_layout_engine_impl &other) = delete;
    text_layout_engine_impl(text_layout_engine_impl &&other) noexcept = default;
    text_layout_engine_impl &operator=(text_layout_engine_impl &&other) noexcept = default;
    ~text_layout_engine_impl() = default;
};

} // namespace camellia::text_layout_helper

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

// Interceptor painter implementations
void interceptor_painter_impl::drawRect(const SkRect &rect, const SkPaintOrID &paint) {
    auto rect_info = std::make_unique<rect_deco_info>();
    rect_info->xywh = {rect.x() + _offset_x, rect.y() + _offset_y, rect.width(), rect.height()};
    rect_info->filled = false;
    _shapes.push_back(std::move(rect_info));
}

void interceptor_painter_impl::drawFilledRect(const SkRect &rect, const DecorationStyle &decorStyle) {
    auto rect_info = std::make_unique<rect_deco_info>();
    rect_info->xywh = {rect.x() + _offset_x, rect.y() + _offset_y, rect.width(), rect.height()};
    rect_info->filled = true;
    rect_info->color = decorStyle.getColor();
    _shapes.push_back(std::move(rect_info));
}

void interceptor_painter_impl::drawPath(const SkPath &path, const DecorationStyle &decorStyle) {
    // TODO: Implement path drawing
}

void interceptor_painter_impl::drawLine(SkScalar x0, SkScalar y0, SkScalar x1, SkScalar y1, const DecorationStyle &decorStyle) {
    auto line_info = std::make_unique<line_deco_info>();
    line_info->start = {x0 + _offset_x, y0 + _offset_y};
    line_info->end = {x1 + _offset_x, y1 + _offset_y};
    line_info->color = decorStyle.getColor();
    _shapes.push_back(std::move(line_info));
}

// interceptor_painter wrapper implementation
interceptor_painter::interceptor_painter() : _impl(std::make_unique<interceptor_painter_impl>()) {}

interceptor_painter::~interceptor_painter() = default;

interceptor_painter::interceptor_painter(interceptor_painter &&other) noexcept = default;

interceptor_painter &interceptor_painter::operator=(interceptor_painter &&other) noexcept = default;

const std::vector<std::unique_ptr<deco_info>> &interceptor_painter::get_shapes() const { return _impl->get_shapes(); }

// paragraph_style implementation
paragraph_style::paragraph_style() : _impl(std::make_unique<paragraph_style_impl>()) {}

paragraph_style::paragraph_style(const paragraph_style &other) : _impl(std::make_unique<paragraph_style_impl>(*other._impl)) {}

paragraph_style &paragraph_style::operator=(const paragraph_style &other) {
    if (this != &other) {
        _impl = std::make_unique<paragraph_style_impl>(*other._impl);
    }
    return *this;
}

paragraph_style::paragraph_style(paragraph_style &&other) noexcept = default;

paragraph_style &paragraph_style::operator=(paragraph_style &&other) noexcept = default;

paragraph_style::~paragraph_style() = default;

void paragraph_style::set_text_align(text_align align) { _impl->_paragraph_style.setTextAlign(to_skia_text_align(align)); }

paragraph_style::text_align paragraph_style::get_text_align() const { return from_skia_text_align(_impl->_paragraph_style.getTextAlign()); }

void paragraph_style::set_text_direction(text_direction direction) { _impl->_paragraph_style.setTextDirection(to_skia_text_direction(direction)); }

paragraph_style::text_direction paragraph_style::get_text_direction() const { return from_skia_text_direction(_impl->_paragraph_style.getTextDirection()); }

void paragraph_style::set_line_height(number_t height) { _impl->_paragraph_style.setHeight(height); }

number_t paragraph_style::get_line_height() const { return _impl->_paragraph_style.getHeight(); }

void paragraph_style::set_max_lines(integer_t max_lines) { _impl->_paragraph_style.setMaxLines(max_lines); }

integer_t paragraph_style::get_max_lines() const { return static_cast<integer_t>(_impl->_paragraph_style.getMaxLines()); }

void paragraph_style::set_ellipsis(const text_t &ellipsis) { _impl->_paragraph_style.setEllipsis(SkString(ellipsis.c_str())); }

text_t paragraph_style::get_ellipsis() const { return text_t(_impl->_paragraph_style.getEllipsis().c_str()); }

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
text_style::text_style() : _impl(std::make_unique<text_style_impl>()) {}

text_style::text_style(const text_style &other) : _impl(std::make_unique<text_style_impl>(*other._impl)) {}

text_style &text_style::operator=(const text_style &other) {
    if (this != &other) {
        _impl = std::make_unique<text_style_impl>(*other._impl);
    }
    return *this;
}

text_style::text_style(text_style &&other) noexcept = default;

text_style &text_style::operator=(text_style &&other) noexcept = default;

text_style::~text_style() = default;

void text_style::set_font_size(number_t size) {
    if (size < 0.0F) {
        throw std::runtime_error(std::format("Invalid font size: {}", size));
    }
    _impl->_text_style.setFontSize(size);
}

number_t text_style::get_font_size() const { return _impl->_text_style.getFontSize(); }

void text_style::set_font_weight(font_weight weight) {
    if (!is_valid_font_weight(weight)) {
        throw std::runtime_error(std::format("Invalid font weight: {}", static_cast<integer_t>(weight)));
    }
    auto current_style = _impl->_text_style.getFontStyle();
    SkFontStyle new_style(to_skia_font_weight(weight), current_style.width(), current_style.slant());
    _impl->_text_style.setFontStyle(new_style);
}

text_style::font_weight text_style::get_font_weight() const { return static_cast<text_style::font_weight>(_impl->_text_style.getFontStyle().weight()); }

void text_style::set_font_style(font_style style) {
    if (!is_valid_font_style(style)) {
        throw std::runtime_error(std::format("Invalid font style: {}", static_cast<integer_t>(style)));
    }
    auto current_style = _impl->_text_style.getFontStyle();
    SkFontStyle new_style(current_style.weight(), current_style.width(), to_skia_font_slant(style));
    _impl->_text_style.setFontStyle(new_style);
}

text_style::font_style text_style::get_font_style() const { return from_skia_font_slant(_impl->_text_style.getFontStyle().slant()); }

void text_style::set_font_family(const text_t &family) {
    // we don't know if the font family is valid yet, so we just set it
    std::vector<SkString> families = {SkString(family.c_str())};
    _impl->_text_style.setFontFamilies(families);
}

text_t text_style::get_font_family() const {
    auto families = _impl->_text_style.getFontFamilies();
    if (!families.empty()) {
        return text_t(families[0].c_str());
    }
    return "";
}

void text_style::set_color(integer_t color) { _impl->_text_style.setColor(static_cast<SkColor>(color)); }

integer_t text_style::get_color() const { return static_cast<integer_t>(_impl->_text_style.getColor()); }

void text_style::set_background_color(integer_t color) {
    SkPaint paint;
    paint.setColor(static_cast<SkColor>(color));
    _impl->_text_style.setBackgroundColor(paint);
}

integer_t text_style::get_background_color() const {
    // Note: This is a simplified approach since Skia stores background as a paint
    if (_impl->_text_style.hasBackground()) {
        return static_cast<integer_t>(_impl->_text_style.getBackground().getColor());
    }
    return 0x00000000; // Transparent
}

void text_style::set_decoration(text_decoration decoration) {
    if (!is_valid_text_decoration(decoration)) {
        throw std::runtime_error(std::format("Invalid text decoration: {}", static_cast<integer_t>(decoration)));
    }
    _impl->_text_style.setDecoration(static_cast<skia::textlayout::TextDecoration>(decoration));
}

text_style::text_decoration text_style::get_decoration() const { return static_cast<text_style::text_decoration>(_impl->_text_style.getDecoration().fType); }

void text_style::set_decoration_color(integer_t color) { _impl->_text_style.setDecorationColor(static_cast<SkColor>(color)); }

integer_t text_style::get_decoration_color() const { return static_cast<integer_t>(_impl->_text_style.getDecorationColor()); }

void text_style::set_decoration_style(decoration_style style) {
    if (!is_valid_decoration_style(style)) {
        throw std::runtime_error(std::format("Invalid decoration style: {}", static_cast<integer_t>(style)));
    }
    _impl->_text_style.setDecorationStyle(to_skia_decoration_style(style));
}

text_style::decoration_style text_style::get_decoration_style() const { return from_skia_decoration_style(_impl->_text_style.getDecorationStyle()); }

void text_style::set_decoration_thickness(number_t thickness) {
    if (thickness < 0.0F) {
        throw std::runtime_error(std::format("Invalid decoration thickness: {}", thickness));
    }
    _impl->_text_style.setDecorationThicknessMultiplier(thickness);
}

number_t text_style::get_decoration_thickness() const { return _impl->_text_style.getDecorationThicknessMultiplier(); }

void text_style::set_letter_spacing(number_t spacing) { _impl->_text_style.setLetterSpacing(spacing); }

number_t text_style::get_letter_spacing() const { return _impl->_text_style.getLetterSpacing(); }

void text_style::set_word_spacing(number_t spacing) { _impl->_text_style.setWordSpacing(spacing); }

number_t text_style::get_word_spacing() const { return _impl->_text_style.getWordSpacing(); }

// font implementation
font::font() : _impl(std::make_unique<font_impl>()) {}

font::font(const font &other) : _impl(std::make_unique<font_impl>(*other._impl)) {}

font &font::operator=(const font &other) {
    if (this != &other) {
        _impl = std::make_unique<font_impl>(*other._impl);
    }
    return *this;
}

font::font(font &&other) noexcept = default;

font &font::operator=(font &&other) noexcept = default;

font::~font() = default;

void font::set_size(number_t size) { _impl->_font.setSize(size); }

number_t font::get_size() const { return _impl->_font.getSize(); }

void font::set_scale_x(number_t scale) { _impl->_font.setScaleX(scale); }

number_t font::get_scale_x() const { return _impl->_font.getScaleX(); }

void font::set_skew_x(number_t skew) { _impl->_font.setSkewX(skew); }

number_t font::get_skew_x() const { return _impl->_font.getSkewX(); }

void font::set_spacing(number_t spacing) { /* Note: SkFont doesn't have direct spacing methods */ }

number_t font::get_spacing() const { return 0.0f; }

void font::set_embolden(boolean_t embolden) { _impl->_font.setEmbolden(embolden); }

boolean_t font::get_embolden() const { return _impl->_font.isEmbolden(); }

void font::set_baseline_snap(boolean_t snap) { _impl->_font.setBaselineSnap(snap); }

boolean_t font::get_baseline_snap() const { return _impl->_font.isBaselineSnap(); }

void font::set_linear_metrics(boolean_t linear) { _impl->_font.setLinearMetrics(linear); }

boolean_t font::get_linear_metrics() const { return _impl->_font.isLinearMetrics(); }

void font::set_subpixel(boolean_t subpixel) { _impl->_font.setSubpixel(subpixel); }

boolean_t font::get_subpixel() const { return _impl->_font.isSubpixel(); }

void font::set_embedded_bitmaps(boolean_t embedded) { _impl->_font.setEmbeddedBitmaps(embedded); }

boolean_t font::get_embedded_bitmaps() const { return _impl->_font.isEmbeddedBitmaps(); }

void font::set_force_auto_hinting(boolean_t force) { _impl->_font.setForceAutoHinting(force); }

boolean_t font::get_force_auto_hinting() const { return _impl->_font.isForceAutoHinting(); }

void font::set_hinting(hinting hint) {
    switch (hint) {
    case hinting::NONE:
        _impl->_font.setHinting(SkFontHinting::kNone);
        break;
    case hinting::SLIGHT:
        _impl->_font.setHinting(SkFontHinting::kSlight);
        break;
    case hinting::NORMAL:
        _impl->_font.setHinting(SkFontHinting::kNormal);
        break;
    case hinting::FULL:
        _impl->_font.setHinting(SkFontHinting::kFull);
        break;
    }
}

font::hinting font::get_hinting() const {
    switch (_impl->_font.getHinting()) {
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

void font::set_edging(edging edge) { _impl->_font.setEdging(to_skia_edging(edge)); }

font::edging font::get_edging() const { return from_skia_edging(_impl->_font.getEdging()); }

// text_paragraph implementation
text_paragraph::text_paragraph() : _impl(std::make_unique<text_paragraph_impl>()) {}

text_paragraph::text_paragraph(text_paragraph &&other) noexcept = default;

text_paragraph &text_paragraph::operator=(text_paragraph &&other) noexcept = default;

text_paragraph::~text_paragraph() = default;

void text_paragraph::layout(number_t width) { _impl->_p_paragraph->layout(width); }

void text_paragraph::visit_foreground(visit_foreground_cb callback) {
    auto *impl = static_cast<skia::textlayout::ParagraphImpl *>(_impl->_p_paragraph.get());

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
                    info._glyphs = std::vector<unsigned short>(run->glyphs().begin() + context.pos, run->glyphs().begin() + context.pos + context.size);

                    std::vector<vector2> positions;
                    positions.reserve(context.size);
                    for (int i = 0; i < context.size; ++i) {
                        auto pos = run->positions()[context.pos + i];
                        positions.push_back({pos.x(), pos.y()});
                    }
                    info._positions = std::move(positions);

                    info._font._impl = std::make_unique<font_impl>(run->font());
                    info._style._impl = std::make_unique<text_style_impl>(style);

                    callback(info);
                });
            return true;
        });
        ++lineNumber;
    }
}

void text_paragraph::visit_background(visit_background_cb callback) {
    auto *impl = static_cast<skia::textlayout::ParagraphImpl *>(_impl->_p_paragraph.get());

    int lineNumber = 0;
    for (auto &line : impl->lines()) {
        line.iterateThroughVisualRuns(false, [&](const skia::textlayout::Run *run, SkScalar runOffsetInLine, skia::textlayout::TextRange textRange,
                                                 SkScalar *runWidthInLine) {
            *runWidthInLine = line.iterateThroughSingleRunByStyles(
                skia::textlayout::TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, skia::textlayout::StyleType::kBackground,
                [&](skia::textlayout::TextRange textRange, const skia::textlayout::TextStyle &style, const skia::textlayout::TextLine::ClipContext &context) {
                    shape_visit_info info;
                    info._line_number = lineNumber;
                    info._start = textRange.start;
                    info._end = textRange.end;

                    // Paint background using the interceptor - simplified implementation
                    if (style.hasBackground()) {
                        auto rect = context.clip.makeOffset(line.offset());
                        SkPaint paint;
                        info._painter._impl->drawRect(rect, paint);
                    }

                    callback(info);
                });
            return true;
        });
        ++lineNumber;
    }
}

void text_paragraph::visit_decoration(visit_decoration_cb callback) {
    auto *impl = static_cast<skia::textlayout::ParagraphImpl *>(_impl->_p_paragraph.get());

    int lineNumber = 0;
    for (auto &line : impl->lines()) {
        line.iterateThroughVisualRuns(false, [&](const skia::textlayout::Run *run, SkScalar runOffsetInLine, skia::textlayout::TextRange textRange,
                                                 SkScalar *runWidthInLine) {
            *runWidthInLine = line.iterateThroughSingleRunByStyles(
                skia::textlayout::TextLine::TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, skia::textlayout::StyleType::kDecorations,
                [&](skia::textlayout::TextRange textRange, const skia::textlayout::TextStyle &style, const skia::textlayout::TextLine::ClipContext &context) {
                    shape_visit_info info;
                    info._line_number = lineNumber;
                    info._start = textRange.start;
                    info._end = textRange.end;

                    // Paint decorations using the interceptor - simplified implementation
                    auto rect = context.clip.makeOffset(line.offset());
                    SkPaint paint;
                    info._painter._impl->drawRect(rect, paint);

                    callback(info);
                });
            return true;
        });
        ++lineNumber;
    }
}

// text_layout_engine implementation
text_layout_engine::text_layout_engine(const std::vector<bytes_t> &font_data_vector) : _impl(std::make_unique<text_layout_engine_impl>()) {

    // Convert font data to Skia format
    for (const auto &font_data : font_data_vector) {
        auto sk_data = SkData::MakeWithCopy(font_data.data(), font_data.size());
        _impl->_font_data_vector.push_back(sk_data);
    }

    // Create font manager
    _impl->_p_font_mgr = SkFontMgr_New_Custom_Data(_impl->_font_data_vector);

    // Create font collection
    _impl->_p_font_collection = sk_make_sp<skia::textlayout::FontCollection>();
    _impl->_p_font_collection->setDefaultFontManager(_impl->_p_font_mgr);
}

text_layout_engine::text_layout_engine(text_layout_engine &&other) noexcept = default;

text_layout_engine &text_layout_engine::operator=(text_layout_engine &&other) noexcept = default;

text_layout_engine::~text_layout_engine() = default;

text_paragraph text_layout_engine::create_paragraph(const text_t &text, const paragraph_style &paragraph_style) {
    auto builder = skia::textlayout::ParagraphBuilder::make(paragraph_style._impl->_paragraph_style, _impl->_p_font_collection);
    builder->addText(text.c_str());
    auto p_paragraph = builder->Build();

    text_paragraph result;
    result._impl = std::make_unique<text_paragraph_impl>(std::move(p_paragraph));
    return result;
}

text_paragraph text_layout_engine::create_paragraph(const std::vector<text_segment> &segments, const paragraph_style &paragraph_style) {
    auto builder = skia::textlayout::ParagraphBuilder::make(paragraph_style._impl->_paragraph_style, _impl->_p_font_collection);

    for (const auto &segment : segments) {
        builder->pushStyle(segment.style._impl->_text_style);
        builder->addText(segment.text.c_str());
        builder->pop();
    }

    auto p_paragraph = builder->Build();

    text_paragraph result;
    result._impl = std::make_unique<text_paragraph_impl>(std::move(p_paragraph));
    return result;
}

font text_layout_engine::create_font() const { return font(); }

font text_layout_engine::create_font(number_t size) const {
    font result;
    result.set_size(size);
    return result;
}

} // namespace camellia::text_layout_helper