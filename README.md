# HamArc

Error-resilient file archiver in C++ with Hamming-code-based storage.

## Features

- Create archives from multiple files
- List archive contents
- Extract all files or selected entries
- Append and delete files
- Concatenate multiple archives
- Store data in a custom binary format with error-correction support

## Tech Stack

- C++
- STL
- CMake
- File system
- Binary formats
- Hamming codes

## CLI

```bash
hamarc --create --file=archive.haf file1 file2 file3
hamarc -l -f archive.haf
hamarc --extract -f archive.haf
```

## Project Structure

- `lib/archive.*` — archive format and file operations
- `lib/hamming.*` — encoding and decoding logic
- `lib/argparser.*` — command-line parsing
- `tests/` — archive tests

## Notes

The project is built as a modular console application and focuses on reliable storage, binary serialization, and command-line tooling.
