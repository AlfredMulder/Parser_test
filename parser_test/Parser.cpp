#include <iostream>
#include <fstream>

#include "Tokenizer.h"

int main(const int argc, char* argv[])
{
    // Check to see that we have at least a input filename
    if (argc < 2)
    {
        cout << "Invalid command line arguments: need filename" << endl;
        cout << "  parser input_file <output_file>" << endl << endl;
        cout << "If no output file is specified, output will be done to std output." << endl;
        _exit(0);
    }

    const string input_filename = argv[1];
    auto output_filename = string::basic_string();

    if (argc > 2)
        output_filename = argv[2];

    fstream source;

    // open the source file
    source.open(input_filename.c_str(), ios_base::in);
    if (source.fail())
    {
        cout << "Error occurred during opening " << input_filename << endl;
        _exit(0);
    }

    cout << "Start parsing " << input_filename << endl;

    // Create the token list
    token_parser parser(source);

    // tokenize - lexical analysis
    parser.tokenize();

    // parse - syntax analysis
    parser.parse();

    // output
    parser.print_file(output_filename);

    cout << "Parsing finished." << endl;

    return 0;
}
