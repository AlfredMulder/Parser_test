#pragma once

#include <fstream>
#include <list>

using namespace std;

class base_token
{
public:
    using type_of_token = enum
    {
        t_invalid_token = 0,
        t_symbol,
        t_integer,
        t_literal,
        t_const_literal,
        t_punctuation,
        t_whitespace,
        t_eol,
        t_eof
    };
private:
    type_of_token token_type_;
    size_t pos_{};
public:
    explicit base_token(const type_of_token token)
        : token_type_(token)
    {
    }

    [[nodiscard]] auto type() const -> type_of_token { return token_type_; }
    auto set_pos(const size_t p) -> void { pos_ = p; }
    [[nodiscard]] auto get_pos() const -> size_t { return pos_; }

    virtual auto parse_token(fstream& stream, int input_char) -> int = 0;
    virtual auto get_value() -> string = 0;

    virtual ~base_token() = default;
};

// A token that may contain a symbol
class symbol_token final : public base_token
{
    string symbol_;
public:
    symbol_token()
        : base_token(t_symbol)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return symbol_; }
};

// A token that represents an integer
class integer_token final : public base_token
{
    string integer_string_;
public:
    integer_token()
        : base_token(t_integer)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return integer_string_; }
};

// A token that represents a literal
class double_quote_token final : public base_token
{
    string literal_string_;
public:
    double_quote_token()
        : base_token(t_literal)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return literal_string_; }
};

// A token that represents a punctuation or separator
class assignment_token final : public base_token
{
    string punctuation_string_;
public:
    assignment_token()
        : base_token(t_punctuation)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return punctuation_string_; }
};

// A token that represents whitespace
class whitespace_token final : public base_token
{
public:
    whitespace_token()
        : base_token(t_whitespace)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return ""; }
};

// A token that represents an eol
class eol_token final : public base_token
{
public:
    eol_token()
        : base_token(t_eol)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return ""; }
};

// A token that represents an eof
class eof_token final : public base_token
{
public:
    eof_token()
        : base_token(t_eof)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return ""; }
};

// A token that represents an illegal character
class invalid_token final : public base_token
{
    int invalid_character_;
public:
    invalid_token()
        : base_token(t_invalid_token), invalid_character_(-1)
    {
    }

    auto parse_token(fstream& stream, int input_char) -> int override;
    [[nodiscard]] auto get_value() -> string override { return ""; }
};

class node
{
    int id_;
    string name_;
    string data_;
    shared_ptr<node> parent_;
    list<shared_ptr<node>> children_;
public:
    explicit node(const int id)
        : id_(id), parent_(nullptr)
    {
    }

    [[nodiscard]] auto get_id() const -> int { return id_; }
    auto set_name(const string& s) -> void { name_ = s; }
    auto set_data(const string& s) -> void { data_ = s; }
    auto add_parent(const shared_ptr<node>& n) -> void { parent_ = n; }
    auto add_child(const shared_ptr<node>& n) -> void { children_.push_back(n); }

    [[nodiscard]] auto get_data() const -> string { return data_; }
    [[nodiscard]] auto get_name() const -> string { return name_; }

    [[nodiscard]] auto get_parent() const -> shared_ptr<node> { return parent_; }
    [[nodiscard]] auto get_children() const -> list<shared_ptr<node>> { return children_; }
    [[nodiscard]] auto to_string() const -> string;
};

class token_parser
{
    fstream& source_stream_;
    list<shared_ptr<base_token>> token_list_;
    list<shared_ptr<base_token>>::iterator node_iterator_;
    list<shared_ptr<node>> node_list_;
public:
    explicit token_parser(fstream& stream)
        : source_stream_(stream)
    {
    }

    auto get_next() -> shared_ptr<base_token>;
    auto peek_next() -> shared_ptr<base_token>;
    static auto parse_error(const string& expected, const string& got, size_t pos) -> void;
    auto tokenize() -> bool;
    auto parse() -> void;
    auto print_file(const string& file_name) -> void;
};
