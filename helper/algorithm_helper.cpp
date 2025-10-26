
#include "algorithm_helper.h"
#include "camellia_typedef.h"
#include "variant.h"
#include "xxhash.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace camellia::algorithm_helper {
boolean_t approx_equals(number_t a, number_t b) {
    auto tolerance = std::max({1.0F, std::fabsf(a), std::fabsf(b)});
    return std::fabsf(a - b) <= std::numeric_limits<float>::epsilon() * tolerance;
}

integer_t get_bbcode_string_length(const text_t &bbcode) {
    integer_t res = 0;
    for (int i = 0; i < bbcode.length(); i++) {
        res++;
        if (bbcode[i] != '[') {
            continue;
        }

        auto j = i + 1;
        while (j < bbcode.length() && bbcode[j] != ']') {
            j++;
        }
        if (j >= bbcode.length()) {
            res += j - i - 1;
            break;
        }

        i = j;
    }

    return res;
}

hash_t calc_hash(const std::string &str) noexcept {
    auto hash = XXH3_64bits_withSeed(str.data(), str.size(), XXHASH_SEED);
    if (hash >= RESERVE_SIZE) [[likely]] {
        return hash;
    }

    // Jackpot!!!
    for (auto seed = XXHASH_SEED + 1; hash < RESERVE_SIZE; seed++) {
        hash = XXH3_64bits_withSeed(str.data(), str.size(), seed);
    }
    return hash;
}

hash_t calc_hash(const char *str) noexcept {
    auto len = std::strlen(str);
    auto hash = XXH3_64bits_withSeed(str, len, XXHASH_SEED);
    if (hash >= RESERVE_SIZE) [[likely]] {
        return hash;
    }

    // Jackpot!!!
    for (auto seed = XXHASH_SEED + 1; hash < RESERVE_SIZE; seed++) {
        hash = XXH3_64bits_withSeed(str, len, seed);
    }
    return hash;
}

number_t calc_bbcode_node_duration(const bbcode::bbcode_node &node, number_t duration_per_char) {
    switch (node.get_type()) {
    case bbcode::bbcode_node::TYPE_TEXT:
        return static_cast<number_t>(static_cast<const bbcode::text_node &>(node).text.length()) * duration_per_char;
    case bbcode::bbcode_node::TYPE_TAG: {
        const auto &tag_node = static_cast<const bbcode::tag_node &>(node);

        number_t speed_multiplier = 1.0F;
        if (tag_node.tag_name == "speed" && !tag_node.params.empty()) {
            try {
                speed_multiplier = std::stof(tag_node.params[0]);
            } catch (const std::exception &e) {
                speed_multiplier = 1.0F;
            }
        }

        number_t duration = 0.0F;
        for (const auto &child : tag_node.children) {
            duration += calc_bbcode_node_duration(*child, duration_per_char * speed_multiplier);
        }
        return duration;
    }
    }
}

number_t calc_bbcode_duration(const bbcode &bbcode, number_t duration_per_char) {
    number_t duration = 0.0F;
    for (const auto &node : bbcode.root_nodes) {
        duration += calc_bbcode_node_duration(*node, duration_per_char);
    }
    return duration;
}

