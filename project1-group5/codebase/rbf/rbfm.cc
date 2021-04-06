#include "rbfm.h"

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
    pfm = PagedFileManager::instance();
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
    return pfm->createFile(fileName);
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    return pfm->destroyFile(fileName);
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
    return pfm->openFile(fileName, fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return pfm->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle,
   const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    const byte *buffer = static_cast<const byte*> (data);
    unsigned recordSize = getSize(recordDescriptor, buffer);
    byte targetPage[PAGE_SIZE];

    unsigned totalPages = fileHandle.getNumberOfPages();
    unsigned nextFreeSpace;
    unsigned *dirPtr;//used for updating the directory
    for(unsigned i = 0; i < totalPages; i++){
        fileHandle.readPage(i, targetPage);
        nextFreeSpace = getSpace(targetPage, recordSize);
        if(nextFreeSpace != PAGE_SIZE + 1){
            for(unsigned k = 0; k < recordSize; k++){
                targetPage[nextFreeSpace] = buffer[k];
                nextFreeSpace++;
            }
            dirPtr = reinterpret_cast<const unsigned*> targetPage[PAGE_SIZE-4];
            dirValue targetPage[PAGE_SIZE - 4] += recordSize;//offset
            numSlots += 1;//numSlots
            
            if(fileHandle.writePage(i, targetPage)){
                rid.pageNum = i;
                rid.slotNum = targetPage[PAGE_SIZE-8];
                return 0;
            } else {
                return -1;
            }
        }
    }
    //there was no page with space to insert a record
    //must append a new page to insert the record
    nextFreeSpace = 0;
    for(unsigned k = 0; k < recordSize; k++){
        targetPage[nextFreeSpace] = buffer[k];
        nextFreeSpace++;
    }
    dirPtr = reinterpret_cast<unsigned*>(targetPage[PAGE_SIZE-4]);
    *dirPtr = recordSize;
    *(dirPtr-4) = 1;
    *(dirPtr-8) = recordSize;
    *(dirPtr-12) = 0;
    rid.pageNum = fileHandle.getNumberOfPages() + 1;
    rid.slotNum = 0;
    return fileHandle.appendPage(targetPage);
}

unsigned RecordBasedFileManager::getSize(
const vector<Attribute> &recordDescriptor,
const byte *buffer)
  {
    unsigned numFields = recordDescriptor.size();
    unsigned numNullBytes = ceil(numFields/8);
    byte nullByte;
    bool nullFlag;
    unsigned nullFieldIndex = (numNullBytes * 8) + 1;
    unsigned record_size = numNullBytes;
    unsigned char k = 0x80;
    for (unsigned i = 0; i < numNullBytes; i++){
        nullByte = buffer[i];
        while(k >= 1) {
            nullFieldIndex--;
            nullFlag = nullByte & k;
            if ((!nullFlag) && (nullFieldIndex <= numFields)){
                if(recordDescriptor.at(nullFieldIndex).type == 2){
                    record_size += recordDescriptor
                                     .at(nullFieldIndex)
                                     .length + 4;
                } else {
                    record_size += 4;
                }
            }
            k = k >> 1;
        }
    }
    return record_size;
}

unsigned RecordBasedFileManager::getSpace(const byte *buffer, const unsigned recordSize){
    unsigned *nextFreeSpace = reinterpret_cast<unsigned*>(buffer[PAGE_SIZE-4]);
    unsigned *numSlots = reinterpret_cast<unsigned*>(buffer[PAGE_SIZE-8]);
    unsigned spaceEnd = PAGE_SIZE - ((*numSlots+1) * 8) - 8;//numSlots +1 for the added slot
    unsigned space = spaceEnd - (*nextFreeSpace);
    if(space >= recordSize){
        return *nextFreeSpace;
    } else {
        return PAGE_SIZE+1;
    }
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle,
  const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    fileHandle.readPage(rid.pageNum, data);
    byte *buffer = static_cast<byte*> (data);
    unsigned *numSlots = reinterpret_cast<unsigned*>(buffer[PAGE_SIZE-8]);



    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor,
  const void *data) {
    return -1;
}
