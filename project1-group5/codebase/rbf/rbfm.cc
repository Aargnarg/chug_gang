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
    byte targetPage[PAGE_SIZE];
    unsigned recordSize = getSizeOfRecord(recordDescriptor, buffer);
    unsigned totalPages = fileHandle.getNumberOfPages();
    unsigned nextSpace;
    unsigned numSlots;
    byte buf[4];

    for(unsigned i = 0; i < totalPages; i++){
        fileHandle.readPage(i, targetPage);

        memcpy(buf, targetPage + PAGE_SIZE - 4, 4);
        charToUint(buf, nextSpace);

        memcpy(buf, targetPage + PAGE_SIZE - 8, 4);
        charToUint(buf, numSlots);

        if(getSpace(targetPage) >= recordSize){
            numSlots += 1;

            memcpy(targetPage + nextSpace, buffer, recordSize);

            intToChar(buf, nextSpace);
            memcpy(targetPage + PAGE_SIZE - (numSlots * 8) - 4, buf, 4);

            nextSpace += recordSize;
            intToChar(buf, nextSpace);
            memcpy(targetPage + PAGE_SIZE - 4, buf, 4);

            intToChar(buf, numSlots);
            memcpy(targetPage + PAGE_SIZE - 8, buf, 4);

            intToChar(buf, recordSize);
            memcpy(targetPage + PAGE_SIZE - (numSlots * 8) - 8, buf, 4);

            if(!fileHandle.writePage(i, targetPage)){
                rid.pageNum = i;
                rid.slotNum = numSlots;
                return 0; //write page was successful
            } else {
                return -1; //writePage return error
            }
        }
    }
    //there was no page with space to insert a record
    //must append a new page to insert the record
    memcpy(targetPage, buffer, recordSize);

    intToChar(buf, 1);
    memcpy(targetPage + PAGE_SIZE - 8, buf, 4);

    intToChar(buf, recordSize);
    memcpy(targetPage + PAGE_SIZE - 4, buf, 4);

    intToChar(buf, recordSize);
    memcpy(targetPage + PAGE_SIZE - 16, buf, 4);

    intToChar(buf, 0);
    memcpy(targetPage + PAGE_SIZE - 12, buf, 4);

    rid.pageNum = totalPages;
    rid.slotNum = 1;
    return fileHandle.appendPage(targetPage);
}

unsigned RecordBasedFileManager::getSizeOfRecord(const vector<Attribute> &recordDescriptor, const byte *buffer)
{
    unsigned numFields = recordDescriptor.size()-1;//assuming we are not passed empty record descriptor
    unsigned numNullBytes = ceil(static_cast<float>(numFields) / 8);
    byte nullByte;
    bool nullFlag;
    unsigned nullFieldIndex = (numNullBytes * 8);
    unsigned recordSize = numNullBytes;
    unsigned char k = 0x80;
    for (unsigned i = 0; i < numNullBytes; i++){
        nullByte = buffer[i];
        while(k >= 1) {
            nullFieldIndex--;
            nullFlag = nullByte & k;
            if ((!nullFlag) && (nullFieldIndex <= numFields)){
                if(recordDescriptor.at(nullFieldIndex).type == 2){
                    recordSize += recordDescriptor
                                     .at(nullFieldIndex)
                                     .length + 4;
                } else {
                    recordSize += 4;
                }
            }
            k = k >> 1;
        }
    }
    return recordSize;
}

unsigned RecordBasedFileManager::getSpace(const byte *targetPage){
    unsigned nextFreeSpace;
    unsigned numSlots;
    byte buf[4];

    memcpy(buf, targetPage + PAGE_SIZE - 4, 4);
    charToUint(buf, nextFreeSpace);

    memcpy(buf, targetPage + PAGE_SIZE - 8, 4);
    charToUint(buf, numSlots);

    unsigned spaceEnd = PAGE_SIZE - ((numSlots + 1) * 8) - 8;
              //plus one to consider the new directory entry !!!

    return spaceEnd - nextFreeSpace;
}

