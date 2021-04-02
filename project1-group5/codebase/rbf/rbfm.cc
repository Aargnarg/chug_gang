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
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
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

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    return unlink(fileName.c_str());
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
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

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    fileHandle.file.close();
    if(fileHandle.file.good()){
        return 0;
    } else {
        return -1;
    }
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
