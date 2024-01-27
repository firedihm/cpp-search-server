#pragma once

#include "search_server.h"

#include <set>
#include <vector>

{
    using namespace std;
    
    string ReadLine();
    int ReadLineWithNumber();
    
    bool IsValidWord(const string& word);
    bool IsStopWord(const string& word, const SearchServer* search_server);
    
    vector<string> SplitIntoWords(const string& text);
    vector<string> SplitIntoWordsNoStop(const string& text, const SearchServer* search_server);
    
    template <typename StringContainer>
    set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
        set<string> non_empty_strings;
        for (const string& str : strings) {
            if (!str.empty() && IsValidWord(str)) {
                non_empty_strings.insert(str);
            }
        }
        return non_empty_strings;
    }
}
