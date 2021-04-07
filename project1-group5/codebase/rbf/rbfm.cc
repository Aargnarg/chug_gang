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
    pfm = 0;
    delete _rbf_manager;
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
    const byte *buffer = static_cast<const byte*>(data);
    byte targetPage[PAGE_SIZE];
    unsigned recordSize = getSizeOfRecord(recordDescriptor, buffer);
    unsigned totalPages = fileHandle.getNumberOfPages();
    unsigned nextSpace;
    unsigned numSlots;

    for(unsigned i = totalPages; i>0; i--){
        fileHandle.readPage(i-1, targetPage);//load new page
        //check if there is enough space on the page
        if(getSpace(targetPage) >= recordSize){
            //get nextspace of new page
            memcpy(&nextSpace, targetPage + PAGE_SIZE - 4, 4);
            //get numSlots of new page
            memcpy(&numSlots, targetPage + PAGE_SIZE - 8, 4);
            //increment numSlots
            numSlots += 1;
            memcpy(targetPage + PAGE_SIZE - 8, &numSlots, 4);
            //put data at next space
            memcpy(targetPage + nextSpace, buffer, recordSize);
            //set offset to start of new record
            memcpy(targetPage + PAGE_SIZE - (numSlots * 8) - 4, &nextSpace, 4);
            //set size of record
            memcpy(targetPage + PAGE_SIZE - (numSlots * 8) - 8, &recordSize, 4);
            //increase nextSpace by size of the new record inserted
            nextSpace += recordSize;
            memcpy(targetPage + PAGE_SIZE - 4, &nextSpace, 4);
            //write the page and fill rid
            if(!fileHandle.writePage(i-1, targetPage)){
                rid.pageNum = i-1;
                rid.slotNum = numSlots;
                return 0; //write page was successful
            } else {
                return -1; //writePage return error
            }
        }
    }
    //there was no page with space to insert a record
    //must append a new page to insert the record
    nextSpace = 0;
    numSlots = 1;
    memset(targetPage, 0, PAGE_SIZE); //make new page empty
    memcpy(targetPage, buffer, recordSize);//copy data to begining of new page
    memcpy(targetPage + PAGE_SIZE - 12, &nextSpace, 4);
    memcpy(targetPage + PAGE_SIZE - 8, &numSlots, 4);
    memcpy(targetPage + PAGE_SIZE - 4, &recordSize, 4);
    memcpy(targetPage + PAGE_SIZE - 16, &recordSize, 4);
    rid.pageNum = totalPages;
    rid.slotNum = numSlots;
    return fileHandle.appendPage(targetPage);
}

unsigned RecordBasedFileManager::getSizeOfRecord(const vector<Attribute> &recordDescriptor, const byte *buffer)
{
    unsigned numFields = recordDescriptor.size()-1;//assuming we are not passed empty record descriptor
    unsigned numNullBytes = ceil(static_cast<float>(numFields) / 8);
    byte nullByte;
    unsigned char bitCheck;
    bool nullFlag;
    unsigned nullFieldIndex = (numNullBytes * 8);
    unsigned recordSize = numNullBytes;

    for (unsigned i = 0; i < numNullBytes; i++){
        memcpy(&nullByte, buffer+i, 1);
        bitCheck = 128; //10000000 or 0x80
        while(bitCheck >= 1) {
            nullFieldIndex--;
            nullFlag = nullByte & bitCheck;
            if ((!nullFlag) && (nullFieldIndex <= numFields)){
                if(recordDescriptor.at(nullFieldIndex).type == 2){
                    recordSize += recordDescriptor
                                     .at(nullFieldIndex)
                                     .length + 4;
                          //4 bytes to store the length, and then the length
                } else {
                    recordSize += 4;
                    //ints and floats are 4 bytes
                }
            }
            bitCheck /= 2; //10000000 >> 01000000
        }
    }
    return recordSize;
}

unsigned RecordBasedFileManager::getSpace(const byte *targetPage){
    unsigned nextFreeSpace;
    unsigned numSlots;
    memcpy(&nextFreeSpace, targetPage + PAGE_SIZE - 4, 4);
    memcpy(&numSlots, targetPage + PAGE_SIZE - 8, 4);
    unsigned spaceEnd = PAGE_SIZE - ((numSlots + 1) * 8) - 8;
              //plus one to consider the new directory entry !!!
    return spaceEnd - nextFreeSpace;
}


RC RecordBasedFileManager::readRecord(FileHandle &fileHandle,
  const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    byte targetPage[PAGE_SIZE];
    unsigned recordStart;
    unsigned recordSize;
    if(fileHandle.readPage(rid.pageNum, targetPage)){
      return -1;
    }
    memcpy(&recordSize, targetPage + PAGE_SIZE - 8 - (rid.slotNum * 8), 4);
    memcpy(&recordStart, targetPage + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);
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
