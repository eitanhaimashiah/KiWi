#ifndef tangible_filesystem_h
#define tangible_filesystem_h

#include <string>
#include <filesystem>

using namespace std;

class FileSystem
{
public:
	static void createDirectory(const wstring &path)
	{
		filesystem::create_directory(pathFromString(path));
	}

	static bool pathExists(const wstring &path)
	{
		return filesystem::exists(pathFromString(path));
	}

	static bool fileExists(const wstring &path)
	{
		return filesystem::is_regular_file(pathFromString(path));
	}

	static bool directoryExists(const wstring &path)
	{
		return filesystem::is_directory(pathFromString(path));
	}

	static wstring getFullPath(const wstring &path)
	{
		return filesystem::absolute(pathFromString(path)).generic_wstring();
	}

	static wstring getFileName(const wstring &path)
	{
		return filesystem::path(pathFromString(path)).filename().generic_wstring();
	}

	static wstring getDirectoryName(const wstring &path)
	{
		return filesystem::path(pathFromString(path)).parent_path().generic_wstring();
	}

	static wchar_t preferredSeparator()
	{
		return filesystem::path::preferred_separator;
	}

private:
	static filesystem::path pathFromString(const wstring &path)
	{
		return filesystem::path(&path[0]);
	}
};

#endif /* tangible_filesystem_h */
