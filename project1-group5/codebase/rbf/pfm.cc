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
      fclose(file);
      return 0;
    }
    else
    {
      return -2; //could not create file
    }
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    return unlink(fileName.c_str());
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
    if (FileHandle.fName != NULL){
      return -1; //filehandle already being used
    }
    FileHandle.fName = fileName.c_str();
    fileHandle.file.open(FileHandle.fName);//assignment needed for freopen
    if(fileHandle.file.good()){
        return 0;
    } else {
        return -1; //file could not be opened
    }
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
    fileHandle.file.close();
    if(fileHandle.file.good()){
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
  totalPages = 0;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    file.seekg(0, file.beg);
    for( int i = 0; i < pageNum; i++){
        file.seekg(PAGE_SIZE, file.cur);
        if(not self.file.good()){
          return -1;
        }
    }
    self.file.read(data, PAGE_SIZE);
    if(self.file.good()){
      readPageCounter++;
      return 0;
    }else{
      readPageCounter++;
      return -1;
    }
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
  file.seekg(0, file.beg);
  for( int i = 0; i < pageNum; i++){
      file.seekg(PAGE_SIZE, file.cur);
      if(not self.file.good()){
        return -1;
      }
  }
  self.file.write(data, PAGE_SIZE);
  if(self.file.good()){
    readPageCounter++;
    return 0;
  }else{
    readPageCounter++;
    return -1;
  }
}


RC FileHandle::appendPage(const void *data)
{
    freopen(pFile, )
    appendPageCounter++;
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
