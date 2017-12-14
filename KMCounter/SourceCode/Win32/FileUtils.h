#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <vector>

typedef struct s_file_content
{
	void *Memory;
	size_t Size;
} s_file_content;

struct CFileUtils
{
	enum e_file_write_type
	{
		Append,
		Overwrite
	};

	static void RelaseFileFromMemory(s_file_content *FileContent);
	static s_file_content LoadFileIntoMemory(const std::string &FileName);

	static std::string BaseName(const std::string &Path);
	static std::string FileName(const std::string &File);

	static bool WriteFileContentIntoDisk(const std::string &ContentToWrite, const std::string &FileName, e_file_write_type op = Overwrite);
	static bool WriteFileContentIntoDisk(const s_file_content &FileContent, const std::string &FileName, e_file_write_type op = Overwrite);
};

#endif
