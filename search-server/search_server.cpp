#include "search_server.h"

#include <cmath>
#include <numeric>

{
    using namespace std;
    
    const set<string>& SearchServer::GetStopWords const {
        return stop_words_;
    }
    
    size_t SearchServer::GetDocumentCount const {
        return documents_.size();
    }
    
    int SearchServer::GetDocumentId(uint n) const {
        if (n > document_ids_.size()) {
            return -1;
        }
        return document_ids_.at(n);
    }
    
    int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }
    
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    
    //лучше возвращать коды ошибок
    bool SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        if (document_id < 0) {
            cerr << "couldn't add document "s << document_id << ": negative id"s << endl;
            return false;
        }
        
        if (documents_.count(document_id) != 0) {
            cerr << "couldn't add document "s << document_id << ": id already exists"s << endl;
            return false;
        }
        
        vector<string> words = SplitIntoWordsNoStop(document, this);
        if (words.empty()) {
            cerr << "couldn't add document "s << document_id << ": document has no valid plus words"s << endl;
            return false;
        }
        
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.push_back(document_id);
        return true;
    }
    
    tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const {
        Query query = ParseQuery(raw_query);
        
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        
        for (const string& word : query.minus_words) {
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
    
}