bbcode::bbcode(const text_t &text) {
    enum class State {
        NORMAL,       // Normal text
        BRACKET_OPEN, // Just saw '['
        TAG_NAME,     // Reading tag name
        TAG_PARAMS,   // Reading tag parameters
        CLOSING_TAG,  // Reading closing tag
        BRACKET_CLOSE // Just saw ']'
    };

    struct ParseContext {
        State state = State::NORMAL;
        text_t buffer;
        text_t tag_name;
        text_t params_buffer;
        std::vector<tag_node *> node_stack;
        integer_t pos = 0;
    };

    ParseContext ctx;

    auto get_current_children = [&]() -> std::vector<std::unique_ptr<bbcode_node>> & {
        if (ctx.node_stack.empty()) {
            return root_nodes;
        }
        return ctx.node_stack.back()->children;
    };

    auto flush_text = [&]() {
        if (!ctx.buffer.empty()) {
            auto node = std::make_unique<text_node>();
            node->text = ctx.buffer;
            get_current_children().push_back(std::move(node));
            ctx.buffer.clear();
        }
    };

    auto create_tag_node = [&](const text_t &tag, const text_t &params_str) {
        flush_text();

        auto node = std::make_unique<tag_node>();
        node->tag_name = tag;

        // Parse space-separated parameters
        if (!params_str.empty()) {
            std::istringstream iss(params_str);
            text_t param;
            while (iss >> param) {
                node->params.push_back(param);
            }
        }

        tag_node *node_ptr = node.get();
        get_current_children().push_back(std::move(node));
        ctx.node_stack.push_back(node_ptr);
    };

    auto close_tag = [&](const text_t &tag) {
        flush_text();

        if (ctx.node_stack.empty()) {
            throw std::runtime_error("BBCode parse error: Unexpected closing tag [/" + tag + "] at position " + std::to_string(ctx.pos));
        }

        tag_node *closing_node = ctx.node_stack.back();
        if (closing_node->tag_name != tag) {
            throw std::runtime_error("BBCode parse error: Mismatched closing tag [/" + tag + "], expected [/" + closing_node->tag_name + "] at position " +
                                     std::to_string(ctx.pos));
        }

        ctx.node_stack.pop_back();
    };

    // Main parsing loop - O(n)
    for (integer_t i = 0; i < static_cast<integer_t>(text.length()); ++i) {
        ctx.pos = i;
        char c = text[i];

        switch (ctx.state) {
        case State::NORMAL:
            if (c == '[') {
                // Check for escaped bracket [[
                if (i + 1 < static_cast<integer_t>(text.length()) && text[i + 1] == '[') {
                    ctx.buffer += '[';
                    ++i; // Skip next bracket
                } else {
                    ctx.state = State::BRACKET_OPEN;
                    ctx.tag_name.clear();
                    ctx.params_buffer.clear();
                }
            } else if (c == ']') {
                // Check for escaped bracket ]]
                if (i + 1 < static_cast<integer_t>(text.length()) && text[i + 1] == ']') {
                    ctx.buffer += ']';
                    ++i; // Skip next bracket
                } else {
                    throw std::runtime_error("BBCode parse error: Unexpected ']' at position " + std::to_string(i));
                }
            } else {
                ctx.buffer += c;
            }
            break;

        case State::BRACKET_OPEN:
            if (c == '/') {
                // Closing tag
                ctx.state = State::CLOSING_TAG;
            } else if (c == ']') {
                throw std::runtime_error("BBCode parse error: Empty tag at position " + std::to_string(i));
            } else if (std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_') {
                ctx.tag_name += c;
                ctx.state = State::TAG_NAME;
            } else {
                throw std::runtime_error("BBCode parse error: Invalid tag name start at position " + std::to_string(i));
            }
            break;

        case State::TAG_NAME:
            if (c == ' ') {
                // Parameters follow
                ctx.state = State::TAG_PARAMS;
            } else if (c == ']') {
                // Tag with no parameters
                create_tag_node(ctx.tag_name, "");
                ctx.state = State::NORMAL;
            } else if (std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_') {
                ctx.tag_name += c;
            } else {
                throw std::runtime_error("BBCode parse error: Invalid character in tag name at position " + std::to_string(i));
            }
            break;

        case State::TAG_PARAMS:
            if (c == ']') {
                // End of tag
                create_tag_node(ctx.tag_name, ctx.params_buffer);
                ctx.state = State::NORMAL;
            } else {
                ctx.params_buffer += c;
            }
            break;

        case State::CLOSING_TAG:
            if (c == ']') {
                // End of closing tag
                close_tag(ctx.tag_name);
                ctx.state = State::NORMAL;
            } else if (std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_') {
                ctx.tag_name += c;
            } else {
                throw std::runtime_error("BBCode parse error: Invalid character in closing tag at position " + std::to_string(i));
            }
            break;

        case State::BRACKET_CLOSE:
            // Should not reach here in normal flow
            break;
        }
    }

    // Check for incomplete tags
    if (ctx.state != State::NORMAL) {
        throw std::runtime_error("BBCode parse error: Incomplete tag at end of string");
    }

    // Check for unclosed tags
    if (!ctx.node_stack.empty()) {
        tag_node *unclosed_node = ctx.node_stack.back();
        throw std::runtime_error("BBCode parse error: Unclosed tag [" + unclosed_node->tag_name + "]");
    }

    // Flush any remaining text
    flush_text();
}

