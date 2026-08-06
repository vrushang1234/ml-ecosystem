#include "ast.hpp"
#include "machine.hpp"
#include "mir.hpp"
#include "parser.hpp"
#include "sema.hpp"
#include "tir.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace
{

    void usage(const char* program)
    {
        std::cerr << "Usage: " << program << " [options] <model.onnx>\n"
                  << "  --emit=ast|resolved|tir|mir   stage to print (default mir)\n"
                  << "  --format=qI.F                 fixed-point format (default q16.16)\n";
    }

} // namespace

int main(int argc, char* argv[])
{
    std::string path;
    std::string emit{"mir"};
    MachineModel machine;

    for (int i = 1; i < argc; i++) {
        std::string arg{argv[i]};
        if (arg.rfind("--emit=", 0) == 0) {
            emit = arg.substr(7);
        }
        else if (arg.rfind("--format=", 0) == 0) {
            try {
                applyFormat(machine, arg.substr(9));
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return 1;
            }
        }
        else if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 0;
        }
        else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Error: unknown option '" << arg << "'" << std::endl;
            usage(argv[0]);
            return 1;
        }
        else {
            path = arg;
        }
    }

    if (path.empty()) {
        std::cerr << "Error: no input file" << std::endl;
        usage(argv[0]);
        return 1;
    }
    if (emit != "ast" && emit != "resolved" && emit != "tir" && emit != "mir") {
        std::cerr << "Error: unknown stage '" << emit << "'" << std::endl;
        return 1;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Error: could not open file '" << path << "'" << std::endl;
        return 1;
    }
    std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(file),
                                    std::istreambuf_iterator<char>()};
    file.close();

    try {
        Program program{parse(bytes, machine)};
        if (emit == "ast") {
            printAST(program);
            return 0;
        }

        SemaResult result{analyze(program)};
        if (!result.errors.empty()) {
            for (const std::string& error : result.errors) {
                std::cerr << "Error: " << error << std::endl;
            }
            return 1;
        }
        if (emit == "resolved") {
            printResolved(result);
            return 0;
        }

        std::vector<TirFunction> tir{lowerToTir(result)};
        if (emit == "tir") {
            printTir(tir, machine);
            return 0;
        }

        printMir(lowerToMir(tir, machine), machine);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
