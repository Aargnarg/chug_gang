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

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {





    return -1;
    //  Format of the data passed into the function is the following:
    //  [n byte-null-indicators for y fields] [actual value for the first field] [actual value for the second field] ...
    //  1) For y fields, there is n-byte-null-indicators in the beginning of each record.
    //     The value n can be calculated as: ceil(y / 8). (e.g., 5 fields => ceil(5 / 8) = 1. 12 fields => ceil(12 / 8) = 2.)
    //     Each bit represents whether each field value is null or not.
    //     If k-th bit from the left is set to 1, k-th field value is null. We do not include anything in the actual data part.
    //     If k-th bit from the left is set to 0, k-th field contains non-null values.
    //     If there are more than 8 fields, then you need to find the corresponding byte first,
    //     then find a corresponding bit inside that byte.
    //  2) Actual data is a concatenation of values of the attributes.
    //  3) For Int and Real: use 4 bytes to store the value;
    //     For Varchar: use 4 bytes to store the length of characters, then store the actual characters.
    //  !!! The same format is used for updateRecord(), the returned data of readRecord(), and readAttribute().
    // For example, refer to the Q6 of Project 1 Environment document.
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
