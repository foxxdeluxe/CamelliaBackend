#ifndef TEXT_LAYOUT_HELPER_H
#define TEXT_LAYOUT_HELPER_H

#include "variant.h"
#include <camellia_typedef.h>
#include <cmath>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

namespace camellia::text_layout_helper {

class text_layout_engine;

struct deco_info {
    integer_t color{0x00000000};

    deco_info() = default;
    deco_info(const deco_info &) = default;
    deco_info(deco_info &&) noexcept = default;
    deco_info &operator=(const deco_info &) = default;
    deco_info &operator=(deco_info &&) noexcept = default;
    virtual ~deco_info() = default;
};

struct rect_deco_info : public deco_info {
    vector4 xywh{0.0F, 0.0F, 0.0F, 0.0F};
    boolean_t filled{true};

    rect_deco_info() = default;
    rect_deco_info(const rect_deco_info &) = default;
    rect_deco_info(rect_deco_info &&) noexcept = default;
    rect_deco_info &operator=(const rect_deco_info &) = default;
    rect_deco_info &operator=(rect_deco_info &&) noexcept = default;
    ~rect_deco_info() override = default;
};

struct line_deco_info : public deco_info {
    vector2 start{0.0F, 0.0F};
    vector2 end{0.0F, 0.0F};

    line_deco_info() = default;
    line_deco_info(const line_deco_info &) = default;
    line_deco_info(line_deco_info &&) noexcept = default;
    line_deco_info &operator=(const line_deco_info &) = default;
    line_deco_info &operator=(line_deco_info &&) noexcept = default;
    ~line_deco_info() override = default;
};

struct path_deco_info : public deco_info {
    // TODO: Support path
};

// Forward declarations for PIMPL
class interceptor_painter_impl;
class paragraph_style_impl;
class text_style_impl;
class font_impl;
class text_paragraph_impl;
class text_layout_engine_impl;

class interceptor_painter {
public:
    interceptor_painter();
    ~interceptor_painter();
    interceptor_painter(const interceptor_painter &) = delete;
    interceptor_painter &operator=(const interceptor_painter &) = delete;
    interceptor_painter(interceptor_painter &&) noexcept;
    interceptor_painter &operator=(interceptor_painter &&) noexcept;

    [[nodiscard]] const std::vector<std::unique_ptr<deco_info>> &get_shapes() const;

#ifndef SWIG
private:
    friend class text_paragraph;
    std::unique_ptr<interceptor_painter_impl> _impl;
#endif
};

class paragraph_style {
public:
    // Text alignment
    enum class text_align : char { LEFT, RIGHT, CENTER, JUSTIFY, START, END };

    // Text direction
    enum class text_direction : char {
        LTR, // Left to right
        RTL  // Right to left
    };

    // Text overflow behavior
    enum class text_overflow : char { CLIP, ELLIPSIS, FADE };

    paragraph_style();
    paragraph_style(const paragraph_style &other);
    paragraph_style &operator=(const paragraph_style &other);
    paragraph_style(paragraph_style &&other) noexcept;
    paragraph_style &operator=(paragraph_style &&other) noexcept;
    ~paragraph_style();

    // Text alignment methods
    void set_text_align(text_align align);
    [[nodiscard]] text_align get_text_align() const;

    // Text direction methods
    void set_text_direction(text_direction direction);
    [[nodiscard]] text_direction get_text_direction() const;

    // Line height methods
    void set_line_height(number_t height);
    [[nodiscard]] number_t get_line_height() const;

    // Max lines methods
    void set_max_lines(integer_t max_lines);
    [[nodiscard]] integer_t get_max_lines() const;

    // Ellipsis methods
    void set_ellipsis(const text_t &ellipsis);
    [[nodiscard]] text_t get_ellipsis() const;

