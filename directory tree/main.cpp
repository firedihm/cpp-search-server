struct EntryInfo {
    std::filesystem::path path_;
    std::filesystem::file_type type_;
};

void PrintTree(ostream& dst, const path& p, std::uint32_t level) {
    std::string lead(level*2, ' ');
    dst << lead << p.filename().string() << "\n";

    std::vector<EntryInfo> directory_data;
    for (const auto & entry : std::filesystem::directory_iterator(p)) {
        directory_data.push_back({entry.path(), entry.status().type()});
    }

    std::sort(directory_data.begin(), directory_data.end(),
              [](const EntryInfo& lhs, const EntryInfo& rhs) {
        return (lhs.type_ > rhs.type_) ||
                ((lhs.type_ == rhs.type_) &&
                (lhs.path_.filename().string() > rhs.path_.filename().string()));
    });

    std::string lead_((level+1)*2, ' ');
    for (const auto& item : directory_data) {
        if (item.type_ == std::filesystem::file_type::directory) {
            PrintTree(dst, item.path_, level + 1);
        } else {
            dst << lead_ << item.path_.filename().string() << "\n";
        }
    }
}

void PrintTree(ostream& dst, const path& p) {
    PrintTree(dst, p, 0);
}
