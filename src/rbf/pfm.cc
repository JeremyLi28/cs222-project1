#include "pfm.h"

#include <iostream>

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const string &fileName)
{
	FILE * file;
	if(exists(fileName)) return -1;			// Check if the file have already exist
	file = fopen (fileName.c_str(),"wb");	// Create the file
	if (fclose(file) != 0){					// Check if the file creation is success
		perror("Close file failure!");
		return -1;
	}
	return 0;
}


RC PagedFileManager::destroyFile(const string &fileName)
{
	if(remove(fileName.c_str()) != 0){ 		//remove the file
	    perror( "Error deleting file" );
	    return -1;
	}
	return 0;
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
	if(!exists(fileName)) return -1;		// Check if the file exists
	fileHandle.setFileName(fileName);
	return 0;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    return -1;
}


FileHandle::FileHandle(): fileName("")
{
	readPageCounter = 0;
	writePageCounter = 0;
	appendPageCounter = 0;

}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    return -1;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    return -1;
}


RC FileHandle::appendPage(const void *data)
{
    return -1;
}


unsigned FileHandle::getNumberOfPages()
{
    return -1;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
	return -1;
}