    // Text overflow methods
    void set_text_overflow(text_overflow overflow);
    [[nodiscard]] text_overflow get_text_overflow() const;

#ifndef SWIG
private:
    friend text_layout_engine;
    std::unique_ptr<paragraph_style_impl> _impl;
#endif
};

class text_style {
public:
    // Font weight
    enum class font_weight : short {
        THIN = 100,
        EXTRA_LIGHT = 200,
        LIGHT = 300,
        NORMAL = 400,
        MEDIUM = 500,
        SEMI_BOLD = 600,
        BOLD = 700,
        EXTRA_BOLD = 800,
        BLACK = 900
    };

    // Font style
    enum class font_style : char { NORMAL, ITALIC };

    // Text decoration
    enum class text_decoration : char { NONE = 0, UNDERLINE = 1, OVERLINE = 2, LINE_THROUGH = 4 };

    // Text decoration style
    enum class decoration_style : char { SOLID, DOUBLE, DOTTED, DASHED, WAVY };

    text_style();
    text_style(const text_style &other);
    text_style &operator=(const text_style &other);
    text_style(text_style &&other) noexcept;
    text_style &operator=(text_style &&other) noexcept;
    ~text_style();

    // Font size methods
    void set_font_size(number_t size);
    [[nodiscard]] number_t get_font_size() const;

    // Font weight methods
    void set_font_weight(font_weight weight);
    [[nodiscard]] font_weight get_font_weight() const;

    // Font style methods
    void set_font_style(font_style style);
    [[nodiscard]] font_style get_font_style() const;

    // Font family methods
    void set_font_family(const text_t &family);
    [[nodiscard]] text_t get_font_family() const;

    // Color methods (ARGB format)
    void set_color(integer_t color);
    [[nodiscard]] integer_t get_color() const;

    // Background color methods (ARGB format) - simplified implementation
    void set_background_color(integer_t color);
    [[nodiscard]] integer_t get_background_color() const;

    // Text decoration methods - simplified implementation
    void set_decoration(text_decoration decoration);
    [[nodiscard]] text_decoration get_decoration() const;

    void set_decoration_color(integer_t color);
    [[nodiscard]] integer_t get_decoration_color() const;

    void set_decoration_style(decoration_style style);
    [[nodiscard]] decoration_style get_decoration_style() const;

    void set_decoration_thickness(number_t thickness);
    [[nodiscard]] number_t get_decoration_thickness() const;

    // Letter spacing methods
    void set_letter_spacing(number_t spacing);
    [[nodiscard]] number_t get_letter_spacing() const;

    // Word spacing methods
    void set_word_spacing(number_t spacing);
    [[nodiscard]] number_t get_word_spacing() const;

#ifndef SWIG
private:
    friend class text_paragraph;
    friend class text_layout_engine;
    std::unique_ptr<text_style_impl> _impl;
#endif
};

class font {
public:
    // Font hinting levels
    enum class hinting : char { NONE, SLIGHT, NORMAL, FULL };

    // Font edging modes
    enum class edging : char { ALIAS, ANTI_ALIAS, SUBPIXEL_ANTI_ALIAS };

    font();
    font(const font &other);
    font &operator=(const font &other);
    font(font &&other) noexcept;
    font &operator=(font &&other) noexcept;
    ~font();

    // Font size methods
    void set_size(number_t size);
    [[nodiscard]] number_t get_size() const;

    // Font scale methods
    void set_scale_x(number_t scale);
    [[nodiscard]] number_t get_scale_x() const;

    void set_skew_x(number_t skew);
    [[nodiscard]] number_t get_skew_x() const;

    // Font spacing methods
    void set_spacing(number_t spacing);
    [[nodiscard]] number_t get_spacing() const;

    // Font appearance methods
    void set_embolden(boolean_t embolden);
    [[nodiscard]] boolean_t get_embolden() const;

    void set_baseline_snap(boolean_t snap);
    [[nodiscard]] boolean_t get_baseline_snap() const;

    void set_linear_metrics(boolean_t linear);
    [[nodiscard]] boolean_t get_linear_metrics() const;

    void set_subpixel(boolean_t subpixel);
    [[nodiscard]] boolean_t get_subpixel() const;

