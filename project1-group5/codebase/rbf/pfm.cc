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
    if (fileHandle.file != NULL){
      return -1; //filehandle already being used
    }
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
}


FileHandle::~FileHandle()
{
  file.close();
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    char *bufptr = reinterpret_cast<char*> (data);
    file.seekg(0, file.beg);
    for( unsigned i = 0; i < pageNum; i++){
        file.seekg(PAGE_SIZE, file.cur);
        if(not file.good()){
          return -1;
        }
    }
    file.read(bufptr, PAGE_SIZE);
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
  const char *bufptr = static_cast<const char*> (data);
  file.seekg(0, file.beg);
  for( unsigned i = 0; i < pageNum; i++){
      file.seekg(PAGE_SIZE, file.cur);
      if(not file.good()){
          return -1;
      }
  }
  file.write(bufptr, PAGE_SIZE);
  if(file.good()){
      writePageCounter++;
      return 0;
  }else{
      writePageCounter++;
      return -1;
  }
}

RC FileHandle::appendPage(const void *data)
{
    const char *bufptr = static_cast<const char*> (data);
    file.seekg(0, file.end);
    file.write(bufptr, PAGE_SIZE);
    if(file.good()){
        appendPageCounter++;
        return 0;
    }else{
        appendPageCounter++;
        return -1;
    }
}


unsigned FileHandle::getNumberOfPages()
{
    file.seekg(0, file.beg);
    file.seekg(0, file.end);
    return file.tellg()/PAGE_SIZE;
}


RC FileHandle::collectCounterValues(unsigned &readPageCount,
  unsigned &writePageCount, unsigned &appendPageCount)
{
  readPageCount = readPageCounter;
  writePageCount = writePageCounter;
  appendPageCount = appendPageCounter;
	return 0;
}
