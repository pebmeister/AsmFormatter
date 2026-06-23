#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

#include "AsmFormatter.h"


static void print_usage(const char* prog) {
    std::cout << "Usage: " << prog << " [options] [inputfile [outputfile]]\n"
              << "Options (position-independent):\n"
              << "  --require-colon / --no-require-colon\n"
              << "  --label-own-line / --no-label-own-line\n"
              << "  --opcode-column <n>    (integer > 0)\n"
              << "  --operand-column <n>   (integer > 0)\n"
              << "  --comment-column <n>   (integer > 0)\n"
              << "  --input-file <path>\n"
              << "  --output-file <path>\n"
              << "  -h, --help             show this help\n";
}

static bool parse_positive_int(const char* s, int &out) {
    if (!s || *s == '\0') return false;
    char* end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (*end != '\0') return false;
    if (v <= 0) return false;
    out = static_cast<int>(v);
    return true;
}

FormatOptions parse_args(int argc, char** argv) {
    FormatOptions opts;
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) args.emplace_back(argv[i]);

    std::vector<std::string> positional;
    bool input_set = false;
    bool output_set = false;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];

        if (a == "-h" || a == "--help") {
            print_usage(argv[0]);
            std::exit(0);
        } else if (a == "--require-colon") {
            opts.requireColon = true;
        } else if (a == "--no-require-colon") {
            opts.requireColon = false;
        } else if (a == "--label-own-line") {
            opts.labelOwnLine = true;
        } else if (a == "--no-label-own-line") {
            opts.labelOwnLine = false;
        } else if (a == "--opcode-column" || a == "--operand-column" || a == "--comment-column") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: missing value for " << a << "\n";
                print_usage(argv[0]);
                std::exit(2);
            }
            int val;
            if (!parse_positive_int(args[++i].c_str(), val)) {
                std::cerr << "Error: invalid positive integer for " << a << ": " << args[i] << "\n";
                std::exit(2);
            }
            if (a == "--opcode-column") opts.opcodeColumn = val;
            else if (a == "--operand-column") opts.operandColumn = val;
            else if (a == "--comment-column") opts.commentColumn = val;
        } else if (a == "--input-file") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: missing value for " << a << "\n";
                print_usage(argv[0]);
                std::exit(2);
            }
            opts.inputfile = args[++i];
            input_set = true;
        } else if (a == "--output-file") {
            if (i + 1 >= args.size()) {
                std::cerr << "Error: missing value for " << a << "\n";
                print_usage(argv[0]);
                std::exit(2);
            }
            opts.outputfile = args[++i];
            output_set = true;
        } else if (!a.empty() && a[0] == '-') {
            std::cerr << "Unknown option: " << a << "\n";
            print_usage(argv[0]);
            std::exit(2);
        } else {
            positional.push_back(a);
        }
    }

    // If --input-file/--output-file not provided, take positional args (position-independent overall)
    if (!input_set && positional.size() >= 1) opts.inputfile = positional[0];
    if (!output_set && positional.size() >= 2) opts.outputfile = positional[1];

    return opts;
}

int main(int argc, char* argv[])
{
    FormatOptions opts = parse_args(argc, argv);

    if (opts.inputfile.empty()) {
        std::cout << "No input file specified.\n";
        print_usage(argv[0]);
        std::exit(1);
    }
    if (opts.outputfile.empty()) {
        std::cout << "No outputfile file specified.\n";
        print_usage(argv[0]);
        std::exit(1);
    }
    
    AsmFormatter formatter(opts);

    if (!formatter.load())
    {
        std::cout << "Unable to open file.\n";
        return 1;
    }

    return 0;
}
