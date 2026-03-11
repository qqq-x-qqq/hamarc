#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace hamarc
{

struct FileData
{
    std::string name;
    std::vector<unsigned char> data;
};

bool BuildArchiveData(const std::vector<std::string>& paths, std::vector<unsigned char>& raw);
bool WriteArchive(const std::filesystem::path& archive, const std::vector<unsigned char>& raw);
bool ReadArchiveBytes(const std::filesystem::path& archive, std::vector<unsigned char>& raw);
bool ParseArchive(const std::vector<unsigned char>& raw, std::vector<FileData>& files);

bool ReadArchive(const std::filesystem::path& archive, std::vector<FileData>& files);
bool RewriteArchive(const std::filesystem::path& archive, const std::vector<FileData>& files);
bool WriteSelectedFiles(const std::vector<FileData>& files, const std::vector<std::string>& names);

void PrintFileList(const std::vector<FileData>& files);
void AppendFiles(std::vector<FileData>& files, const std::vector<std::string>& paths, bool& ok);
void DeleteFiles(std::vector<FileData>& files, const std::vector<std::string>& names);
bool ConcatenateArchives(const std::vector<std::string>& archives, std::vector<FileData>& result);
bool HasName(const std::vector<std::string>& names, const std::string& name);

}  // namespace hamarc
