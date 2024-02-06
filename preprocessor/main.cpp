#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

void PrintError(const string& dest, const string& src, int line_index) {
    cout << "unknown include file "s << dest << " at file " << src << " at line "s << line_index << endl;
}

bool Preprocess(istream& input, ostream& output, const path& file_name, const vector<path>& include_directories) {
    static regex header_reg (R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static regex library_reg (R"/(\s*#\s*include\s*<([^>]*)>\s*)/");
    smatch match;
    int line_index = 0;
    
    for (string str; getline(input, str); ) {
        bool extern_header = false;
        path next_path;
        ++line_index;
        
        if (regex_match(str, match, header_reg)) {
            next_path = file_name.parent_path() / string(match[1]);
            if (filesystem::exists(next_path)) {
                ifstream fin(next_path, ios::in | ios::binary);
                if (fin) {
                    if (!Preprocess(fin, output, next_path, include_directories)) {
                        return false;
                    }
                    continue;
                } else {
                    PrintError(next_path.filename().string(), file_name.string(), line_index);
                    return false;
                }
            } else {
                extern_header = true;
            }
        }
        
        if (extern_header || regex_match(str, match, library_reg)) {
            bool found = false;
            for (const auto& dir : include_directories) {
                next_path = dir / string(match[1]);
                if (filesystem::exists(next_path)) {
                    ifstream fin(next_path.string(), ios::in);
                    if (fin) {
                        if (!Preprocess(fin, output, next_path, include_directories)) {
                            return false;
                        }
                        found = true;
                        break;
                    }
                    else {
                        PrintError(next_path.filename().string(), file_name.string(), line_index);
                        return false;
                    }
                }
            }
            if (!found) {
                PrintError(next_path.filename().string(), file_name.string(), line_index);
                return false;
            }
            continue;
        }
        
        //обычная строка
        output << str << endl;
    }
    
    return true;
}

bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    ifstream fin(in_file, ios::in | ios::binary);
    if (!fin) {
        return false;
    }
    
    ofstream fout(out_file, ios::out | ios::binary);
    return Preprocess(fin, fout, in_file, include_directories);
}

string GetFileContents(string file) {
    ifstream stream(file);
    
    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);
    
    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"sv;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"sv;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"sv;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"sv;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"sv;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"sv;
    }
    
    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));
    
    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"sv;
    
    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
}
