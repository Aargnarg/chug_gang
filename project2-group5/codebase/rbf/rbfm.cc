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

RC RecordBasedFileManager::openFile(const string &fileName,
                                    FileHandle &fileHandle) {
    return pfm->openFile(fileName, fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return pfm->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle,
                                  const vector<Attribute> &recordDescriptor,
                                  const void *data,
                                        RID &rid) {

    const byte *buffer = static_cast<const byte*>(data);
    byte page[PAGE_SIZE];
    int recordSize = getSizeOfRecord(recordDescriptor, buffer);
    int totalPages = fileHandle.getNumberOfPages();
    int nextSpace;
    int numSlots;

    for(int i = totalPages; i>0; i--){
        fileHandle.readPage(i-1, page);
        if((getSpace(page) - 8) >= recordSize){
        //check if there is enough space on the page
        // -8 to consider the space of the new directory entry !

            //get nextspace of new page
            memcpy(&nextSpace, page + PAGE_SIZE - 4, 4);
            //get numSlots of new page
            memcpy(&numSlots, page + PAGE_SIZE - 8, 4);
            //increment numSlots
            numSlots += 1;
            memcpy(page + PAGE_SIZE - 8, &numSlots, 4);
            //put data at next space
            memcpy(page + nextSpace, buffer, recordSize);
            //set start of new record
            memcpy(page + PAGE_SIZE - (numSlots * 8) - 4, &nextSpace, 4);
            //set size of record
            memcpy(page + PAGE_SIZE - (numSlots * 8) - 8, &recordSize, 4);
            //increase offset by size of the new record inserted
            nextSpace += recordSize;
            memcpy(page + PAGE_SIZE - 4, &nextSpace, 4);
            //write the page and fill rid
            if(!fileHandle.writePage(i-1, page)){
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
    memset(page, 0, PAGE_SIZE);//make new page empty
    memcpy(page, buffer, recordSize);//copy data to begining of new page
    memcpy(page + PAGE_SIZE - 12, &nextSpace, 4);
    memcpy(page + PAGE_SIZE - 8, &numSlots, 4);
    memcpy(page + PAGE_SIZE - 4, &recordSize, 4);
    memcpy(page + PAGE_SIZE - 16, &recordSize, 4);
    rid.pageNum = totalPages;
    rid.slotNum = numSlots;
    return fileHandle.appendPage(page);
}

int RecordBasedFileManager::getSizeOfRecord(
  const vector<Attribute> &recordDescriptor, const byte *buffer){
    //function assumes it is not passed empty record descriptor
    int numFields = recordDescriptor.size()-1;
    int numNullBytes = ceil(static_cast<float>(numFields) / 8);
    int recordSize = numNullBytes;
    int nullFieldIndex = 0;
    int strlen;
    bool nullFlag;
    bool flag = true;
    byte nullByte;
    unsigned char bitCheck;

    for (int i = 0; i < numNullBytes; i++){
        memcpy(&nullByte, buffer+i, 1);
        bitCheck = 128; //10000000 or 0x80
        while(bitCheck >= 1) {
            if (flag){
                flag = false;
            }else{
                nullFieldIndex++;
            }
            nullFlag = nullByte & bitCheck;
            if ((!nullFlag) && (nullFieldIndex <= numFields)){
                if(recordDescriptor.at(nullFieldIndex).type == 2){
                    memcpy(&strlen, buffer + recordSize, 4);
                    recordSize += strlen + 4;
                    //4 bytes to store the length, and then the length
                } else {
                    recordSize += 4;
                    //ints and floats are 4 bytes
                }
            }
            bitCheck /= 2;
            //10000000 >> 01000000
        }
    }
    return recordSize;
}


int RecordBasedFileManager::getSpace(const byte *page){
    int nextFreeSpace;
    int numSlots;
    int spaceEnd;

    memcpy(&nextFreeSpace, page + PAGE_SIZE - 4, 4);
    memcpy(&numSlots, page + PAGE_SIZE - 8, 4);
    spaceEnd = PAGE_SIZE - ((numSlots) * 8) - 8;

    if(spaceEnd <= nextFreeSpace)
        return 0;
    return spaceEnd - nextFreeSpace;
}


RC RecordBasedFileManager::readRecord(FileHandle &fileHandle,
                                const vector<Attribute> &recordDescriptor,
                                const RID &rid,
                                      void *data) {

    byte page[PAGE_SIZE];
    int recordStart;
    int recordSize;

    if (fileHandle.readPage(rid.pageNum, page))
        return -1;
    memcpy(&recordSize, page + PAGE_SIZE - 8 - (rid.slotNum * 8), 4);
    memcpy(&recordStart, page + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);

    if(recordSize < 0){

    }

    memcpy(data, page + recordStart, recordSize);
    return 0;
}


RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor,
                                       const void *data) {

    const byte *buffer = static_cast<const byte*>(data);
    unsigned numFields = recordDescriptor.size() - 1;
    unsigned numNullBytes = ceil(static_cast<float>(numFields) / 8);
    byte nullByte = 0;
    bool nullFlag;
    unsigned nullFieldIndex = 0;
    unsigned char bitChecker = 128;
    int intVal;
    float floatVal;
    unsigned strLen;
    char strBuffer[PAGE_SIZE];
    unsigned offset = numNullBytes;
    bool flag = true;

    for (unsigned i = 0; i < numNullBytes; i++){
        memcpy(&nullByte, buffer+i, 1);
        bitChecker = 128;
        while(bitChecker >= 1) {
            if (flag){
                flag = false;
            }else{
                nullFieldIndex++;
            }
            nullFlag = nullByte & bitChecker;
            if ((!nullFlag) && (nullFieldIndex <= numFields)){
                cout<<recordDescriptor.at(nullFieldIndex).name<<": ";
                if(recordDescriptor.at(nullFieldIndex).type==0) {
                    memcpy(&intVal, buffer + offset, 4);
                    cout << intVal << "    ";
                    offset += 4;
                }else if(recordDescriptor.at(nullFieldIndex).type==1) {
                    memcpy(&floatVal, buffer + offset, 4);
                    cout << floatVal << "    ";
                    offset += 4;
                } else {
                    memcpy(&strLen, buffer + offset, 4);
                    offset += 4;
                    memcpy(strBuffer, buffer + offset, strLen);
                    for(unsigned i = 0; i < strLen; i++){
                        cout << strBuffer[i];
                    }
                    cout << "    ";
                    offset += strLen;
                }
            } else if ((nullFlag) && (nullFieldIndex <= numFields)) {
                cout << recordDescriptor.at(nullFieldIndex).name
                     << ": NULL    ";
            }
            bitChecker /= 2;
        }
    }
    cout << endl;
    return 0;
}

RC RecordBasedFileManager::deleteRecord(FileHandle &fileHandle,
                                  const vector<Attribute> &recordDescriptor,
                                  const RID &rid) {
    byte page[PAGE_SIZE];
    int recordStart;
    int recordSize;
    int recordEnd;
    int totalSlots;
    int temp;

    if(fileHandle.readPage(rid.pageNum, page))
        return -1;
    memcpy(&recordSize, page + PAGE_SIZE - 8 - (rid.slotNum * 8), 4);
    memcpy(&recordStart, page + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);
    memcpy(&totalSlots, page + PAGE_SIZE - 8, 4);
    memcpy(&spaceStart, page + PAGE_SIZE - 4, 4);
    recordEnd = recordStart + recordSize;

    if(recordSize < 0){

    }
    //else record exists on the page

    //simultaneously delete and fill in the freespace left by the deleted record
    //by sliding over all records that came after it
    memmove(page + recordStart, page + recordEnd, spaceStart - recordEnd);

    //IMPORTANT
    //made the choice for 'PAGE_SIZE + 1' as the directory size
    //to represent a deleted record
    temp = 0;
    memcpy(page + PAGE_SIZE - 8 - (rid.slotNum * 8), &temp, 4);

    //update the offsets of all slots that came after the deleted record
    for(int i = 1; i < rid.slotNum; i++){
        memcpy(&temp, page + PAGE_SIZE - 8 - (rid.slotNum * 8), 4);
        temp -= recordSize;
        memcpy(page + PAGE_SIZE - 8 - (rid.slotNum * 8), &temp, 4);
    }

    //update the spaceStart of the page
    spaceStart -= recordSize;
    memcpy(page + PAGE_SIZE - 4, &spaceStart, 4);

    return fileHandle.writePage(rid.pageNum, page);
}

RC updateRecord(FileHandle &fileHandle,
                const vector<Attribute> &recordDescriptor,
                const void *data,
                const RID &rid) {
    const byte *buffer = static_cast<const byte*>(data);
    byte page[PAGE_SIZE];
    int recordStart;
    int recordSize;
    int new_recordSize = getSizeOfRecord(recordDescriptor, buffer);
    int recordEnd;
    int totalSlots;
    int freeSpace;
    int sizeDifference;
    int recordEnd;

    if(fileHandle.readPage(rid.pageNum, page)) return -1;

    memcpy(&recordSize, page + PAGE_SIZE - 8 - (rid.slotNum * 8), 4);
    memcpy(&recordStart, page + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);
    memcpy(&totalSlots, page + PAGE_SIZE - 8, 4);
    memcpy(&spaceStart, page + PAGE_SIZE - 4, 4);
    recordEnd = recordStart + recordSize;

    if (rid.slotnum > totalSlots) return -1;

    if(recordSize < 0){ //record was deleted

    }
    //else record exists on the page

    if(recordSize == new_recordSize) {
        //trivial update case
        memcpy(page + recordStart, buffer, recordSize);

    } else if(recordSize > new_recordSize) {
        sizeDifference =  recordSize - new_recordSize;
        //no need to check if there is space on the page for this case

        //update the slot's size
        memcpy(page + PAGE_SIZE - 8 - (rid.slotNum * 8),
               &new_recordSize, 4);

        //update the slot data
        memcpy(page + recordStart, buffer, new_recordSize);

        //move all records after it to fill in the free space
        memmove(page + recordStart + new_recordSize,
                Page + recordEnd,
                spaceStart - recordEnd);

        //update offsets of all slots in the directory
        for(int i = 1; i < rid.slotNum; i++) {
            memcpy(&temp, page + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);
            temp -= sizeDifference;
            memcpy(page + PAGE_SIZE - 4 - (rid.slotNum * 8), &temp, 4);
        }

        //update the spaceStart in the directory
        spaceStart -= sizeDifference;
        memcpy(page + PAGE_SIZE - 4, &spaceStart, 4);

    } else {//(recordSize < new_recordSize)
        sizeDifference =  new_recordSize - recordSize;

        //page has space for the update
        if(getSpace(page) >= sizeDifference){
            //update the slot's size
            memcpy(page + PAGE_SIZE - 8 - (rid.slotNum * 8),
                   &new_recordSize, 4);

            //update the slot data
            memcpy(page + recordStart, buffer, new_recordSize);

            //move all records after it to fill in the free space
            memmove(page + recordStart + new_recordSize,
                   Page + recordEnd,
                   spaceStart - recordEnd);

            //update offsets of all slots in the directory
            for(int i = 1; i < rid.slotNum; i++) {
                memcpy(&temp, page + PAGE_SIZE - 4 - (rid.slotNum * 8), 4);
                temp += sizeDifference;
                memcpy(page + PAGE_SIZE - 4 - (rid.slotNum * 8), &temp, 4);
            }

            //update the spaceStart in the directory
            spaceStart += sizeDifference;
            memcpy(page + PAGE_SIZE - 4, &spaceStart, 4);
        }else{//page does not have space for the update


        }
    }
    return fileHandle.writePage(rid.pageNum, page);
}
