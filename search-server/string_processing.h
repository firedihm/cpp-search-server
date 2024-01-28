#pragma once

#include <set>
#include <string>
#include <vector>

std::string ReadLine();
int ReadLineWithNumber();

bool IsValidWord(const std::string& word);

std::vector<std::string> SplitIntoWords(const std::string& text);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty() && IsValidWord(str)) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}
