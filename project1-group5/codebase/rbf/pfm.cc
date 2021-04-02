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
    if (FILE *file = fopen(fileName.c_str(), "r")){
       fclose(file);
       return -1;//file exists
    } else if ((file = fopen(fileName.c_str(), "w"))){
        fclose(file);
        return 0;
    } else {
        return -1; //could not create file
    }
}


RC PagedFileManager::destroyFile(const string &fileName)
{
    return unlink(fileName.c_str());
}


RC PagedFileManager::openFile(const string &fileName,
  FileHandle &fileHandle)
{
    if (fileHandle.fName.size() != 0){
      return -1; //filehandle already being used
    }
    fileHandle.fName = fileName;
    fileHandle.file.open(fileName.c_str(),
    fstream::in | fstream::out | fstream::binary);
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
  fName = "";
}


FileHandle::~FileHandle()
{
  file.close();
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    file.seekg(0, file.beg);
    for( unsigned i = 0; i < pageNum; i++){
        file.seekg(PAGE_SIZE, file.cur);
        if(not file.good()){
          return -1;
        }
    }
    file.read(data, PAGE_SIZE);
    if(file.good()){
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
  for( unsigned i = 0; i < pageNum; i++){
      file.seekg(PAGE_SIZE, file.cur);
      if(not file.good()){
          return -1;
      }
  }
  file.write(data, PAGE_SIZE);
  if(file.good()){
      readPageCounter++;
      return 0;
  }else{
      readPageCounter++;
      return -1;
  }
}


RC FileHandle::appendPage(const void *data)
{
    file.seekg(0, file.end);
    file.write(data, PAGE_SIZE);
    appendPageCounter++;
    return -1;
}


unsigned FileHandle::getNumberOfPages()
{
    int pages = 0;
    file.seekg(0, file.beg);
    for(;;){
        file.seekg(PAGE_SIZE, file.cur);
        if(not file.good()){
          return pages;
        } else {
          pages++;
        }
    }
    return -1;//should never return here
}


RC FileHandle::collectCounterValues(unsigned &readPageCount,
  unsigned &writePageCount, unsigned &appendPageCount)
{
	return -1;
}
