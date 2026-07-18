#include "ast.hpp"
#include "helper.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "sema.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
int main(int argc, char* argv[])
{
    std::string path{argc > 1 ? argv[1] : "../test/invalid_dup.nn"};
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: could not open file '" << path << "'" << std::endl;
        return 1;
    }
    std::string s;
    std::string code_string;
    while (std::getline(file, s)) {
        code_string += s;
        code_string += '\n';
    }
    try {
        std::vector<Token> tokenList{tokenize(code_string)};
        printTokens(tokenList);
        Program program{parse(tokenList)};
        printAST(program);
        SemaResult result{analyze(program)};
        if (!result.errors.empty()) {
            for (const std::string& error : result.errors) {
                std::cerr << "Error: " << error << std::endl;
            }
            return 1;
        }
        printResolved(result);
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