    void set_embedded_bitmaps(boolean_t embedded);
    [[nodiscard]] boolean_t get_embedded_bitmaps() const;

    void set_force_auto_hinting(boolean_t force);
    [[nodiscard]] boolean_t get_force_auto_hinting() const;

    // Font quality methods
    void set_hinting(hinting hint);
    [[nodiscard]] hinting get_hinting() const;

    void set_edging(edging edge);
    [[nodiscard]] edging get_edging() const;

#ifndef SWIG
private:
    friend class text_paragraph;
    friend class text_layout_engine;
    std::unique_ptr<font_impl> _impl;
#endif
};

struct visit_info {
public:
    [[nodiscard]] integer_t get_line_number() const { return _line_number; }
    [[nodiscard]] unsigned long get_start() const { return _start; }
    [[nodiscard]] unsigned long get_end() const { return _end; }

private:
    friend class text_paragraph;

    integer_t _line_number{0};
    unsigned long _start{0};
    unsigned long _end{0};
};

struct foreground_visit_info : public visit_info {
public:
    [[nodiscard]] float_t get_offset_x() const { return _offset_x; }
    [[nodiscard]] float_t get_offset_y() const { return _offset_y; }
    [[nodiscard]] const std::vector<unsigned short> &get_glyphs() const { return _glyphs; }
    [[nodiscard]] const std::vector<vector2> &get_positions() const { return _positions; }
    [[nodiscard]] font get_font() const { return _font; }
    [[nodiscard]] text_style get_style() const { return _style; }

private:
    friend class text_paragraph;

    float_t _offset_x{0.0F};
    float_t _offset_y{0.0F};
    std::vector<unsigned short> _glyphs;
    std::vector<vector2> _positions;
    font _font;
    text_style _style;
};

struct shape_visit_info : public visit_info {
public:
    [[nodiscard]] const std::vector<std::unique_ptr<deco_info>> &get_shapes() const { return _painter.get_shapes(); }

private:
    friend class text_paragraph;

    interceptor_painter _painter;
};

class text_paragraph {
public:
    void layout(number_t width);

    text_paragraph();
    text_paragraph(const text_paragraph &other) = delete;
    text_paragraph &operator=(const text_paragraph &other) = delete;
    text_paragraph(text_paragraph &&other) noexcept;
    text_paragraph &operator=(text_paragraph &&other) noexcept;
    ~text_paragraph();

#ifndef SWIG
    using visit_foreground_cb = std::function<void(const foreground_visit_info &)>;
    using visit_background_cb = std::function<void(const shape_visit_info &)>;
    using visit_decoration_cb = std::function<void(const shape_visit_info &)>;
    void visit_foreground(visit_foreground_cb callback);
    void visit_background(visit_background_cb callback);
    void visit_decoration(visit_decoration_cb callback);

private:
    friend text_layout_engine;
    std::unique_ptr<text_paragraph_impl> _impl;
#endif
};

// Create a paragraph consists of styled text segments
struct text_segment {
    text_t text;
    text_style style;

    text_segment(text_t t, text_style s) : text(std::move(t)), style(std::move(s)) {}
};

class text_layout_engine {
public:
    explicit text_layout_engine(const std::vector<bytes_t> &font_data_vector);
    text_layout_engine(const text_layout_engine &other) = delete;
    text_layout_engine &operator=(const text_layout_engine &other) = delete;
    text_layout_engine(text_layout_engine &&other) noexcept;
    text_layout_engine &operator=(text_layout_engine &&other) noexcept;
    ~text_layout_engine();

    text_paragraph create_paragraph(const text_t &text, const paragraph_style &paragraph_style);

    text_paragraph create_paragraph(const std::vector<text_segment> &segments, const paragraph_style &paragraph_style);

    // Create a font wrapper for use with the text layout engine
    font create_font() const;
    font create_font(number_t size) const;

#ifndef SWIG
private:
    std::unique_ptr<text_layout_engine_impl> _impl;
#endif
};

} // namespace camellia::text_layout_helper

#endif // TEXT_LAYOUT_HELPER_H