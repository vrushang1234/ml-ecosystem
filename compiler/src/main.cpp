#include "ast.hpp"
#include "helper.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
int main()
{
    std::ifstream file("../test/test.nn");
    std::string s;
    std::string code_string;
    while (std::getline(file, s)) {
        code_string += s;
    }
    try {
        std::vector<Token> tokenList{tokenize(code_string)};
        printTokens(tokenList);
        Program program{parse(tokenList)};
        printAST(program);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    if (file.eof())
        std::cout << "Reached end of file." << std::endl;
    else
        std::cerr << "Error: File reading failed!" << std::endl;
    file.close();
    return 0;
}
