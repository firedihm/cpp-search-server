#pragma once

{
    using namespace std;
    
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
}
