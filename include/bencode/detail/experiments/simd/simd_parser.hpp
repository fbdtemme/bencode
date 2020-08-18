////
//// Created by fbdtemme on 4/24/20.
////
//#pragma once
//#include "../base_parser.hpp"
//#include "block_reader.hpp"
//#include "token_indexer.hpp"
////#include "tape_entry.hpp"
//
//namespace bencode {
//
//// TODO: error handling
////
///// Parse a bencoded data into a bencode_token stream using an index.
//struct structural_index_parser
//{
//    struct stack_frame
//    {
//        detail::parser_state state;
//        std::uint32_t position;
//        std::uint32_t size;
//    };
//
//    using parser_state = detail::parser_state;
//    using decode_token_type = detail::decode_token_type;
//
//    structural_index_parser() = default;
//
//    structural_index_parser(const parser_options& options)
//            : options_(options) {};
//
//    using state = detail::parser_state;
//    using options = parser_options;
//
//    auto tokens() -> std::vector<decode_token>&
//    { return tokens_; }
//
//    auto tokens() const -> const std::vector<decode_token>&
//    { return tokens_; }
//
//    auto has_error() noexcept -> bool
//    { return error_.has_value(); }
//
//    auto error() noexcept -> parse_error
//    {
//        Ensures(error_.has_value());
//        return *error_;
//    }
//
//    auto parse(const token_indexer& index, block_reader<64>& reader) noexcept -> bool
//    {
//        // aliases for brevity
//        // TODO [c++20] : change to using enum
//        constexpr auto list_t = state::expect_list_value;
//        constexpr auto dict_t = state::expect_dict_value;
//        constexpr auto value_t = state::expect_value;
//
//        index_ = &index;
//        reader_ = &reader;
//
//        auto dict_begin = index.dict_begin();
//        auto list_begin = index.list_begin();
//        auto end_pos = index.end_positions();
//        auto string_len_begin = index.string_length_begin();
//        auto string_len_end = index.string_length_end();
//        auto int_begin = index.integer_begin();
//        auto int_end = index.integer_end();
//
//        while (position_ < reader.size()) {
//            // verify bvalue limits
//            if (error_.has_value()) [[unlikely]] {
//                return false;
//            }
//            if (tokens_.size() > options_.value_limit) [[unlikely]] {
//                set_error(parser_errc::value_limit_exceeded);
//                return false;
//            }
//
//            // check current parsing context
//            if (!stack_.empty()) {
//                auto [context, _1, _2] = stack_.top();
//
//                switch (context) {
//                case state::expect_dict_key: {
//                    if (position_ == string_len_begin[string_idx_]
//                            || is_digit_continuation_string_) [[likely]] {
//                        handle_dict_key();
//                        continue;
//                    }
//                    else if (position_ == end_pos[end_idx_]) {
//                        handle_dict_end();
//                        continue;
//                    }
//                    else [[unlikely]] {
//                        set_error(parser_errc::expected_dict_key_or_end);
//                        continue;
//                    }
//                }
//                case state::expect_dict_value: {
//                    if (position_ == end_pos[end_idx_]) [[unlikely]] {
//                        set_error(parser_errc::expected_dict_value);
//                        continue;
//                    }
//                    handle_value<dict_t>();
//                    continue;
//                }
//                case state::expect_list_value: {
//                    if (position_ == end_pos[end_idx_]) [[unlikely]] {
//                        handle_list_end();
//                        continue;
//                    }
//                    else {
//                        handle_value<list_t>();
//                        continue;
//                    }
//                }
//                default:
//                    set_error(parser_errc::internal_error);
//                    BENCODE_UNREACHABLE;
//                }
//            }
//            // No current parsing context. This means we are parsing the first element
//            // from the data or successive elements from a stream.
//            handle_value<value_t>();
//        }
//
//        if (error_.has_value())
//            return false;
//
//        // set stop flag on last token
//        if (!tokens_.empty())
//            tokens_.back().set_stop_flag();
//
//        if (!stack_.empty()) {
//            set_error(parser_errc::unexpected_eof);
//            return false;
//        }
//        Ensures(position_ == reader.size());
//        return true;
//    }
//
//private:
//    bool handle_integer(decode_token::token_type modifier)
//    {
//        Ensures(index_);
//        Ensures(reader_);
//
//        const auto type = (decode_token::token_type::integer | modifier);
//        auto& t = tokens_.emplace_back(type, position_);
//
//        auto i_pos = index_->integer_begin()[integer_idx_];
//        auto e_pos = index_->integer_end()[integer_idx_];
//
//        reader_->seek(i_pos+1);
//        auto s = std::string_view(reader_->data() + position_ +1, e_pos-i_pos-1);
//        const auto bvalue = detail::avx2::convert_digits(s);
//
//        t.set_value(bvalue);
//
//        ++integer_idx_;
//        position_ = e_pos+1;
//        return true;
//    }
//
//    bool handle_string(decode_token::token_type modifier)
//    {
//        Ensures(index_);
//        Ensures(reader_);
//
//        const auto type = (decode_token::token_type::string | modifier);
//
//        auto size_start_pos = position_;
//        auto size_end_pos = index_->string_length_end()[string_idx_];
//
//        auto offset = size_end_pos - size_start_pos + 1;
//
//        reader_->seek(size_start_pos);
//        std::string_view s = reader_->read_chars();
//        const auto str_size = detail::avx2::convert_digits(s.substr(0, offset-1));
//
//        auto& t = tokens_.emplace_back(type, position_);
//        t.set_offset(offset);
//        t.set_size(str_size);
//
//        ++string_idx_;
//        position_ += offset + str_size;
//        advance_positions();
//        return true;
//    }
//
//    template <state ParserState>
//    bool handle_value()
//    {
//        Expects(ParserState != state::expect_dict_key);
//        Ensures(index_);
//        Ensures(reader_);
//
//        constexpr auto type_modifier = get_type_modifier(ParserState);
//
//        // Update the size of the current list or dict
//        auto dispatch = [&](bool success = true) {
//            if (!success) [[unlikely]] return false;
//
//            if constexpr (ParserState == state::expect_dict_value) {
//                auto&[st, start, size] = stack_.top();
//                st = state::expect_dict_key;
//                ++size;
//            }
//            else if constexpr (ParserState == state::expect_list_value) {
//                auto&[st, start, size] = stack_.top();
//                ++size;
//            }
//            return true;
//        };
//
//        auto int_begin = index_->integer_begin();
//        auto string_begin = index_->string_length_begin();
//        auto list_begin = index_->list_begin();
//        auto dict_begin = index_->dict_begin();
//
//        bool success = false;
//        if (position_ == int_begin[integer_idx_]) {
//            success = dispatch(handle_integer(type_modifier));
//        }
//        else if (position_ == list_begin[list_idx_]) {
//            success =  handle_list_begin(type_modifier);
//        }
//        else if (position_ == dict_begin[dict_idx_]) {
//            success =  handle_dict_begin(type_modifier);
//        }
//        else if (position_ == string_begin[string_idx_]
//                    || is_digit_continuation_string_) {
//            success = dispatch(handle_string(type_modifier));
//        }
//        else {
//            set_error(parser_errc::expected_value);
//        }
//        return success;
//    }
//
//    bool handle_list_begin(decode_token::token_type modifier)
//    {
//        const auto type = (decode_token::token_type::list | modifier);
//
//        if (stack_.size() == options_.recursion_limit) {
//            set_error(parser_errc::recursion_depth_exceeded);
//            return false;
//        }
//
//        tokens_.emplace_back(type, position_);
//        stack_.push({
//                .state=state::expect_list_value,
//                .position=static_cast<std::uint32_t>(tokens_.size()-1),
//                .size=0
//        });
//
//        ++list_idx_;
//        ++position_;
//        return true;
//    }
//
//    bool handle_dict_begin(decode_token::token_type modifier)
//    {
//        const auto type = (decode_token::token_type::dict | modifier);
//
//        if (stack_.size() == options_.recursion_limit) {
//            set_error(parser_errc::recursion_depth_exceeded);
//            return false;
//        }
//
//        tokens_.emplace_back(type, position_);
//        stack_.push({
//                .state=state::expect_dict_key,
//                .position=static_cast<std::uint32_t>(tokens_.size()-1),
//                .size=0
//        });
//
//        ++dict_idx_;
//        ++position_;
//        return true;
//    }
//
//    bool handle_list_end()
//    {
//        Expects(stack_.top().state == state::expect_list_value);
//
//        auto type = (decode_token::token_type::list | decode_token::token_type::end);
//        const auto[state, start_pos, size] = stack_.top();
//        const auto offset = tokens_.size()-start_pos;
//
//        stack_.pop();
//
//        if (auto s = handle_nested_structures(); s)
//            type |= get_type_modifier(*s);
//
//        auto& t = tokens_.emplace_back(type, position_);
//        tokens_[start_pos].set_offset(offset);
//        tokens_[start_pos].set_size(size);
//        t.set_offset(offset);
//        t.set_size(size);
//
//        ++position_;
//        ++end_idx_;
//        return true;
//    }
//
//    bool handle_dict_end()
//    {
////        Expects(*input_ == symbol::end);
//        Expects(stack_.top().state == state::expect_dict_key);
//
//        auto type = (decode_token::token_type::dict | decode_token::token_type::end);
//        const auto&[state, start_pos, size] = stack_.top();
//        const auto offset = tokens_.size()-start_pos;
//
//        stack_.pop();
//
//        if (auto s = handle_nested_structures(); s)
//            type |= get_type_modifier(*s);
//
//        auto& t = tokens_.emplace_back(type, position_);
//        tokens_[start_pos].set_offset(offset);
//        tokens_[start_pos].set_size(size);
//        t.set_offset(offset);
//        t.set_size(size);
//
//        ++position_;
//        ++end_idx_;
//        return true;
//    }
//
//    bool handle_dict_key()
//    {
//        Expects(stack_.top().state == state::expect_dict_key);
//        Ensures(index_);
//        Ensures(reader_);
//
//        static const auto type = (decode_token::token_type::string | decode_token::token_type::dict_key);
//
//        std::size_t size_start_pos = 0;
//        size_start_pos = position_;
//        auto size_end_pos = index_->string_length_end()[string_idx_];
//
//        // offset to the first string character from size_start_pos
//        auto offset = size_end_pos - size_start_pos + 1;
//
//        reader_->seek(size_start_pos);
//        std::string_view s = reader_->read_chars();
//        const auto str_size = detail::avx2::convert_digits(s.substr(0, offset-1));
//
//        stack_.top().state = state::expect_dict_value;
//        auto& t = tokens_.emplace_back(type, position_);
//        t.set_offset(offset);
//        t.set_size(str_size);
//
//        ++string_idx_;
//        position_ += offset + str_size;
//        advance_positions();
//        return true;
//    }
//
//    inline auto handle_nested_structures() -> std::optional<state>
//    {
//        if (stack_.empty()) return std::nullopt;
//
//        auto&[state, position, size] = stack_.top();
//        const auto old_state = state;
//        ++size;
//
//        switch (state) {
//        case state::expect_dict_value:  state = state::expect_dict_key;   break;
//        case state::expect_dict_key  :  state = state::expect_dict_value; break;
//        }
//        return old_state;
//    }
//
//    /// Advance positions in the indices to match the current position.
//    /// This is used to skip structural tokens that are positions inside strings.
//    inline void advance_positions()
//    {
//        Ensures(index_);
//
//        auto int_begin = index_->integer_begin();
//        auto string_begin = index_->string_length_begin();
//        auto list_begin = index_->list_begin();
//        auto dict_begin = index_->dict_begin();
//        auto end_begin = index_->end_positions();
//
//        is_digit_continuation_string_ = false;
//
//        while ((int_begin[integer_idx_] < position_) && (integer_idx_ < int_begin.size())) {
//            ++integer_idx_;
//        };
//
//        // check for strings that end in digits with a new string after that.
//        // the string digits and string length are concatenated and the index is wrong
//        while (string_begin[string_idx_] < position_ && string_idx_ < string_begin.size())
//        {
//            auto b = std::all_of(
//                    reader_->data() + string_begin[string_idx_], reader_->data() + position_+1,
//                    [] (char c) { return std::isdigit(c); });
//            if (b) {
//                is_digit_continuation_string_ = true;
//                break;
//            }
//            else { ++string_idx_; }
//        };
//        while ((list_begin[list_idx_] < position_) && (list_idx_ < list_begin.size())) {
//            ++list_idx_;
//        };
//        while ((dict_begin[dict_idx_] < position_) && (dict_idx_ < dict_begin.size())) {
//            ++dict_idx_;
//        };
//        while ((end_begin[end_idx_] < position_)  && (end_idx_ < end_begin.size())) {
//            ++end_idx_;
//        };
//    }
//
//    inline void set_error(parser_errc ec)
//    {
//        error_.emplace(ec, position_);
//    }
//
//private:
//    static constexpr auto get_type_modifier(parser_state s) noexcept -> decode_token_type
//    {
//        switch (s) {
//        case parser_state::expect_list_value:
//            return decode_token_type::list_value;
//        case parser_state::expect_dict_key:
//            return decode_token_type::dict_key;
//        case parser_state::expect_dict_value:
//            return decode_token_type::dict_value;
//        default:
//            return {};
//        }
//    };
//
//    const token_indexer* index_ = nullptr;
//    block_reader<64>* reader_ = nullptr;
//
//    std::size_t position_ = 0;
//    std::size_t dict_idx_ = 0;
//    std::size_t list_idx_ = 0;
//    std::size_t end_idx_ = 0;
//    std::size_t string_idx_ = 0;
//    std::size_t integer_idx_ = 0;
//    bool is_digit_continuation_string_ = false;
//
//    std::vector<decode_token> tokens_ {};
//    std::stack<stack_frame> stack_ {};
//    std::optional<parse_error> error_ {};
//    const options options_ {};
//};
//
//
//
//
//} // namespace bencode
