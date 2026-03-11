#include <filesystem>
#include <vector>

#include <labwork3-qqq-x-qqq/lib/argparser.h>
#include <labwork4-qqq-x-qqq/lib/archive.h>

namespace fs = std::filesystem;

static bool ReadFilesFromArgs(nargparse::ArgumentParser parser, std::vector<std::string>& names)
{
    names.clear();
    int count = nargparse::GetRepeatedCount(parser, "files");
    for (int i = 0; i < count; i++)
    {
        const char* name = nullptr;
        if (!nargparse::GetRepeated(parser, "files", i, &name))
        {
            return false;
        }
        names.push_back(name);
    }
    return true;
}

int main(int argc, char* argv[])
{
    bool create = false;
    bool extract = false;
    bool list = false;
    bool append = false;
    bool remove_file = false;
    bool concatenate = false;
    char archive_name[nargparse::kMaxArgLen] = {};
    char first_file[nargparse::kMaxArgLen] = {};

    nargparse::ArgumentParser parser = nargparse::CreateParser("hamarc", nargparse::kMaxArgLen);
    nargparse::AddFlag(parser, "-c", "--create", &create, "create archive");
    nargparse::AddFlag(parser, "-l", "--list", &list, "list archive");
    nargparse::AddFlag(parser, "-x", "--extract", &extract, "extract archive");
    nargparse::AddFlag(parser, "-a", "--append", &append, "append files");
    nargparse::AddFlag(parser, "-d", "--delete", &remove_file, "delete files");
    nargparse::AddFlag(parser, "-A", "--concatenate", &concatenate, "concatenate archives");
    nargparse::AddArgument(parser, "-f", "--file", &archive_name, "archive");
    nargparse::AddArgument(parser, &first_file, "files", nargparse::kNargsZeroOrMore);
    nargparse::AddHelp(parser);

    if (!nargparse::Parse(parser, argc, argv))
    {
        nargparse::FreeParser(parser);
        return 1;
    }

    std::vector<std::string> args_files;
    if (!ReadFilesFromArgs(parser, args_files))
    {
        nargparse::FreeParser(parser);
        return 1;
    }

    if (archive_name[0] == '\0')
    {
        nargparse::FreeParser(parser);
        return 1;
    }

    fs::path archive_path(archive_name);

    if (create)
    {
        std::vector<unsigned char> raw;
        bool ok = !args_files.empty() && hamarc::BuildArchiveData(args_files, raw) && hamarc::WriteArchive(archive_path, raw);
        nargparse::FreeParser(parser);
        return ok ? 0 : 1;
    }

    if (list)
    {
        std::vector<hamarc::FileData> files;
        bool ok = hamarc::ReadArchive(archive_path, files);
        if (ok)
        {
            hamarc::PrintFileList(files);
        }
        nargparse::FreeParser(parser);
        return ok ? 0 : 1;
    }

    if (extract)
    {
        std::vector<hamarc::FileData> files;
        bool ok = hamarc::ReadArchive(archive_path, files) && hamarc::WriteSelectedFiles(files, args_files);
        nargparse::FreeParser(parser);
        return ok ? 0 : 1;
    }

    if (append)
    {
        std::vector<hamarc::FileData> files;
        bool append_ok = false;
        bool ok = !args_files.empty() && hamarc::ReadArchive(archive_path, files);
        if (ok)
        {
            hamarc::AppendFiles(files, args_files, append_ok);
            ok = append_ok && hamarc::RewriteArchive(archive_path, files);
        }
        nargparse::FreeParser(parser);
        return ok ? 0 : 1;
    }

    if (remove_file)
    {
        std::vector<hamarc::FileData> files;
        bool ok = hamarc::ReadArchive(archive_path, files);
        if (ok)
        {
            hamarc::DeleteFiles(files, args_files);
            ok = hamarc::RewriteArchive(archive_path, files);
        }
        nargparse::FreeParser(parser);
        return ok ? 0 : 1;
    }

    if (concatenate)
    {
        std::vector<hamarc::FileData> files;
        bool ok = args_files.size() >= 2 && hamarc::ConcatenateArchives(args_files, files) && hamarc::RewriteArchive(archive_path, files);
        nargparse::FreeParser(parser);
        return ok ? 0 : 1;
    }

    nargparse::FreeParser(parser);
    return 1;
}
