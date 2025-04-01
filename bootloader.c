#include <efi.h>
#include <efilib.h>

// Kernel entry point prototype
typedef void (*KernelMain)(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *KeyboardProtocol);

// UEFI application entry point
EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    // Initialize UEFI library
    InitializeLib(ImageHandle, SystemTable);
    
    // Clear the screen
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    
    // Print welcome message
    Print(L"Kontact UEFI Bootloader\n\r");
    Print(L"Loading kernel...\n\r");

    // Get handle to our own image
    EFI_LOADED_IMAGE *LoadedImage;
    EFI_STATUS Status = uefi_call_wrapper(
        SystemTable->BootServices->HandleProtocol,
        3,
        ImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (void **)&LoadedImage
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error getting loaded image: %r\n", Status);
        return Status;
    }
    
    // Get the file system
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
    Status = uefi_call_wrapper(
        SystemTable->BootServices->HandleProtocol,
        3,
        LoadedImage->DeviceHandle,
        &gEfiSimpleFileSystemProtocolGuid,
        (void **)&FileSystem
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error getting file system: %r\n", Status);
        return Status;
    }
    
    // Open the root directory
    EFI_FILE *Root;
    Status = uefi_call_wrapper(
        FileSystem->OpenVolume,
        2,
        FileSystem,
        &Root
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error opening root directory: %r\n", Status);
        return Status;
    }
    
    // Open the kernel file
    EFI_FILE *KernelFile;
    Status = uefi_call_wrapper(
        Root->Open,
        5,
        Root,
        &KernelFile,
        L"kernel.bin",
        EFI_FILE_MODE_READ,
        0
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error opening kernel file: %r\n", Status);
        return Status;
    }
    
    // Get file information to determine its size
    UINTN FileInfoSize = 0;
    EFI_FILE_INFO *FileInfo = NULL;
    
    Status = uefi_call_wrapper(
        KernelFile->GetInfo,
        4,
        KernelFile,
        &gEfiFileInfoGuid,
        &FileInfoSize,
        NULL
    );
    
    if (Status == EFI_BUFFER_TOO_SMALL) {
        // Allocate buffer for file info
        Status = uefi_call_wrapper(
            SystemTable->BootServices->AllocatePool,
            3,
            EfiLoaderData,
            FileInfoSize,
            (void **)&FileInfo
        );
        
        if (EFI_ERROR(Status)) {
            Print(L"Error allocating memory for file info: %r\n", Status);
            return Status;
        }
        
        // Get file info
        Status = uefi_call_wrapper(
            KernelFile->GetInfo,
            4,
            KernelFile,
            &gEfiFileInfoGuid,
            &FileInfoSize,
            FileInfo
        );
        
        if (EFI_ERROR(Status)) {
            Print(L"Error getting file info: %r\n", Status);
            return Status;
        }
    } else if (EFI_ERROR(Status)) {
        Print(L"Error getting file info size: %r\n", Status);
        return Status;
    }
    
    // Allocate memory for the kernel
    UINTN Pages = (FileInfo->FileSize + 0xFFF) / 0x1000;
    EFI_PHYSICAL_ADDRESS KernelAddress = 0x100000;  // 1MB mark
    
    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePages,
        4,
        AllocateAddress,
        EfiLoaderData,
        Pages,
        &KernelAddress
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error allocating memory for kernel: %r\n", Status);
        return Status;
    }
    
    // Read the kernel into memory
    UINTN KernelSize = FileInfo->FileSize;
    
    Status = uefi_call_wrapper(
        KernelFile->Read,
        3,
        KernelFile,
        &KernelSize,
        (void *)KernelAddress
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error reading kernel file: %r\n", Status);
        return Status;
    }
    
    // Close the kernel file
    uefi_call_wrapper(KernelFile->Close, 1, KernelFile);
    
    // Free file info
    uefi_call_wrapper(SystemTable->BootServices->FreePool, 1, FileInfo);
    
    // Get keyboard protocol for the shell
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL *KeyboardProtocol = SystemTable->ConIn;
    
    // Exit boot services
    UINTN MapKey = 0;
    UINTN MemoryMapSize = 0;
    UINTN DescriptorSize = 0;
    UINT32 DescriptorVersion = 0;
    EFI_MEMORY_DESCRIPTOR *MemoryMap = NULL;
    
    // Get memory map size
    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap,
        5,
        &MemoryMapSize,
        MemoryMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );
    
    // Allocate memory for memory map
    Status = uefi_call_wrapper(
        SystemTable->BootServices->AllocatePool,
        3,
        EfiLoaderData,
        MemoryMapSize,
        (void **)&MemoryMap
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error allocating memory for memory map: %r\n", Status);
        return Status;
    }
    
    // Get memory map
    Status = uefi_call_wrapper(
        SystemTable->BootServices->GetMemoryMap,
        5,
        &MemoryMapSize,
        MemoryMap,
        &MapKey,
        &DescriptorSize,
        &DescriptorVersion
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error getting memory map: %r\n", Status);
        return Status;
    }
    
    // Exit boot services
    Status = uefi_call_wrapper(
        SystemTable->BootServices->ExitBootServices,
        2,
        ImageHandle,
        MapKey
    );
    
    if (EFI_ERROR(Status)) {
        Print(L"Error exiting boot services: %r\n", Status);
        return Status;
    }
    
    // Jump to kernel with keyboard protocol
    KernelMain kernel = (KernelMain)KernelAddress;
    kernel(KeyboardProtocol);
    
    // We should never get here
    return EFI_SUCCESS;
}
