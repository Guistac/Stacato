#include <pch.h>

#include "FileDialog.h"

#include <nfd.hpp>

namespace FileDialog {

	bool init() {
		return nfdresult_t::NFD_OKAY == NFD::Init();
	}

	void terminate() {
		NFD::Quit();
	}

    bool load(FilePath& filePath) {
        std::vector<FileTypeFilter> dummy;
        return load(filePath, dummy);
    }

	bool load(FilePath& filePath, std::vector<FileTypeFilter>& fileTypeFilters) {
        NFD::UniquePath outPath;
        nfdfilteritem_t* filters = new nfdfilteritem_t[fileTypeFilters.size()];
        for (int i = 0; i < fileTypeFilters.size(); i++) {
            filters[i].name = fileTypeFilters[i].fileTypeString;
            filters[i].spec = fileTypeFilters[i].extensionsString;
        }
        nfdresult_t result = NFD::OpenDialog(outPath, filters, fileTypeFilters.size());
        if (result == NFD_OKAY) {
            filePath.set(outPath.get());
            return true;
        }
        else if (result == NFD_CANCEL) return false;
        else Logger::error("LoadFileDialogError: {}", NFD::GetError());
        return false;
	}

    bool loadMultiple(std::vector<FilePath>& paths) {
        std::vector<FileTypeFilter> dummy;
        return loadMultiple(paths, dummy);
    }

    bool loadMultiple(std::vector<FilePath>& paths, std::vector<FileTypeFilter>& fileTypeFilters) {
        NFD::UniquePathSet outPaths;
        nfdfilteritem_t* filters = new nfdfilteritem_t[fileTypeFilters.size()];
        for (int i = 0; i < fileTypeFilters.size(); i++) {
            filters[i].name = fileTypeFilters[i].fileTypeString;
            filters[i].spec = fileTypeFilters[i].extensionsString;
        }
        nfdresult_t result = NFD::OpenDialogMultiple(outPaths, filters, fileTypeFilters.size());
        if (result == NFD_OKAY) {
            nfdpathsetsize_t numPaths;
            NFD::PathSet::Count(outPaths, numPaths);
            paths.resize(numPaths);
            nfdpathsetsize_t i;
            for (i = 0; i < numPaths; ++i) {
                NFD::UniquePathSetPath path;
                NFD::PathSet::GetPath(outPaths, i, path);
                paths[i].set(path.get());
            }
            return true;
        }
        else if (result == NFD_CANCEL) return false;
        else Logger::error("LoadMultipleFileDialogError: {}", NFD::GetError());
        return false;
    }

    bool save(FilePath& path) {
        std::vector<FileTypeFilter> dummy;
        return save(path, dummy);
    }

    bool save(FilePath& path, std::vector<FileTypeFilter>& fileTypeFilters) {
        NFD::UniquePath savePath;
        nfdfilteritem_t* filters = new nfdfilteritem_t[fileTypeFilters.size()];
        for (int i = 0; i < fileTypeFilters.size(); i++) {
            filters[i].name = fileTypeFilters[i].fileTypeString;
            filters[i].spec = fileTypeFilters[i].extensionsString;
        }
        nfdresult_t result = NFD::SaveDialog(savePath, filters, fileTypeFilters.size(), NULL, "Untitled.c");
        if (result == NFD_OKAY) {
            path.set(savePath.get());
            return true;
        }
        else if (result == NFD_CANCEL) return false;
        else Logger::error("Error: {}", NFD::GetError());
        return false;
    }
    
    bool openFolder(FilePath& path) {
        NFD::UniquePath outPath;
        nfdresult_t result = NFD::PickFolder(outPath);
        if (result == NFD_OKAY) {
            path.set(outPath.get());
            return true;
        }
        else if (result == NFD_CANCEL) return false;
        else Logger::error("Error: {}", NFD::GetError());
        return false;
    }


}