#include "string_processing.h"

{
    using namespace std;
    
    string ReadLine() {
        string s;
        getline(cin, s);
        return s;
    }
    
    int ReadLineWithNumber() {
        int result;
        cin >> result;
        ReadLine();
        return result;
    }
    
    vector<string> SplitIntoWords(const string& text) {
        vector<string> words;
        string word;
        for (const char c : text) {
            if (c == ' ') {
                if (!word.empty()) {
                    words.push_back(word);
                    word.clear();
                }
            } else {
                word += c;
            }
        }
        if (!word.empty()) {
            words.push_back(word);
        }
        
        return words;
    }
    
    template <typename StringContainer>
    set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
        set<string> non_empty_strings;
        for (const string& str : strings) {
            if (!IsValidWord(str)) {
                throw invalid_argument("stop words contain illegal characters"s);
            }
            
            if (!str.empty()) {
                non_empty_strings.insert(str);
            }
        }
        return non_empty_strings;
    }
}
