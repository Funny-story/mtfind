#include "search_core.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <list>
#include <set>
#include <regex>
#include <algorithm>
#include <cstring>
#include <mutex>

namespace ares_test
{

enum READ_FILE_RESULT
{
    SUCCESS = 1,
    NOT_EXISTS,
    NOT_ASCII,
    VERY_LARGE
};

static const size_t THREADS_COUNT = std::thread::hardware_concurrency() - 1 < 4 ? 4 : std::thread::hardware_concurrency() - 1;

struct SearchResultLine
{
    SearchResultLine(): 
        lineNumber(0), 
        startWith(0)
    {}
    size_t lineNumber;
    size_t startWith;
    std::string text;
};

class SearchCorePrivate
{
public:
    SearchCorePrivate() 
    {
        threads.reserve(THREADS_COUNT); 
    }
    ~SearchCorePrivate()
    {
        for (auto it = threads.begin(); it != threads.end();)
        {
            if (it->joinable())
                it->join();
        }       
    }
    bool fileExists(const std::string &_fileName)
    {
        std::ifstream s(_fileName.c_str());
        return s.is_open();
    }
    READ_FILE_RESULT readFile()
    {
        if (fileExists(fileName))
        {
            char c;
            size_t size = 0;
            size_t maxFileSize = 1024 * 1024 * 1024;
            std::ifstream in(fileName);
            while (in.eof() == 0) 
            {
                in.get(c);
                //check ASCII 7-bit
                if (c < 0 || c > 127)
                {
                    text = "";
                    return READ_FILE_RESULT::NOT_ASCII;
                }
                text += c;
                ++size;
                if (size == maxFileSize)
                {
                    text = "";
                    return READ_FILE_RESULT::VERY_LARGE;
                }
            }
            return READ_FILE_RESULT::SUCCESS;
        }
        return READ_FILE_RESULT::NOT_EXISTS;
    }
    static bool sortResult(const SearchResultLine &_first, const SearchResultLine &_second)
    {
        return _first.lineNumber < _second.lineNumber;
    }
    void uniqueResult()
    {
        std::set<std::string> exists; 
        std::list<SearchResultLine> tmp;
        for (const auto &r : result)
        {
            if (exists.insert(r.text).second)
                tmp.push_back(r);
        }
        result = tmp;
    }
    static void search(const int _lineStart, const int _startPosInLine, std::string _partOfText, const std::string _mask, std::list<SearchResultLine> *_result)
    {   
        std::list<SearchResultLine> localResult;
         
        std::regex rx(_mask);
        std::smatch m;
		int newRowCount = 0;
        while (std::regex_search(_partOfText, m, rx))
        {
            for (auto x : m)
            {  
                SearchResultLine srl;
                newRowCount += std::count(_partOfText.begin(), _partOfText.begin()+m.position(), '\n');
                std::string tmp = _partOfText.substr(0, m.position()); 
                int lastNewRowPos = tmp.find_last_of('\n');
                srl.lineNumber = _lineStart + newRowCount;
                srl.startWith = lastNewRowPos == std::string::npos ? _startPosInLine + m.position() : m.position() - lastNewRowPos;
                srl.text = x;
                localResult.push_back(srl);
            } 
            _partOfText = m.suffix().str();
        }
         
        if (localResult.size())
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto r : localResult)
                _result->push_back(r);
        }
    }
    static void control(std::vector<std::thread> *_threads)
    {
        for (auto it = _threads->begin(); it != _threads->end(); ++it)
        {
            if (it->joinable())
                it->join();
        }       
        _threads->clear();
    }
    std::string fileName;
    std::string mask;
    std::string text;
    std::list<SearchResultLine> result;
    std::vector<std::thread> threads;
    static std::mutex mutex;
};

std::mutex SearchCorePrivate::mutex;

SearchCore::SearchCore(const std::string &_fileName, const std::string &_mask):
    m_impl(new SearchCorePrivate())
{
    setFileName(_fileName);
    setMask(_mask);
}

SearchCore::~SearchCore()
{ 
    delete m_impl;
    m_impl = NULL;
}

