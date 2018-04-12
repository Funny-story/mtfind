ABOUT
This is utility for multi-threading search in ASCII text file.
Features:
 - input file only in ASCII;
 - case-sensitive search; 
 - no duplicates;
 - an entry can not include a newline character. The mask can not contain a newline character;
 - multi-threading; 
 - use c++11;
 - no boost.

BUILD
cd build && cmake .. && make

RUN
Example
mtfind input.txt "?ad" 

HELP
mtfind --help
