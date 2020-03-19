#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>

using namespace std;

#include "Tokenizer.h"

// parse the rest of a symbol
auto symbol_token::parse_token(fstream& stream, int input_char) -> int
{
    symbol_ = input_char;
    while (true)
    {
        input_char = stream.get();
        if (isalpha(input_char) || isdigit(input_char) || input_char == '_')
        {
            symbol_ += input_char;
            continue;
        }
        return input_char;
    }
}

// parse the rest of an integer
auto integer_token::parse_token(fstream& stream, int input_char) -> int
{
    integer_string_ = input_char;
    if (input_char == '0')
    {
        input_char = stream.peek();
        if (input_char == 'X' || input_char == 'x')
        {
            integer_string_ += input_char;
            stream.get();
            while (true)
            {
                input_char = stream.get();
                if (isxdigit(input_char))
                {
                    integer_string_ += input_char;
                    continue;
                }
                return input_char;
            }
        }
        // if no space after a number, then symbol is illegal
        if (input_char != ' ')
        {
            cout << static_cast<size_t>(stream.tellg()) - 1 << ": Illegal symbol. Exit." << endl;
            exit(-1);
        }
    }
    while (true)
    {
        input_char = stream.get();
        if (isdigit(input_char))
        {
            integer_string_ += input_char;
            continue;
        }
        return input_char;
    }
}

// parse the double quote
auto double_quote_token::parse_token(fstream& stream, int input_char) -> int
{
    literal_string_.clear();
    while (true)
    {
        input_char = stream.get();
        if (input_char != '\"' && input_char != -1)
        {
            literal_string_ += input_char;
            continue;
        }
        if (input_char == -1)
        {
            cout << "error: EOF encountered before closing literal quotes" << endl;
            exit(0);
        }
        input_char = stream.get();
        return input_char;
    }
}

// parse the assignment operator
auto assignment_token::parse_token(fstream& stream, int input_char) -> int
{
    punctuation_string_ = input_char;
    input_char = stream.peek();
    if (input_char == '=')
    {
        input_char = stream.get();
        punctuation_string_ += input_char;
    }
    input_char = stream.get();
    return input_char;
}

// parse the whitespace characters
auto whitespace_token::parse_token(fstream& stream, int input_char) -> int
{
    while (true)
    {
        input_char = stream.get();
        if (input_char == ' ')
        {
            continue;
        }
        return input_char;
    }
}

// parse the eol character
auto eol_token::parse_token(fstream& stream, int input_char) -> int
{
    while (true)
    {
        input_char = stream.get();
        return input_char;
    }
}

// parse the eof character
auto eof_token::parse_token(fstream& stream, int input_char) -> int
{
    return 0;
}

// parse the invalid character
auto invalid_token::parse_token(fstream& stream, int input_char) -> int
{
    invalid_character_ = input_char;
    input_char = stream.get();
    return input_char;
}

// parse the input source
auto token_parser::tokenize() -> bool
{
    shared_ptr<base_token> token;

    while (!source_stream_.eof())
    {
        auto input_char = source_stream_.get();

        // Determine what the leading character is of the sequence,
        // create an appropriate token and get the actual token
        // class to parse the rest of it (if any)

        while (!source_stream_.eof())
        {
            while (true)
            {
                if (isalpha(input_char) || input_char == '_')
                {
                    token = make_shared<symbol_token>();
                    break;
                }
                if (input_char == 0x0A)
                {
                    token = make_shared<eol_token>();
                    break;
                }
                if (isspace(input_char))
                {
                    token = make_shared<whitespace_token>();
                    break;
                }
                if (input_char == '"')
                {
                    token = make_shared<double_quote_token>();
                    break;
                }
                if (isdigit(input_char))
                {
                    token = make_shared<integer_token>();
                    break;
                }
                if (ispunct(input_char))
                {
                    token = make_shared<assignment_token>();
                    break;
                }
            }

            if (token == nullptr)
                return false;

            // save position in stream of the current token
            token->set_pos(static_cast<size_t>(source_stream_.tellg()) - 1);

            // start parsing it
            input_char = token->parse_token(source_stream_, input_char);

            // append token to the the list
            // ignore whitespaces and EOL for better performance when parsing is done later
            if (token->type() != base_token::t_whitespace && (token->type() != base_token::t_eol))
                token_list_.emplace_back(token);
        }
    }
    // Add the EOF token to the end of the list
    token = make_shared<eof_token>();
    if (token != nullptr)
        token_list_.emplace_back(token);

    node_iterator_ = token_list_.begin();
    return true;
}

