#include "ast.hpp"
#include "parser.hpp"
#include "sema.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>
int main(int argc, char* argv[])
{
    std::string path{argc > 1 ? argv[1] : "../test/test.onnx"};
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: could not open file '" << path << "'" << std::endl;
        return 1;
    }
    std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(file),
                                    std::istreambuf_iterator<char>()};
    try {
        Program program{parse(bytes)};
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
    file.close();
    return 0;
}
