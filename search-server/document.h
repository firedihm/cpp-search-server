#pragma once

#include <iostream>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    
    Document() = default;
    Document(int document_id, double relevance, int rating) : id(document_id), relevance(relevance), rating(rating) {}
    
    std::ostream& operator<<(std::ostream& os) const;
};
