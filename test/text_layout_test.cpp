#include <gtest/gtest.h>

#include "camellia_typedef.h"
#include "helper/text_layout_helper.h"
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <vector>

using namespace camellia;

TEST(text_layout_test, test_text_layout) {
    std::ifstream font_file("/usr/share/fonts/open-sans/OpenSans-Regular.ttf", std::ios::binary);
    bytes_t font_data;
    font_file.seekg(0, std::ios::end);
    font_data.resize(font_file.tellg());
    font_file.seekg(0, std::ios::beg);
    font_file.read(reinterpret_cast<char *>(font_data.data()), font_data.size());
    font_file.close();

    text_layout_helper::text_layout_engine engine(std::vector<bytes_t>{font_data});

    text_layout_helper::text_style ts1;
    ts1.set_font_size(16);
    ts1.set_font_weight(text_layout_helper::text_style::font_weight::NORMAL);
    ts1.set_font_style(text_layout_helper::text_style::font_style::NORMAL);
    ts1.set_color(0xFFFF00FF);

    text_layout_helper::text_style ts2;
    ts2.set_font_size(32);
    ts2.set_font_weight(text_layout_helper::text_style::font_weight::NORMAL);
    ts2.set_font_style(text_layout_helper::text_style::font_style::NORMAL);
    ts2.set_color(0xFF0000FF);
    ts2.set_decoration(text_layout_helper::text_style::text_decoration::UNDERLINE);
    ts2.set_decoration_style(text_layout_helper::text_style::decoration_style::SOLID);
    ts2.set_decoration_color(0x0000FFFF);
    ts2.set_decoration_thickness(1);

    std::vector<text_layout_helper::text_segment> segments;
    segments.emplace_back("Hello, Wo", ts1);
    segments.emplace_back("rld!", ts2);

    text_layout_helper::text_paragraph paragraph = engine.create_paragraph(segments, text_layout_helper::paragraph_style());
    paragraph.layout(100);

    paragraph.visit_background([](const text_layout_helper::shape_visit_info &info) {
        std::cout << "Background: " << info.get_line_number() << ", " << info.get_start() << ", " << info.get_end() << '\n';
        for (const auto &shape : info.get_shapes()) {
            std::cout << "  Shape: " << std::hex << std::setw(8) << std::setfill('0') << shape->color << std::dec << '\n';
            auto *rect_shape = dynamic_cast<text_layout_helper::rect_deco_info *>(shape.get());
            if (rect_shape) {
                std::cout << "  Rect: " << rect_shape->xywh.get_x() << ", " << rect_shape->xywh.get_y() << ", " << rect_shape->xywh.get_z() << ", "
                          << rect_shape->xywh.get_w() << '\n';
                return;
            }
            auto *line_shape = dynamic_cast<text_layout_helper::line_deco_info *>(shape.get());
            if (line_shape) {
                std::cout << "  Line: " << line_shape->start.get_x() << ", " << line_shape->start.get_y() << ", " << line_shape->end.get_x() << ", "
                          << line_shape->end.get_y() << '\n';
                return;
            }
            auto *path_shape = dynamic_cast<text_layout_helper::path_deco_info *>(shape.get());
            if (path_shape) {
                std::cout << "  Path: " << '\n';
                return;
            }
        }
    });

    paragraph.visit_foreground([](const text_layout_helper::foreground_visit_info &info) {
        std::cout << "Foreground: " << info.get_line_number() << ", " << info.get_start() << ", " << info.get_end() << '\n';

        std::cout << "Font: " << info.get_font().get_size() << ", " << std::hex << std::setw(8) << std::setfill('0') << info.get_style().get_color() << std::dec
                  << ", " << info.get_style().get_font_family() << '\n';

        for (size_t i = 0; i < info.get_glyphs().size(); i++) {
            std::cout << info.get_glyphs()[i] << ' ' << info.get_positions()[i].get_x() << ", " << info.get_positions()[i].get_y() << '\n';
        }
        std::cout << '\n';
    });

    paragraph.visit_decoration([](const text_layout_helper::shape_visit_info &info) {
        std::cout << "Decoration: " << info.get_line_number() << ", " << info.get_start() << ", " << info.get_end() << '\n';
        for (const auto &shape : info.get_shapes()) {
            std::cout << "  Shape: " << std::hex << std::setw(8) << std::setfill('0') << shape->color << std::dec << '\n';
            auto *rect_shape = dynamic_cast<text_layout_helper::rect_deco_info *>(shape.get());
            if (rect_shape) {
                std::cout << "  Rect: " << rect_shape->xywh.get_x() << ", " << rect_shape->xywh.get_y() << ", " << rect_shape->xywh.get_z() << ", "
                          << rect_shape->xywh.get_w() << '\n';
                return;
            }
            auto *line_shape = dynamic_cast<text_layout_helper::line_deco_info *>(shape.get());
            if (line_shape) {
                std::cout << "  Line: " << line_shape->start.get_x() << ", " << line_shape->start.get_y() << ", " << line_shape->end.get_x() << ", "
                          << line_shape->end.get_y() << '\n';
                return;
            }
            auto *path_shape = dynamic_cast<text_layout_helper::path_deco_info *>(shape.get());
            if (path_shape) {
                std::cout << "  Path: " << '\n';
                return;
            }
        }
    });
}