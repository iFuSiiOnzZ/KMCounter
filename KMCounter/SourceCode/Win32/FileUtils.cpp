#include "FileUtils.h"

#include <Windows.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>

#include <fstream>

//////////////////////////////////////////////////////////////////////////

s_file_content CFileUtils::LoadFileIntoMemory(const std::string &FileName)
{
	s_file_content f = { 0 };
	struct stat st = { 0 };
	FILE *pFile = NULL;

	if (!stat(FileName.c_str(), &st) && !fopen_s(&pFile, FileName.c_str(), "rb"))
	{
		f.Size = st.st_size;
		f.Memory = malloc(f.Size);

		fread(f.Memory, f.Size, 1, pFile);
		fclose(pFile);
	}

	return f;
}


void CFileUtils::RelaseFileFromMemory(s_file_content *FileContent)
{
	if (FileContent->Memory)
	{
		FileContent->Size = 0;
		free(FileContent->Memory);
		FileContent->Memory = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////

bool CFileUtils::WriteFileContentIntoDisk(const s_file_content &FileContent, const std::string &FileName, e_file_write_type op/* = Overwrite*/)
{
	FILE *pFile = NULL;
	if(fopen_s(&pFile, FileName.c_str(), op == Overwrite ? "wb" : "a+b")) return false;

	size_t w = fwrite(FileContent.Memory, FileContent.Size, 1, pFile);
	fclose(pFile);

	return w == 1;
}

bool CFileUtils::WriteFileContentIntoDisk(const std::string &ContentToWrite, const std::string &FileName, e_file_write_type op/* = Overwrite*/)
{
	s_file_content FileContent = { 0 };

	FileContent.Memory = (void *) ContentToWrite.c_str();
	FileContent.Size = ContentToWrite.size();

	return WriteFileContentIntoDisk(FileContent, FileName, op);
}

//////////////////////////////////////////////////////////////////////////

std::string CFileUtils::BaseName(const std::string &Path)
{
	size_t lastIndex = Path.find_last_of("\\/");
	return Path.substr(lastIndex + 1);
}

std::string CFileUtils::FileName(const std::string &File)
{
	std::string baseName = BaseName(File);
	size_t lastIndex = baseName.find_last_of(".");

	return  baseName.substr(0, lastIndex);
}

//////////////////////////////////////////////////////////////////////////
