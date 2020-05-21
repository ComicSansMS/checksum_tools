
#include <boost/crc.hpp>
#include <boost/timer/progress_display.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <cassert>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <locale>
#include <optional>
#include <tuple>
#include <vector>

class ThousandsSeparated : public std::numpunct<char>
{
protected:
    char do_thousands_sep() const override { return ','; }
    std::string do_grouping() const override { return "\03"; }
};

std::optional<uint32_t> compute_crc(std::istream& fin, std::streamsize fsize)
{
    std::streamsize const BUFFER_SIZE = 4096;
    std::vector<char> buffer;
    buffer.resize(BUFFER_SIZE);
    std::streamsize bytes_read = 0;
    boost::crc_32_type crc_sum;
    boost::timer::progress_display progress(static_cast<unsigned long>(fsize / BUFFER_SIZE) + 1);
    if (fsize == 0) { ++progress; }
    while (bytes_read < fsize) {
        auto const chunk_size = std::min(BUFFER_SIZE, fsize - bytes_read);

        fin.read(buffer.data(), chunk_size);
        if (!fin) {
            fmt::print(std::cerr, "File read error.");
            return std::nullopt;
        }
        crc_sum.process_bytes(buffer.data(), chunk_size);

        bytes_read += chunk_size;
        ++progress;
    }
    assert(bytes_read == fsize);

    return crc_sum.checksum();
}

std::optional<uint32_t> processFile(std::filesystem::path const& filename)
{
    std::ifstream f(filename, std::ios_base::binary);
    if (!f) {
        fmt::print(std::cerr, "Error opening file {}\n", filename.string());
        return std::nullopt;
    }

    f.seekg(0, std::ios_base::end);
    std::streamsize const fsize = f.tellg();
    f.seekg(0, std::ios_base::beg);
    {
        std::locale::global(std::locale(std::locale::classic(), new ThousandsSeparated));
        fmt::print("\nComputing CRC32 of {} - {:n} bytes", filename.string(), fsize);
        std::locale::global(std::locale::classic());
    }

    return compute_crc(f, fsize);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        fmt::print("Usage: {} <filename> [<additional files>]\n", argv[0]);
        return 0;
    }

    std::vector<std::tuple<std::filesystem::path, uint32_t>> hashes;
    hashes.reserve(static_cast<std::size_t>(argc) - 1);
    for (int findex = 1; findex < argc; ++findex) {
        std::filesystem::path filepath(argv[findex]);
        if (std::filesystem::is_directory(filepath)) {
            for (auto& p: std::filesystem::directory_iterator(filepath)) {
                if (!std::filesystem::is_regular_file(p)) { continue; }
                auto const opt_hash = processFile(p);
                if (!opt_hash) {
                    return 1;
                }
                hashes.push_back(std::make_tuple(p, *opt_hash));
            }
        } else if(std::filesystem::is_regular_file(filepath)) {
            auto const opt_hash = processFile(filepath);
            if (!opt_hash) {
                return 1;
            }
            hashes.push_back(std::make_tuple(filepath, *opt_hash));
        }
    }

    auto const getPathLen = [](auto p) { return std::get<0>(p).string().length(); };
    auto const max_len = getPathLen(*std::max_element(begin(hashes), end(hashes), [getPathLen](auto const& l, auto const& r) {
        return getPathLen(l) < getPathLen(r);
    }));
    for (auto const& [fname, hash] : hashes) {
        fmt::print("\n{0:{2}} - {1:0>8X}", fname.string(), hash, max_len);
    }
    return 0;
}