auto token_parser::parse() -> void
{
    auto id = 1;

    auto parent_node = make_shared<node>(id); // this is the root node

    auto tmp_node = parent_node; // will hold the parent node during parsing

    node_list_.emplace_back(tmp_node); // a list of parsing result

    shared_ptr<base_token> current_token; // current token in token_list
    shared_ptr<base_token> next_token; // next token in token_list

    while (((current_token = get_next()) != nullptr) && current_token->type() != base_token::t_eof)
    {
        auto val = current_token->get_value();
        next_token = peek_next();
        switch (current_token->type())
        {
        case base_token::t_symbol:
            if (next_token == nullptr)
            {
                parse_error("=", "nullptr", next_token->get_pos());
            }

            tmp_node->set_name(val);

            if (next_token->type() == base_token::t_punctuation)
            {
                if (next_token->get_value() == "=")
                {
                    continue;
                }
                parse_error("=", next_token->get_value(), next_token->get_pos());
            }
            else
            {
                parse_error("=", next_token->get_value(), next_token->get_pos());
            }

            continue;

        case base_token::t_punctuation:
            if (val == "{")
            {
                if (next_token->type() == base_token::t_symbol)
                {
                    auto new_elem = make_shared<node>(++id);
                    new_elem->add_parent(tmp_node);
                    tmp_node->add_child(new_elem);
                    node_list_.emplace_back(new_elem);
                    parent_node = tmp_node;
                    tmp_node = new_elem;
                    continue;
                }
                parse_error("a symbol", next_token->get_value(), next_token->get_pos());
            }
            else if (val == "}")
            {
                // go back to parent node
                if (tmp_node != parent_node)
                    tmp_node = parent_node->get_parent();
                if (next_token->type() == base_token::t_symbol)
                {
                    if (tmp_node != nullptr)
                    {
                        auto new_elem = make_shared<node>(++id);
                        new_elem->add_parent(tmp_node);
                        tmp_node->add_child(new_elem);
                        node_list_.emplace_back(new_elem);
                        tmp_node = new_elem;
                        continue;
                    }
                    parse_error("End of file (should have only 1 root element)", "another root element",
                                next_token->get_pos());
                }
                else if (next_token->type() == base_token::t_punctuation)
                {
                    if (next_token->get_value() == "}")
                    {
                        // go one level up for parent node
                        parent_node = parent_node->get_parent();
                        continue;
                    }
                }
                else if (next_token->type() == base_token::t_eof)
                {
                    // this was the last '}' brace
                    continue;
                }

                parse_error("a symbol or }", next_token->get_value(), next_token->get_pos());
            }
            else if (val == "=")
            {
                if (next_token->type() == base_token::t_punctuation)
                {
                    if (next_token->get_value() == "{")
                    {
                        // there will be a list
                        continue;
                    }
                    parse_error("{", next_token->get_value(), next_token->get_pos());
                }
                else if (next_token->type() == base_token::t_literal || next_token->type() == base_token::
                    t_const_literal || next_token->
                    type() == base_token::t_integer)
                {
                }
                else
                {
                    // not a valid token
                    parse_error("{ or \"value\"", next_token->get_value(), next_token->get_pos());
                }
            }
            break;

        case base_token::t_literal:
        case base_token::t_integer:
        case base_token::t_const_literal:
            tmp_node->set_data(val);
            // if next symbol, create new elem
            if (next_token->type() == base_token::t_symbol)
            {
                if (tmp_node->get_parent() != nullptr)
                {
                    shared_ptr<node> new_elem(new node(++id));
                    new_elem->add_parent(parent_node);
                    parent_node->add_child(new_elem);
                    node_list_.emplace_back(new_elem);
                    tmp_node = new_elem;
                    continue;
                }
                parse_error("End of file (should have only 1 root element)", "another root element",
                            next_token->get_pos());
            }
            else if (next_token->type() == base_token::t_punctuation)
            {
                if (next_token->get_value() == "}")
                {
                    // fine, nothing to do
                    continue;
                }
            }
            else if (next_token->type() == base_token::t_eof)
            {
                // in case node = "value" as root element
                continue;
            }
            parse_error("symbol or }", next_token->get_value(), next_token->get_pos());
            break;

        default:
            break;
        }
    }
}

auto token_parser::get_next() -> shared_ptr<base_token>
{
    if (node_iterator_ != token_list_.end())
        return (*node_iterator_++);
    return nullptr;
}

auto token_parser::peek_next() -> shared_ptr<base_token>
{
    if ((*node_iterator_) != nullptr && std::next(node_iterator_, 0) != token_list_.end())
        return (*std::next(node_iterator_, 0));
    return nullptr;
}

auto token_parser::parse_error(const string& expected, const string& got, const size_t pos) -> void
{
    cout << "At " << pos << ": Expected '" << expected << "', got '" << got << "'. Exit." << endl;
    exit(-1);
}

auto token_parser::print_file(const string& file_name) -> void
{
    const auto output_buff = std::cout.rdbuf();
    const ofstream out(file_name);

    if (out.fail())
    {
        cout << "No file specified or error occurred during opening " << file_name << endl;
        cout << "Output to standard output will be used instead." << endl;
    }
    else
    {
        std::cout.rdbuf(out.rdbuf());
    }

    // print to file/console
    auto n_it = node_list_.begin();
    while (n_it != node_list_.end())
    {
        std::cout << (*n_it++)->to_string();
    }

    std::cout.rdbuf(output_buff);
}

auto node::to_string() const -> string
{
    std::ostringstream oss;
    oss << "(" << this->get_id() << ", "
        << ((this->get_parent() != nullptr) ? this->get_parent()->get_id() : 0) << ", "
        << this->get_name() << ", "
        << this->get_data() << ")" << std::endl;

    return oss.str();
}
