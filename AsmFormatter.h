#ifndef ASMFORMATTER_H
#define ASMFORMATTER_H

#include <string>
#include <vector>

struct FormatOptions
{
    bool requireColon = true;
    bool labelOwnLine = true;

    bool upperOpcodes = true;
    bool upperDirectives = false;

    int opcodeColumn = 12;
    int operandColumn = 20;
    int commentColumn = 40;

    int tabSize = 4;

    bool expandTabs = true;
};

class AsmFormatter
{
public:
    explicit AsmFormatter(const std::string& filename, const FormatOptions& options);

    bool load();


private:

    struct AsmLine
    {
        std::string label;
        std::string opcode;
        std::string operand;
        std::string comment;

        bool blankLine  = false;
        bool commentOnly = false;
        bool directive = false;
    };

    std::string m_filename;
    FormatOptions m_options;
    AsmLine parseLine(const std::string& line) const;
};

#endif
