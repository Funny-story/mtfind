#include <iostream>
#include "search_core.h"

int main(int argc, char *argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--help")
    {
        std::cout << "Hello!\nThis is mtfind utility!\nRun me with two arguments:\n - file name;\n - mask to find.\nExample: mtfind input.txt \"?ad\"\nHave a nice day!" << std::endl;
        return 1;
    }
    if (argc < 3)
    {
        std::cout << "Low arguments. Example to run: mtfind input.txt \"?ad\"" << std::endl;
        return 1;
    } 
    if (argc > 3)
    {
        std::cout << "Many arguments. May be you forgot to enclose the mask in quotes. Example to run: mtfind input.txt \"?ad\""  << std::endl;
        return 1;
    }
    //check mask on \n
    if (std::string(argv[2]).find("\\n") != std::string::npos)
    {
        std::cout << "Mask should not contain \\n" << std::endl;
        return 1;
    }
   
    ares_test::SearchCore sc(argv[1], argv[2]);
    sc.setFileName(argv[1]);
    sc.setMask(argv[2]);
    if (sc.search() == -1 )
    	std::cout << "Search error" << std::endl;
    else	
        std::cout << sc.result() << std::endl;
    return 0;
}

// vim:ts=4:sts=4:sw=4:et:
