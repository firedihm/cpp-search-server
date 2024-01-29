#pragma once

#include <iostream>
#include <string>
#include <vector>

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

std::ostream& operator<<(std::ostream& out, const DocumentStatus& status);
void PrintMatchDocumentResult(int id, const std::vector<std::string>& words, DocumentStatus status);

struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    
    Document() = default;
    Document(int id, double relevance, int rating) : id(id), relevance(relevance), rating(rating) {}
};

std::ostream& operator<<(std::ostream& os, const Document& document);
void PrintDocument(const Document& document);
