#include "archive.h"
#include "hamming.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace hamarc
{

static void PushUint32(std::vector<unsigned char>& out, std::uint32_t value)
{
    for (int i = 0; i < 4; i++)
    {
        out.push_back((value >> (i * 8)) & 0xFF);
    }
}  // namespace hamarc

static void PushUint64(std::vector<unsigned char>& out, std::uint64_t value)
{
    for (int i = 0; i < 8; i++)
    {
        out.push_back((value >> (i * 8)) & 0xFF);
    }
}

static std::uint32_t ReadUint32(const std::vector<unsigned char>& data, std::size_t& pos)
{
    std::uint32_t value = 0;
    for (int i = 0; i < 4; i++)
    {
        value |= (std::uint32_t)data[pos++] << (i * 8);
    }
    return value;
}

static std::uint64_t ReadUint64(const std::vector<unsigned char>& data, std::size_t& pos)
{
    std::uint64_t value = 0;
    for (int i = 0; i < 8; i++)
    {
        value |= (std::uint64_t)data[pos++] << (i * 8);
    }
    return value;
}

static bool ReadFile(const fs::path& path, std::vector<unsigned char>& data)
{
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open())
    {
        return false;
    }

    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0)
    {
        return false;
    }

    data.resize((std::size_t)size);
    if (size > 0)
    {
        in.read((char*)data.data(), size);
    }

    return in.good() || in.eof();
}

static bool WriteFile(const fs::path& path, const std::vector<unsigned char>& data)
{
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open())
    {
        return false;
    }

    if (!data.empty())
    {
        out.write((const char*)data.data(), (std::streamsize)data.size());
    }

    return out.good();
}

static bool BuildArchiveFromFiles(const std::vector<FileData>& files, std::vector<unsigned char>& raw)
{
    raw.clear();
    raw.push_back('H');
    raw.push_back('A');
    raw.push_back('F');
    raw.push_back('1');
    PushUint32(raw, (std::uint32_t)files.size());

    for (std::size_t i = 0; i < files.size(); i++)
    {
        PushUint32(raw, (std::uint32_t)files[i].name.size());
        for (std::size_t j = 0; j < files[i].name.size(); j++)
        {
            raw.push_back((unsigned char)files[i].name[j]);
        }

        PushUint64(raw, (std::uint64_t)files[i].data.size());
        for (std::size_t j = 0; j < files[i].data.size(); j++)
        {
            raw.push_back(files[i].data[j]);
        }
    }

    return true;
}

bool BuildArchiveData(const std::vector<std::string>& paths, std::vector<unsigned char>& raw)
{
    std::vector<FileData> files;
    for (std::size_t i = 0; i < paths.size(); i++)
    {
        fs::path path(paths[i]);
        FileData file;
        file.name = path.filename().string();
        if (!ReadFile(path, file.data))
        {
            return false;
        }
        files.push_back(file);
    }

    return BuildArchiveFromFiles(files, raw);
}

bool WriteArchive(const fs::path& archive, const std::vector<unsigned char>& raw)
{
    std::ofstream out(archive, std::ios::binary);
    if (!out.is_open())
    {
        return false;
    }

    for (std::size_t i = 0; i < raw.size(); i++)
    {
        std::uint16_t code = EncodeByte(raw[i]);
        unsigned char lo = code & 0xFF;
        unsigned char hi = (code >> 8) & 0xFF;
        out.put((char)lo);
        out.put((char)hi);
    }

    return out.good();
}

bool ReadArchiveBytes(const fs::path& archive, std::vector<unsigned char>& raw)
{
    std::ifstream in(archive, std::ios::binary);
    if (!in.is_open())
    {
        return false;
    }

    std::vector<unsigned char> encoded;
    in.seekg(0, std::ios::end);
    std::streamsize size = in.tellg();
    in.seekg(0, std::ios::beg);

    if (size < 0 || size % 2 != 0)
    {
        return false;
    }

    encoded.resize((std::size_t)size);
    if (size > 0)
    {
        in.read((char*)encoded.data(), size);
    }

    if (!(in.good() || in.eof()))
    {
        return false;
    }

    raw.clear();
    for (std::size_t i = 0; i < encoded.size(); i += 2)
    {
        std::uint16_t code = encoded[i] | ((std::uint16_t)encoded[i + 1] << 8);
        raw.push_back(DecodeByte(code));
    }

    return true;
}

