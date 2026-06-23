#ifndef ASMFORMATTER_H
#define ASMFORMATTER_H

#include <string>
#include <vector>

struct FormatOptions
{
    bool requireColon = true;
    bool labelOwnLine = true;

    int opcodeColumn = 12;
    int operandColumn = 20;
    int commentColumn = 40;

    std::string inputfile;
    std::string outputfile;
};

class AsmFormatter
{
public:
    explicit AsmFormatter(const FormatOptions& options);

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

    int m_column;
    FormatOptions m_options;
    AsmLine parseLine(const std::string& line) const;
    void printAtOffset(std::ofstream& outputfile, const std::string& string, int position);
};

#endif
