#include "pfm.h"

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
    if (FILE *file = fopen(fileName.c_str(), "r"))
    {
       fclose(file);
       return -1;//file exists
    }
    else if((file = fopen(fileName.c_str(), "w")))
    {
      return 0;
    }
    else
    {
      return -2; //could not create file
    }
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    int i = unlink(fileName.c_str());
    if (i==0){
      return 0;
    } else {
      return i;
    }
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    if((fileHandle.pFile = fopen(fileName.c_str(), "r"))){
        return 0;
    } else {
        return -1;//file could not be opened
    }
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    if (fileHandle.pFile) {
        fclose(fileHandle.pFile);
        return 0;
    } else {
        return -1;
    }
}


FileHandle::FileHandle()
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
