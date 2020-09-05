#pragma once

#include "bencode/detail/bview/bview.hpp"
#include "bencode/detail/bpointer.hpp"
#include "bencode/detail/out_of_range.hpp"
#include "bencode/detail/bview/accessors.hpp"

namespace bencode::detail {

inline bview evaluate(const bpointer& pointer, const bview& bv)
{
    auto v = bv;

    for (const auto& token : pointer) {
        switch (v.type()) {
        case bencode_type::dict: {
            v = get_dict(v).at(token);
            break;
        }
        case bencode_type::list: {
            if (token=="-") [[unlikely]] {
                throw out_of_range("unresolved token '-': list index '-' is not supported");
            }

            auto res = detail::parse_integer<std::uint64_t>(token.begin(), token.end());
            if (!res) [[unlikely]] {
                throw out_of_range(
                        fmt::format("unresolved token '{}': expected list index", token));
            }

            v = get_list(v).at(res.value());
            break;
        }
        default:
            throw out_of_range(
                    fmt::format("unresolved token '{}': expected list or dict but got {}",
                            token, v.type()));
        }
    }
    return v;
}

}