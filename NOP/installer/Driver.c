#include <ntifs.h>

volatile int i = 0;
#define PATH_SRC L"\\DosDevices\\AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
#define PATH_DST L"\\DosDevices\\BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
#define BUFSIZ 1024
#define BUFTAG 'ilav'
// echo load | GDRVLoader.exe DrvCopyFile.sys & echo unload | GDRVLoader.exe DrvCopyFile.sys

NTSTATUS DriverUnload(_In_ PDRIVER_OBJECT driverObject) {
    UNREFERENCED_PARAMETER(driverObject);

    DbgPrint(("NewOutlookPatcher: DriverUnload\n"));
    return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath) {
    UNREFERENCED_PARAMETER(registryPath);

    DbgPrint(("NewOutlookPatcher: DriverEntry\n"));
    driverObject->DriverUnload = DriverUnload;

    NTSTATUS rv = STATUS_SUCCESS;

    UNICODE_STRING szSrcName;
    RtlInitUnicodeString(&szSrcName, PATH_SRC);
    OBJECT_ATTRIBUTES oaSrcName;
    InitializeObjectAttributes(&oaSrcName, &szSrcName, OBJ_CASE_INSENSITIVE, NULL, NULL);
    UNICODE_STRING szDstName;
    RtlInitUnicodeString(&szDstName, PATH_DST);
    OBJECT_ATTRIBUTES oaDstName;
    InitializeObjectAttributes(&oaDstName, &szDstName, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (PATH_SRC[12 + i] == L'Z' && PATH_SRC[13 + i] == L'w' && PATH_SRC[14 + i] == L'D' && PATH_SRC[15 + i] == L'e' && PATH_SRC[16 + i] == L'l' && PATH_SRC[17 + i] == L'e' && PATH_SRC[18 + i] == L't' && PATH_SRC[19 + i] == L'e' && PATH_SRC[20 + i] == L'F' && PATH_SRC[21 + i] == L'i' && PATH_SRC[22 + i] == L'l' && PATH_SRC[23 + i] == L'e' && PATH_SRC[24 + i] == L'\0') {
        
        rv = ZwDeleteFile(&oaDstName);
        DbgPrint("NewOutlookPatcher: ZwDeleteFile Src: %S (%d)\n", PATH_DST, rv);
    }
    else {

        HANDLE hSrc;
        IO_STATUS_BLOCK iosbSrcCreate;
        rv = ZwCreateFile(&hSrc, GENERIC_READ, &oaSrcName, &iosbSrcCreate, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
        DbgPrint("NewOutlookPatcher: ZwCreateFile Src: %S (%d)\n", PATH_SRC, rv);
        if (NT_SUCCESS(rv)) {

            HANDLE hDst;
            IO_STATUS_BLOCK iosbDstCreate;
            rv = ZwCreateFile(&hDst, GENERIC_WRITE, &oaDstName, &iosbDstCreate, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
            DbgPrint("NewOutlookPatcher: ZwCreateFile Dst: %S (%d)\n", PATH_DST, rv);
            if (NT_SUCCESS(rv)) {

                PVOID buffer = ExAllocatePool2(POOL_FLAG_PAGED, BUFSIZ, BUFTAG);
                DbgPrint("NewOutlookPatcher: ExAllocatePool2: %p\n", buffer);
                if (buffer) {

                    LARGE_INTEGER liReadPos, liWritePos;
                    liReadPos.QuadPart = 0;
                    liWritePos.QuadPart = 0;

                    while (NT_SUCCESS(rv)) {
                        IO_STATUS_BLOCK iosbSrcRead;
                        rv = ZwReadFile(hSrc, NULL, NULL, NULL, &iosbSrcRead, buffer, BUFSIZ, &liReadPos, NULL);
                        //DbgPrint(("ZwReadFile Src: %x\n", rv));
                        if (NT_SUCCESS(rv)) {

                            liReadPos.QuadPart += iosbSrcRead.Information;
                            IO_STATUS_BLOCK iosbSrcWrite;
                            rv = ZwWriteFile(hDst, NULL, NULL, NULL, &iosbSrcWrite, buffer, (ULONG)iosbSrcRead.Information, &liWritePos, NULL);
                            //DbgPrint(("ZwWriteFile Dst: %x\n", rv));
                            if (NT_SUCCESS(rv)) {

                                liWritePos.QuadPart += iosbSrcWrite.Information;
                            }
                        }
                    }

                    ExFreePoolWithTag(buffer, BUFTAG);
                }

                ZwClose(hDst);
            }
            ZwClose(hSrc);
        }
    }

    return STATUS_SUCCESS;
}
