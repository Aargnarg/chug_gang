
- Modify the "CODEROOT" variable in makefile.inc to point to the root
  of your code base if you can't compile the code.
 
- Implement the Record-Based Files (RBF) Component:

   Go to folder "rbf" and type in:

    make clean
    make
    ./rbftest1


   The program should run. But it will generate an error. You are supposed to
   implement the API of the Paged File Manager defined in pfm.h and some
   of the methods in rbfm.h, as explained in the project description.

- By default you should not change those functions of the PagedFileManager,
  FileHandle, and RecordBasedFileManager classes defined in rbf/pfm.h and rbf/rbfm.h.
  If you think that some changes are really necessary, please contact us first.
