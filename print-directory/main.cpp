#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

struct EntryInfo {
    path path_;
    filesystem::file_type type_;
};

void PrintTree(ostream& dst, const path& p, int offset) {
    dst << string(offset, ' ') << p.filename().string() << "\n";

    vector<EntryInfo> dir_data;
    for (const auto & dir_entry : filesystem::directory_iterator(p)) {
        dir_data.emplace_back(dir_entry.path(), dir_entry.status().type());
    }

    sort(dir_data.begin(), dir_data.end(), [](const EntryInfo& lhs, const EntryInfo& rhs) {
        return (lhs.type_ > rhs.type_) ||
               ((lhs.type_ == rhs.type_) && (lhs.path_.filename().string() > rhs.path_.filename().string()));
    });

    for (const auto& item : dir_data) {
        if (item.type_ == filesystem::file_type::directory) {
            PrintTree(dst, item.path_, offset + 2);
        } else {
            dst << string(offset + 2, ' ') << item.path_.filename().string() << "\n";
        }
    }
}

void PrintTree(ostream& dst, const path& p) {
    PrintTree(dst, p, 0);
}

int main() {
    error_code err;
    filesystem::remove_all("test_dir", err);
    filesystem::create_directories("test_dir"_p / "a"_p, err);
    filesystem::create_directories("test_dir"_p / "b"_p, err);

    ofstream("test_dir"_p / "b"_p / "f1.txt"_p);
    ofstream("test_dir"_p / "b"_p / "f2.txt"_p);
    ofstream("test_dir"_p / "a"_p / "f3.txt"_p);

    ostringstream out;
    PrintTree(out, "test_dir"_p);
    assert(out.str() ==
        "test_dir\n"
        "  b\n"
        "    f2.txt\n"
        "    f1.txt\n"
        "  a\n"
        "    f3.txt\n"s
    );
}