bool ParseArchive(const std::vector<unsigned char>& raw, std::vector<FileData>& files)
{
    if (raw.size() < 8)
    {
        return false;
    }

    if (raw[0] != 'H' || raw[1] != 'A' || raw[2] != 'F' || raw[3] != '1')
    {
        return false;
    }

    std::size_t pos = 4;
    std::uint32_t count = ReadUint32(raw, pos);
    files.clear();

    for (std::uint32_t i = 0; i < count; i++)
    {
        if (pos + 4 > raw.size())
        {
            return false;
        }

        std::uint32_t name_size = ReadUint32(raw, pos);
        if (pos + name_size > raw.size())
        {
            return false;
        }

        FileData file;
        file.name.assign((const char*)&raw[pos], (std::size_t)name_size);
        pos += name_size;

        if (pos + 8 > raw.size())
        {
            return false;
        }

        std::uint64_t file_size = ReadUint64(raw, pos);
        if (pos + file_size > raw.size())
        {
            return false;
        }

        file.data.resize((std::size_t)file_size);
        for (std::uint64_t j = 0; j < file_size; j++)
        {
            file.data[(std::size_t)j] = raw[pos++];
        }

        files.push_back(file);
    }

    return pos == raw.size();
}

bool ReadArchive(const fs::path& archive, std::vector<FileData>& files)
{
    std::vector<unsigned char> raw;
    return ReadArchiveBytes(archive, raw) && ParseArchive(raw, files);
}

bool RewriteArchive(const fs::path& archive, const std::vector<FileData>& files)
{
    std::vector<unsigned char> raw;
    return BuildArchiveFromFiles(files, raw) && WriteArchive(archive, raw);
}

bool HasName(const std::vector<std::string>& names, const std::string& name)
{
    for (std::size_t i = 0; i < names.size(); i++)
    {
        if (names[i] == name)
        {
            return true;
        }
    }
    return false;
}

bool WriteSelectedFiles(const std::vector<FileData>& files, const std::vector<std::string>& names)
{
    for (std::size_t i = 0; i < files.size(); i++)
    {
        if (!names.empty() && !HasName(names, files[i].name))
        {
            continue;
        }

        if (!WriteFile(files[i].name, files[i].data))
        {
            return false;
        }
    }

    return true;
}

void PrintFileList(const std::vector<FileData>& files)
{
    for (std::size_t i = 0; i < files.size(); i++)
    {
        std::cout << files[i].name << '\n';
    }
}

void AppendFiles(std::vector<FileData>& files, const std::vector<std::string>& paths, bool& ok)
{
    ok = true;

    for (std::size_t i = 0; i < paths.size(); i++)
    {
        fs::path path(paths[i]);
        FileData file;
        file.name = path.filename().string();
        if (!ReadFile(path, file.data))
        {
            ok = false;
            return;
        }

        bool replaced = false;
        for (std::size_t j = 0; j < files.size(); j++)
        {
            if (files[j].name == file.name)
            {
                files[j] = file;
                replaced = true;
                break;
            }
        }

        if (!replaced)
        {
            files.push_back(file);
        }
    }
}

void DeleteFiles(std::vector<FileData>& files, const std::vector<std::string>& names)
{
    std::vector<FileData> filtered;

    for (std::size_t i = 0; i < files.size(); i++)
    {
        if (!HasName(names, files[i].name))
        {
            filtered.push_back(files[i]);
        }
    }

    files = filtered;
}

bool ConcatenateArchives(const std::vector<std::string>& archives, std::vector<FileData>& result)
{
    result.clear();

    for (std::size_t i = 0; i < archives.size(); i++)
    {
        std::vector<FileData> files;
        if (!ReadArchive(archives[i], files))
        {
            return false;
        }

        for (std::size_t j = 0; j < files.size(); j++)
        {
            result.push_back(files[j]);
        }
    }

    return true;
}

}
