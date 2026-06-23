#include <iostream>
#include "AsmFormatter.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cout << "Usage: formatter <file.asm>\n";
        return 1;
    }
    FormatOptions options;
   
    AsmFormatter formatter(argv[1], options);

    if (!formatter.load())
    {
        std::cout << "Unable to open file.\n";
        return 1;
    }

    return 0;
}