void RecordBasedFileManager::intToChar(byte *buf, const unsigned n){
    buf[0] = (n >> 24) & 0xFF;
    buf[1] = (n >> 16) & 0xFF;
    buf[2] = (n >> 8)  & 0xFF;
    buf[3] =  n & 0xFF;
}

void RecordBasedFileManager::charToUint(const byte *buf, unsigned &n){
    n = ((unsigned char)(buf[0]) << 24 |
        (unsigned char)(buf[1]) << 16 |
        (unsigned char)(buf[2]) << 8  |
        (unsigned char)(buf[3]));
}

void RecordBasedFileManager::charToInt(const byte *buf, int &n){
    n = ((unsigned char)(buf[0]) << 24 |
        (unsigned char)(buf[1]) << 16 |
        (unsigned char)(buf[2]) << 8  |
        (unsigned char)(buf[3]));
}

void RecordBasedFileManager::charToFloat(const byte *buf, float &n){
    n = ((unsigned char)(buf[0]) << 24 |
        (unsigned char)(buf[1]) << 16 |
        (unsigned char)(buf[2]) << 8  |
        (unsigned char)(buf[3]));
}


RC RecordBasedFileManager::readRecord(FileHandle &fileHandle,
  const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    byte targetPage[PAGE_SIZE];
    byte buf[4];
    unsigned recordStart;
    unsigned recordSize;

    if(fileHandle.readPage(rid.pageNum, targetPage)){
      return -1;
    }

    memcpy(buf, targetPage + PAGE_SIZE - 8 - (rid.slotNum * 8), 4);
    charToUint(buf, recordSize);

    memcpy(buf, targetPage + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);
    charToUint(buf, recordStart);

    memcpy(data, targetPage + recordStart, recordSize);

    return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor,
  const void *data) {


    //THIS IS SOME PRETTY SHITTY CODE, prob gon redo

    //const byte *buffer = static_cast<const byte*>(data);
    //unsigned numFields = recordDescriptor.size();
    //unsigned numNullBytes = ceil(static_cast<float>(numFields) / 8);
    //byte *nullByte = 0;
    //bool nullFlag;
    //unsigned nullFieldIndex = (numNullBytes * 8) + 1;
    //unsigned char k = 0x80;
    //byte buf[4] = "";
    //int intVal;
    //float floatVal;
    //unsigned offset = numNullBytes;
    //char bigBuf[1000] = "";

    //for (unsigned i = 0; i < numNullBytes; i++){
    //    memcpy(nullByte, buffer+i, 1);
    //    while(k >= 1) {
    //        nullFieldIndex--;
    //        nullFlag = (*nullByte) & k;
    //        if ((!nullFlag) && (nullFieldIndex <= numFields)){
    //            cout<<recordDescriptor.at(nullFieldIndex-1).name<<": ";
    //            if(recordDescriptor.at(nullFieldIndex-1).type==0){
    //                memcpy(buf, buffer + offset, 4);
    //                charToInt(buf, intVal);
    //                cout << intVal <<"    ";
    //                offset += 4;
    //            }else if(recordDescriptor.at(nullFieldIndex-1).type==1){
    //                memcpy(buf, buffer + offset, 4);
    //                charToFloat(buf, floatVal);
    //                cout << floatVal <<"    ";
    //                offset += 4;
    //            }else{
    //              memcpy(bigBuf, buffer + offset, recordDescriptor.at(nullFieldIndex-1).length);
    //              for(unsigned i = 0;i<recordDescriptor.at(nullFieldIndex-1).length;i++){
    //                  cout << bigBuf[i];
    //              }
    //              cout<<"    ";
    //              offset += recordDescriptor.at(nullFieldIndex-1).length;
    //            }
    //        } else if ((nullFlag) && (nullFieldIndex <= numFields)){
    //            cout<<recordDescriptor.at(nullFieldIndex-1).name<<": NULL    ";
    //        }
    //        k = k >> 1;
    //    }
    //}
    //cout<<endl;
    return -1;
}
