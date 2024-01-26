#pragma once

#include <iostream>
#include <set>
#include <vector>

{
    using namespace std;
    
    string ReadLine();
    int ReadLineWithNumber();
    
    vector<string> SplitIntoWords(const string& text);
    //template <typename StringContainer> set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings);
}
