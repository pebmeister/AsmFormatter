#include <sstream>
#include <iostream>
#include <string>
#include <fstream>

#include "AsmFormatter.h"

std::string ltrim(const std::string& s);
std::string rtrim(const std::string& s);
std::string trim(const std::string& s);

AsmFormatter::AsmFormatter(const FormatOptions& options)
	: m_options(options)
{
}

bool AsmFormatter::load()
{
	std::ifstream file(m_options.inputfile);
	std::ofstream outfile(m_options.outputfile);

	if (!file)
	{
		outfile.close();
		return false;
	}

	std::string line;
	auto blanklines = 0;
	while (std::getline(file, line))
	{
		auto fmat = parseLine(line);
		m_column = 0;
		if (fmat.blankLine)
		{
			if (++blanklines == 1) 
			{
				outfile << "\n";
			}
			continue;
		}

		blanklines = 0;
		if (fmat.commentOnly)
		{
			printAtOffset(outfile, fmat.comment, line[0] == ';' ? 0 : m_options.opcodeColumn);
			outfile << "\n";
			continue;
		}

		if (!fmat.label.empty())
		{
			printAtOffset(outfile, fmat.label, 0);
			if (m_options.requireColon && fmat.label[fmat.label.size() -1] != ':')
			{
				outfile << ":";
				m_column += 1;
			}

			if (m_options.labelOwnLine)
			{
				outfile << "\n";
				m_column = 0;
				if (fmat.opcode.empty() &&
				        fmat.operand.empty() &&
				        fmat.comment.empty())
				{
					continue;
				}
			}
		}
		printAtOffset(outfile, fmat.opcode, m_options.opcodeColumn);
		printAtOffset(outfile, fmat.operand, m_options.operandColumn);
		printAtOffset(outfile, fmat.comment, m_options.commentColumn);

		outfile << "\n";
	}
	outfile.close();
	return true;
}

void AsmFormatter::printAtOffset(std::ofstream& outputfile, const std::string& string, const int position)
{
	if (string.empty())
	{
		return;
	}

	if (position > 0)
	{
		do
		{
			outputfile << " ";
			m_column++;
		} while (m_column < position);
	}
	outputfile << string;
	m_column += string.size();
}

AsmFormatter::AsmLine AsmFormatter::parseLine(const std::string& rawLine) const 
{
    AsmLine line;

    // 1. Handle completely blank lines early
    if (trim(rawLine).empty()) {
        line.blankLine = true;
        return line;
    }

    // 2. Separate the comment from the rest of the code safely
    size_t commentIdx = std::string::npos;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;

    for (size_t i = 0; i < rawLine.length(); ++i) {
        char ch = rawLine[i];

        // Toggle quote states, ensuring they don't interfere with each other
        if (ch == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
        } else if (ch == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
        }
        // If we hit a semicolon outside of any quotes, we found the true comment start
        else if (ch == ';' && !inSingleQuote && !inDoubleQuote) {
            commentIdx = i;
            break;
        }
    }

    std::string codePart = rawLine;
    if (commentIdx != std::string::npos) {
        line.comment = rawLine.substr(commentIdx); // Keep the ';'
        codePart = rawLine.substr(0, commentIdx);
    }

    // 3. Check for comment-only line
    if (trim(codePart).empty()) {
        line.commentOnly = true;
        return line;
    }

    // 4. Extract Column 1 (Label, Directive, or Equate Constant Name)
    size_t idx = 0;
    std::string col1 = "";
    
    // Check if the very first character is text (not whitespace)
    if (!codePart.empty() && !std::isspace(static_cast<unsigned char>(codePart[0]))) {
        // Read until whitespace to get the first token
        while (idx < codePart.length() && !std::isspace(static_cast<unsigned char>(codePart[idx]))) {
            ++idx;
        }
        col1 = codePart.substr(0, idx);
    }

    // Advance idx to look ahead and find the second token (checking for .equ or =)
    size_t lookAheadIdx = idx;
    while (lookAheadIdx < codePart.length() && std::isspace(static_cast<unsigned char>(codePart[lookAheadIdx]))) {
        ++lookAheadIdx;
    }

    std::string nextToken = "";
    if (lookAheadIdx < codePart.length()) {
        size_t tokenEnd = lookAheadIdx;
        while (tokenEnd < codePart.length() && !std::isspace(static_cast<unsigned char>(codePart[tokenEnd]))) {
            ++tokenEnd;
        }
        nextToken = codePart.substr(lookAheadIdx, tokenEnd - lookAheadIdx);
    }

    // 5. Categorize based on look-ahead context
    if (!col1.empty()) {
        if (col1[0] == '.') {
            line.directive = true;
            line.opcode = col1;
        } 
        // Equate detection: if next token is an equate directive, col1 is the constant name (opcode)
        else if (nextToken == ".equ" || nextToken == "=" || col1 == "=") {
            line.opcode = col1; 
        } 
        else {
            line.label = col1;
        }
    }

    // 6. Parse Opcode and Operand from the remaining code string
    codePart = rtrim(codePart);

    if (idx < codePart.length()) {
        std::string remaining = ltrim(codePart.substr(idx));

        if (line.opcode.empty()) {
            // Standard path: Extract opcode and then operand
            size_t opEnd = 0;
            while (opEnd < remaining.length() && !std::isspace(static_cast<unsigned char>(remaining[opEnd]))) {
                ++opEnd;
            }
            line.opcode = remaining.substr(0, opEnd);

            if (opEnd < remaining.length()) {
                line.operand = ltrim(remaining.substr(opEnd));
            }
        } else {
            // Equate or Directive path: 
            // The first token is already isolated in line.opcode. 
            // The entire rest of the line (e.g., ".equ 0x05" or "= 10") becomes the operand.
            line.operand = remaining;
        }
    }

    // 7. Clean up fields
    line.label = trim(line.label);
    line.opcode = trim(line.opcode);
    line.comment = rtrim(line.comment);

    return line;
}

std::string ltrim(const std::string& s)
{
	// Added \n, \r, \f, and \v
	size_t pos = s.find_first_not_of(" \t\n\r\f\v");

	if (pos == std::string::npos)
	{
		return "";
	}

	return s.substr(pos);
}

std::string rtrim(const std::string& s)
{
	// Added \n, \r, \f, and \v
	size_t pos = s.find_last_not_of(" \t\n\r\f\v");

	if (pos == std::string::npos)
	{
		return "";
	}

	return s.substr(0, pos + 1);
}

std::string trim(const std::string& s)
{
	return rtrim(ltrim(s));
}
