#pragma once

#include <fstream>
#include <string>

inline bool DecodeRLE(const std::string& src_name, const std::string& dst_name) {
    using namespace std;
    
    ifstream fin(src_name, ios::in | ios::binary);
    if (!fin) {
        return false;
    }
    
    ofstream fout(dst_name, ios::out | ios::binary);
    
    do {
        char buff[1024];
        unsigned char header = fin.get();
        if (!fin) {
            break;
        }
        
        int block_type = (header & 1);
        int data_size = (header >> 1) + 1;
        if (block_type) {
            string str(data_size, fin.get());
            fout.write(&str[0], data_size);
        } else {
            fin.read(buff, data_size);
            fout.write(buff, fin.gcount());
        }
    } while (fin);
    
    return true;
}
