#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c != ' ') {
            word += c;
        } else if (!word.empty()) {
            words.push_back(word);
            word.clear();
        }
    }
    if (!word.empty()) {
        words.push_back(word);
	}
    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string word : SplitIntoWords(text)) {
            stop_words_.insert(word);
		}
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        
        //рассчитаем вес слова в текущем документе
        int N = words.size();
        double freq = 1.0 / N;
        
        //слова добавляем в словарь и привязываем к ним текущий документ, рассчитываем TF
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += freq;
        }
		
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        auto matched_documents = FindAllDocuments(ParseQuery(raw_query));
        
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
		
        return matched_documents;
    }

private:
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
			}
        }
        return words;
    }

    struct Query {
        set<string> normal_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& raw_query) const {
        set<string> split_query;
        for (const string& word : SplitIntoWordsNoStop(raw_query)) {
            split_query.insert(word);
        }
		
        Query query;
        for (const string& word : split_query) {
            if (word[0] == '-') {
                query.minus_words.insert(word.substr(1));
            } else {
                query.normal_words.insert(word);
			}
        }
        return query;
    }
    
    double CalculateIDF(const string& word) const {
        double N = static_cast<double>(word_to_document_freqs_.at(word).size());
        return log(document_count_ / N);
    }
    
    vector<Document> FindAllDocuments(const Query& query) const {
        //вычислим релевантность всех релевантных документов
        map<int, double> document_relevancies;
        for (const string& word : query.normal_words) {
            if (!word_to_document_freqs_.count(word)) {
                continue;
            }
            
            double idf = CalculateIDF(word);
            for (const auto& [document_id, tf] : word_to_document_freqs_.at(word)) {
                document_relevancies[document_id] += tf * idf;
			}
        }
        
        //отбракуем результаты, содержащие минус-слова
        //забыл про это в предыдущем комите, и никто не заметил :)
        for (const string& word : query.minus_words) {
            if (!word_to_document_freqs_.count(word)) {
                continue;
            }
            
            //итерируем по документам, в которых встречается очередное минус-слово
            for (const auto& [document_id, tf] : word_to_document_freqs_.at(word)) {
                document_relevancies.erase(document_id);
            }
        }
        
        //переведём в вектор для дальнейшего .resize()
        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_relevancies) {
            matched_documents.push_back({document_id, relevance});
		}
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
	}
    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", " << "relevance = "s << relevance << " }"s << endl;
    }
}
