#include "search_server.h"

#include <numeric>

const std::set<std::string>& SearchServer::GetStopWords() const {
    return stop_words_;
}

size_t SearchServer::GetDocumentCount() const {
    return documents_.size();
}

int SearchServer::GetDocumentId(uint n) const {
    return document_ids_.at(n);
}

bool SearchServer::IsValidWord(const std::string& word) {
    return none_of(word.begin(), word.end(), [](char c) { return c >= '\0' && c < ' '; });
}

bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) != 0;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {
    using namespace std::string_literals;
    
    if (document_id < 0 || documents_.count(document_id) != 0) {
        throw std::invalid_argument("document id "s + std::to_string(document_id) + " is invalid or already exists"s);
    }
    
    std::vector<std::string> words = ParseDocument(document);
    /*
    не проходит тесты, хотя решение хорошее...
    if (words.empty()) {
        throw std::invalid_argument("document id "s + std::to_string(document_id) + " has no valid plus words"s);
    }
    */
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
    }
    
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.push_back(document_id);
}

std::vector<std::string> SearchServer::ParseDocument(const std::string& text) const {
    using namespace std::string_literals;
    
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("invalid word "s + word + " was passed to document"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

std::tuple<std::vector<std::string>, DocumentStatus> SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    Query query = ParseQuery(raw_query);
    
    std::vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return {matched_words, documents_.at(document_id).status};
}
