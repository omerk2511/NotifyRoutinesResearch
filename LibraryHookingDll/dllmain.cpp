#include <windows.h>

#include "inline_hook.h"

InlineHook* g_inline_hook;

typedef
NTSYSCALLAPI
NTSTATUS
(*NtCreateThreadExPtr)(
    OUT PHANDLE hThread,
    IN ACCESS_MASK DesiredAccess,
    IN PVOID ObjectAttributes,
    IN HANDLE ProcessHandle,
    IN PVOID lpStartAddress,
    IN PVOID lpParameter,
    IN ULONG Flags,
    IN SIZE_T StackZeroBits,
    IN SIZE_T SizeOfStackCommit,
    IN SIZE_T SizeOfStackReserve,
    OUT PVOID lpBytesBuffer
    );

NTSTATUS
NTAPI
NtCreateThreadExHook(
    OUT PHANDLE hThread,
    IN ACCESS_MASK DesiredAccess,
    IN PVOID ObjectAttributes,
    IN HANDLE ProcessHandle,
    IN PVOID lpStartAddress,
    IN PVOID lpParameter,
    IN ULONG Flags,
    IN SIZE_T StackZeroBits,
    IN SIZE_T SizeOfStackCommit,
    IN SIZE_T SizeOfStackReserve,
    OUT PVOID lpBytesBuffer
)
{
    MessageBoxA(nullptr, "NtCreateThreadEx has been called.", "Hi!", 0);

    NTSTATUS result = reinterpret_cast<NtCreateThreadExPtr>(
        g_inline_hook->get_duplicate_function())(
            hThread,
            DesiredAccess,
            ObjectAttributes,
            ProcessHandle,
            lpStartAddress,
            lpParameter,
            Flags,
            StackZeroBits,
            SizeOfStackCommit,
            SizeOfStackReserve,
            lpBytesBuffer
            );

    return result;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        try {
            void* original_function = ::GetProcAddress(
                ::GetModuleHandleA("ntdll"),
                "NtCreateThreadEx"
            );

            g_inline_hook = new InlineHook(original_function, NtCreateThreadExHook);
        } catch (...) {
            return false;
        }

        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (g_inline_hook) {
            delete g_inline_hook;
        }

        break;
    }

    default:
        break;
    }

    return true;
}