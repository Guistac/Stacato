#pragma once

namespace FileDialog {

	bool init();
	void terminate();


	struct FilePath {
		void set(const char* fp) {
			strcpy(path, fp);
			size_t l = strlen(path);
			size_t nameStartIndex = 0;
			for (int i = l - 1; i >= 0; i--) {
				if (path[i] == '/' || path[i] == '\\') {
					file = &path[i + 1];
					break;
				}
			}
			size_t extensionStartIndex = 0;
			for (int i = l - 1; i >= 0; i--) {
				if (path[i] == '.') {
					extension = &path[i + 1];
					break;
				}
			}
		}
		char path[512] = { 0 };
		const char* file = nullptr;
		const char* extension = nullptr;
	};

	struct FileTypeFilter {
		FileTypeFilter(const char* type, const char* extensions) {
			strcpy(fileTypeString, type);
			strcpy(extensionsString, extensions);
		}
		char fileTypeString[128];
		char extensionsString[128];
	};


	bool load(FilePath& path);
	bool load(FilePath& path, std::vector<FileTypeFilter>& fileTypeFilters);
	bool load(FilePath& path, FileTypeFilter& fileTypeFilter);
	bool loadMultiple(std::vector<FilePath>& paths);
	bool loadMultiple(std::vector<FilePath>& paths, std::vector<FileTypeFilter>& fileTypeFilter);
	bool loadMultiple(std::vector<FilePath>& paths, FileTypeFilter& fileTypeFilter);
	bool save(FilePath& path, const char* defaultName = nullptr);
	bool save(FilePath& path, std::vector<FileTypeFilter>& fileTypeFiler, const char* defaultName = nullptr);
	bool save(FilePath& path, FileTypeFilter& fileTypeFilter, const char* defaultName = nullptr);
	bool openFolder(FilePath& path);

}