variant bbcode::to_variant() const {
    std::vector<variant> variant_root_nodes;
    variant_root_nodes.reserve(root_nodes.size());
    for (const auto &node : root_nodes) {
        variant_root_nodes.emplace_back(node->to_variant());
    }
    return {variant_root_nodes};
}

bbcode bbcode::from_variant(const variant &v) {
    auto variant_root_nodes = v.get_array();
    bbcode res;
    res.root_nodes.reserve(variant_root_nodes.size());
    for (const auto &node : variant_root_nodes) {
        switch (node.get_value_type()) {
        case variant::TEXT:
            res.root_nodes.push_back(std::make_unique<text_node>(text_node::from_variant(node)));
            break;
        case variant::DICTIONARY:
            res.root_nodes.push_back(std::make_unique<tag_node>(tag_node::from_variant(node)));
            break;
        default:
            throw std::runtime_error("Cannot convert variant to bbcode: Invalid root node type");
        }
    }
    return res;
}

text_t bbcode::to_text() const {
    text_t res;
    for (const auto &node : root_nodes) {
        res += node->to_text();
    }
    return res;
}

variant bbcode::tag_node::to_variant() const {
    std::map<variant, variant> dict;
    dict["tag_name"] = tag_name;

    std::vector<variant> variant_params;
    variant_params.reserve(params.size());
    for (const auto &param : params) {
        variant_params.emplace_back(param);
    }
    dict["params"] = variant_params;

    std::vector<variant> variant_children;
    variant_children.reserve(children.size());
    for (const auto &child : children) {
        variant_children.push_back(child->to_variant());
    }
    dict["children"] = variant_children;
    return {dict};
}

bbcode::tag_node bbcode::tag_node::from_variant(const variant &v) {
    tag_node node;
    auto dict = v.get_dictionary();
    node.tag_name = dict["tag_name"].get_text();
    auto variant_params = dict["params"].get_array();
    node.params.reserve(variant_params.size());
    for (const auto &param : variant_params) {
        node.params.push_back(param.get_text());
    }
    auto variant_children = dict["children"].get_array();
    node.children.reserve(variant_children.size());
    for (const auto &child : variant_children) {
        switch (child.get_value_type()) {
        case variant::TEXT:
            node.children.push_back(std::make_unique<text_node>(text_node::from_variant(child)));
            break;
        case variant::DICTIONARY:
            node.children.push_back(std::make_unique<tag_node>(tag_node::from_variant(child)));
            break;
        default:
            throw std::runtime_error("Cannot convert variant to bbcode node: Invalid child node type");
        }
    }
    return node;
}

text_t bbcode::tag_node::to_text() const {
    text_t res = "[" + tag_name;
    for (const auto &param : params) {
        res += " " + param;
    }
    res += "]";
    for (const auto &child : children) {
        res += child->to_text();
    }
    res += "[/";
    res += tag_name;
    res += "]";
    return res;
}

bbcode::text_node bbcode::text_node::from_variant(const variant &v) {
    text_node node;
    node.text = v.get_text();
    return node;
}

} // namespace camellia::algorithm_helper