#include "search_server.h"

#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

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
