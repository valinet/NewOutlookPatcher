#include "global.h"
#include "binary/dropper.h"

#define CUSTOM_DRIVER L"CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    {
        //if (DropDriverFromBytes(DriverPath))
        {
            Status = WindLoadDriver((PWCHAR)L"", (PWCHAR)CUSTOM_DRIVER, FALSE);

            if (NT_SUCCESS(Status))
                printf("Driver loaded successfully\n");
        }
    }
    {
        // Unload driver
        Status = WindUnloadDriver((PWCHAR)CUSTOM_DRIVER, 0);
        if (NT_SUCCESS(Status))
            printf("Driver unloaded successfully\n");
    }
    if (!NT_SUCCESS(Status))
        printf("Error: %08X\n", Status);
    return true;
}
