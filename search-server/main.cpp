#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int    MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

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

//существует только для возвратных значений FindTopDocuments и FindAllDocuments
struct Document {
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
    
    Document() = default;
    Document(int document_id, double relevance, int rating) : id(document_id), relevance(relevance), rating(rating) {}
};

ostream& operator<<(ostream& os, const Document& document) {
    os << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating
       << " }"s;
    
    return os;
}

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator begin, Iterator end, size_t page_size) {
        size_t docs_left = distance(begin, end);
        while (docs_left) {
            size_t docs_per_current_page = min(page_size, docs_left);
            pages_.emplace_back(begin, begin + docs_per_current_page);
            
            begin += docs_per_current_page;
            docs_left -= docs_per_current_page;
        }
    }
    
    auto   begin() const { return pages_.begin(); }
    auto   end()   const { return pages_.end(); }
    bool   empty() const { return pages_.empty(); }
    size_t size()  const { return pages_.size(); }
    
private:
    struct IteratorRange {
        Iterator begin;
        Iterator end;
        
        IteratorRange(Iterator begin, Iterator end) : begin(begin), end(end) {}
        
        friend ostream& operator<<(ostream& os, const IteratorRange& range) {
            for (auto it = range.begin; it != range.end; ++it) {
                //помним, что it указывает на Document
                os << *it;
            }
            return os;
        }
    };
    
    vector<IteratorRange> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(c.begin(), c.end(), page_size);
}

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words) : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {}
    explicit SearchServer(const string &stop_words) : SearchServer(SplitIntoWords(stop_words)) {}
    
    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        CheckDocumentInput(document_id, document);
        
        vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.push_back(document_id);
    }
    
    //вызов без явно переданного статуса или предиката = вызов с предикатом по умолчанию
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus sought_status = DocumentStatus::ACTUAL) const {
        return FindTopDocuments(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                               return status == sought_status;
                                           });
    }
    
    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate predicate) const {
        vector<Document> matched_documents = FindAllDocuments(ParseQuery(raw_query), predicate);
        
        sort(matched_documents.begin(), matched_documents.end(), [](const Document& lhs, const Document& rhs) {
             if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
                 return lhs.rating > rhs.rating;
             }
             return lhs.relevance > rhs.relevance;
        });
        
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }
    
    int GetDocumentCount() const {
        return documents_.size();
    }
    
    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
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
    
    int GetDocumentId(uint n) const {
        return document_ids_.at(n);
    }
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> document_ids_;
    
    static bool IsValidWord(const string& word) {
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
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
    
    void CheckDocumentInput(int document_id, const string& document) {
        if (document_id < 0) {
            throw invalid_argument("document's id can't be negative"s);
        }
        if (documents_.count(document_id) > 0) {
            throw invalid_argument("document with specified id already exists"s);
        }
        if (document.empty()) {
            throw invalid_argument("document is empty"s);
        }
    }
    
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("document contains illegal characters"s);
            }
            
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }
    
    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }
    
    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        
        //если после отрубания минуса осталась пустота или ещё один минус или слово содержит спецсимволы
        if (text.empty() || text[0] == '-' || !IsValidWord(text)) {
            throw invalid_argument("query contains illegal characters"s);
        }
        
        return {text, is_minus, IsStopWord(text)};
    }
    
    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }
    
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    
    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query, DocumentPredicate predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
        
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
        
        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.emplace_back(document_id, relevance, documents_.at(document_id).rating);
        }
        return matched_documents;
    }
};

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server)  : search_server_(search_server), no_result_requests_(0) {}
    
    //вызов без явно переданного статуса или предиката = вызов с предикатом по умолчанию
    vector<Document> AddFindRequest(const string& raw_query, DocumentStatus sought_status = DocumentStatus::ACTUAL) {
        return AddFindRequest(raw_query, [=](int document_id, DocumentStatus status, int rating) {
                                                  return status == sought_status;
                                              });
    }
    
    template <typename DocumentPredicate>
    vector<Document> AddFindRequest(const string& raw_query, DocumentPredicate predicate) {
        vector<Document> result = search_server_.FindTopDocuments(raw_query, predicate);
        
        //избавляемся от записей, старше суток
        if (requests_.size() == MIN_IN_DAY) {
            if (requests_.front().empty()) {
                --no_result_requests_;
            }
            requests_.pop();
        }
        
        requests_.emplace(raw_query, result.size());
        
        if (result.empty()) {
            ++no_result_requests_;
        }
        
        return result;
    }
    
    uint GetNoResultRequests() const {
        return no_result_requests_;
    }
    
private:
    const static size_t MIN_IN_DAY = 1440;
    
    struct QueryResult {
        string query;
        bool is_empty;
        
        QueryResult(const string& query, int documents_found) : query(query), is_empty(!documents_found) {}
        
        bool empty() { return is_empty; }
    };
    
    const SearchServer& search_server_;
    queue<QueryResult> requests_;
    uint no_result_requests_;
};

// ==================== для примера =========================

int main() {
    SearchServer search_server("and in at"s);
    RequestQueue request_queue(search_server);
    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});
    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }
    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("curly dog"s);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("big collar"s);
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("sparrow"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    return 0;
}