void SearchCore::setFileName(const std::string &_fileName)
{
    m_impl->fileName = _fileName;
}
 
void SearchCore::setMask(const std::string &_mask)
{
    m_impl->mask = _mask;
    //preparing for regex
    std::string dot = ".";
    std::string shieldingDot = "\\.";
    for(int pos=0; (pos = m_impl->mask.find(dot, pos)) != std::string::npos; pos += 2)
        m_impl->mask.replace(pos, 1, shieldingDot);
    std::replace(m_impl->mask.begin(), m_impl->mask.end(), '?', '.'); 
}

int SearchCore::search() const
{
    m_impl->result.clear();
    READ_FILE_RESULT r = m_impl->readFile();
    if (r == READ_FILE_RESULT::NOT_EXISTS)
    {
        std::cout << "SearchCore::search(): " << "file not exist" << std::endl;
        return -1;
    }
    else if (r == READ_FILE_RESULT::NOT_ASCII)
    {
        std::cout << "SearchCore::search(): " << "file not in ASCII" << std::endl;
        return -1;
    }
    else if (r == READ_FILE_RESULT::VERY_LARGE)
    {
        std::cout << "SearchCore::search(): " << "file is very large. Maximum file size = 1Gb" << std::endl;
        return -1; 
    }
    if (m_impl->mask.length() > 100)
    {
        std::cout << "SearchCore::search(): " << "mask is very large. Maximum mask length = 100" << std::endl;
        return -1;    
    }
    if (m_impl->mask.length() > m_impl->text.length())
    {
        std::cout << "SearchCore::search(): " << "oooo, the mask is very large in relation to the text" << std::endl;
        return -1;    
    }
    int maskSize = m_impl->mask.length();
    int partSize = m_impl->text.length()/THREADS_COUNT;
    //correct parts for mask size
    if (partSize < maskSize)
        partSize = maskSize;
    int startWith = 0; 
    int lineCounter = 1;
    int startPosInLine = 1;
    int lastNewRowPos = 0;
    int leftBorder = 0;
    int rightBorder = 0;
    size_t textLength = m_impl->text.length();
    for (size_t i=0; startWith < textLength && i <= THREADS_COUNT; ++i)
    {
        leftBorder = startWith-maskSize >= 0 ? startWith-maskSize : 0;
        rightBorder = i==THREADS_COUNT-1 ? textLength-1 : startWith+partSize <= textLength-1 ? startWith+partSize : textLength-1;;
        std::string partOfText = m_impl->text.substr(leftBorder, rightBorder-leftBorder);
        std::string tmp = m_impl->text.substr(lastNewRowPos, leftBorder-lastNewRowPos>=0 ? leftBorder-lastNewRowPos : 0); 
        int lastNewRowPosInPart = tmp.find_last_of('\n');
        if (lastNewRowPosInPart != std::string::npos)
            lastNewRowPos += lastNewRowPosInPart;
        startPosInLine = leftBorder-lastNewRowPos > 0 ? leftBorder-lastNewRowPos : 1;
	std::thread t(SearchCorePrivate::search, lineCounter, startPosInLine, partOfText, m_impl->mask, &m_impl->result);
        m_impl->threads.push_back(std::move(t));
        std::string partWithoutMask = partOfText.substr(0, partOfText.length()-maskSize); 
        lineCounter += std::count(partWithoutMask.begin(), partWithoutMask.end(), '\n');
        startWith += partSize;
    }
    
    std::thread threadController(SearchCorePrivate::control, &m_impl->threads);
    threadController.join();
    
    m_impl->result.sort(SearchCorePrivate::sortResult);
    m_impl->uniqueResult(); 
    return m_impl->result.size();
}

std::string SearchCore::result() const
{
    std::string resultStr = std::to_string(m_impl->result.size()) + "\n";
    for (auto r : m_impl->result)
        resultStr += std::to_string(r.lineNumber) + " " + std::to_string(r.startWith) + " " + r.text + "\n";
    return resultStr;
}

}// ares_test

// vim:ts=4:sts=4:sw=4:et:
