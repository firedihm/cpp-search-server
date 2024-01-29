#include "document.h"

std::ostream& operator<<(std::ostream& os, const DocumentStatus& status) {
    using namespace std::string_literals;
    
    switch (status) {
        case DocumentStatus::ACTUAL:
            os << "ACTUAL"s;
            break;
        case DocumentStatus::IRRELEVANT:
            os << "IRRELEVANT"s;
            break;
        case DocumentStatus::BANNED:
            os << "BANNED"s;
            break;
        case DocumentStatus::REMOVED:
            os << "REMOVED"s;
            break;
    }
    return os;
}

void PrintMatchDocumentResult(int id, const std::vector<std::string>& words, DocumentStatus status) {
    using namespace std;
    
    cout << "{ "s
         << "document_id = "s << id << ", "s
         << "status = "s << status << ", "s
         << "words ="s;
         for (const auto& word : words) {
             cout << ' ' << word;
         }
    cout << "}"s << endl;
}

std::ostream& operator<<(std::ostream& os, const Document& document) {
    using namespace std::string_literals;
    
    os << "{ "s
       << "document_id = "s << document.id << ", "s
       << "relevance = "s << document.relevance << ", "s
       << "rating = "s << document.rating
       << " }"s;
    return os;
}

void PrintDocument(const Document& document) {
    using namespace std;
    
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}
