#pragma warning(disable: 4100)
#include "kernel_modules.h"

VOID ListKernelDrivers(VOID)
{
    PLIST_ENTRY list = &PsLoadedModuleList;
    PLIST_ENTRY current = list->Flink;

    while (current != list)
    {
        PKLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(current, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

        DbgPrintEx(DPFLTR_DEFAULT_ID, DPFLTR_INFO_LEVEL,
            "Driver: %wZ | Base: %p | Size: 0x%X\n",
            &entry->BaseDllName,
            entry->DllBase,
            entry->SizeOfImage);

        current = current->Flink;
    }
}

