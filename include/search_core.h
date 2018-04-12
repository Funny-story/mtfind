/** 
 *  @file    search_core.h
 *  @author  FedorovVD
 *  @date    9.04.2018  
 *  @version 1.0 
 *  
 *  @brief Implements multi-threading search in file
 *
 *  @section DESCRIPTION
 *  
 *  This class reads the file that is specified in the first parameter
 *  and and performs a search in this file using the mask passed by the second parameter.
 *  Features:
 *   - input file only in ASCII;
 *   - case-sensitive search; 
 *   - no duplicates;
 *   - an entry can not include a newline character. The mask can not contain a newline character;
 *   - multi-threading;
 *   - use c++11;
 *   - no boost.
 */

#ifndef SEARCH_CORE_H
#define SEARCH_CORE_H

#include <string>

namespace ares_test
{
class SearchCorePrivate;

/**
  *  @brief Class that implements multi-threading search in text file with encode ASCII 7-bit.  
  */ 
class SearchCore
{
public:
    /// Default constructor 
    SearchCore(const std::string &_fileName = "", const std::string &_mask = ""); 
    // Begin non copiable section:
    SearchCore(const SearchCore &_second) = delete;
    SearchCore &operator = (const SearchCore &_second) = delete;
    // End non copiable section
    /// Destructor
    ~SearchCore();
    /// Change file name for using with different files
    void setFileName(const std::string &_fileName);
    /// Change mask for using with different masks
    void setMask(const std::string &_mask);
    /// Main method which implements multi-threading search
    int search() const;
    /// Get result string after search
    std::string result() const;

private:
    SearchCorePrivate *m_impl;  ///< pimpl object
}; // end of class SearchCore

}// ares_test
#endif //SEARCH_CORE_H

// vim:ts=4:sts=4:sw=4:et:
