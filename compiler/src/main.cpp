#include "helper.hpp"
#include "lexer.hpp"

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
    std::vector<Token> tokenList{tokenize(code_string)};
    printTokens(tokenList);
    if (file.eof())
        std::cout << "Reached end of file." << std::endl;
    else
        std::cerr << "Error: File reading failed!" << std::endl;
    file.close();
    return 0;
}
