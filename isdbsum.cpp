// https://trac.opensubtitles.org/projects/opensubtitles/wiki/HashSourceCodes
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

uint64_t compute_isdb_hash(std::istream& f)
{
    f.seekg(0, std::ios_base::end);
    auto const fsize = static_cast<uint64_t>(f.tellg());
    f.seekg(0);

    uint64_t const bytes_to_read = std::min(uint64_t{ 65536 }, fsize - (fsize % 8));
    std::vector<uint64_t> buffer(bytes_to_read / sizeof(uint64_t), 0);
    f.read(reinterpret_cast<char*>(buffer.data()), bytes_to_read);
    uint64_t const hash_acc = std::accumulate(begin(buffer), end(buffer), fsize);
    f.seekg(-65536, std::ios_base::end);
    buffer.assign(buffer.size(), 0);
    f.read(reinterpret_cast<char*>(buffer.data()), bytes_to_read);
    return std::accumulate(begin(buffer), end(buffer), hash_acc);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fmt::print("Usage: {} <filename>\n", argv[0]);
        return 0;
    }

    std::filesystem::path filepath(argv[1]);
    std::ifstream f(filepath, std::ios_base::binary);
    if (!f) {
        fmt::print(std::cerr, "Error opening file {}\n", filepath);
        return 1;
    }

    uint64_t const isdb_hash = compute_isdb_hash(f);
    fmt::print("{} - {:0>16X}\n", filepath.string(), isdb_hash);
    return 0;
 }
