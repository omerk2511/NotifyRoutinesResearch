#include "apc_routines.h"

void apc_routines::kernel_free_kapc(PKAPC apc, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*)
{
	delete apc;
}

void apc_routines::rundown_free_kapc(PKAPC apc)
{
	delete apc;

	::ExReleaseRundownProtection(&g_rundown_protection);
}

void apc_routines::normal_inject_code(PVOID, PVOID, PVOID)
{
	void* address{ };
	size_t shellcode_size = sizeof(injected_shellcode);

	auto status = ::ZwAllocateVirtualMemory(
		NtCurrentProcess(),
		&address,
		0,
		&shellcode_size,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READ
	);

	if (!NT_SUCCESS(status)) {
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not allocate memory in new process: 0x%p.\n", status));

		return;
	}

	void* protection_address = address;
	ULONG protection_size = static_cast<ULONG>(shellcode_size);
	ULONG old_protection;

	status = ::ZwProtectVirtualMemory(
		NtCurrentProcess(),
		&protection_address,
		&protection_size,
		PAGE_READWRITE,
		&old_protection
	);

	if (!NT_SUCCESS(status)) {
		::ZwFreeVirtualMemory(
			NtCurrentProcess(),
			&address,
			&shellcode_size,
			MEM_FREE | MEM_RELEASE
		);

		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not protect virtual memory: 0x%p.\n", status));

		return;
	}

	::memcpy_s(address, sizeof(injected_shellcode), injected_shellcode, sizeof(injected_shellcode));

	status = ::ZwProtectVirtualMemory(
		NtCurrentProcess(),
		&protection_address,
		&protection_size,
		PAGE_EXECUTE_READ,
		&old_protection
	);

	if (!NT_SUCCESS(status)) {
		::ZwFreeVirtualMemory(
			NtCurrentProcess(),
			&address,
			&shellcode_size,
			MEM_FREE | MEM_RELEASE
		);

		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not protect virtual memory: 0x%p.\n", status));

		return;
	}

	PKAPC apc = new (NonPagedPool, config::kDriverTag) KAPC;

	if (!apc) {
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not queue a user APC due to insufficient memory.\n"));

		return;
	}

	::KeInitializeApc(
		apc,
		::KeGetCurrentThread(),
		OriginalApcEnvironment,
		&apc_routines::kernel_free_kapc,
		&apc_routines::rundown_free_kapc,
		reinterpret_cast<PKNORMAL_ROUTINE>(address),
		UserMode,
		nullptr
	);

	auto inserted = ::KeInsertQueueApc(
		apc,
		nullptr,
		nullptr,
		0
	);

	if (!inserted) {
		delete apc;
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not insert a user APC.\n"));

		return;
	}

	KdPrint(("[+] Injected code and queued an APC successfully (pid=%d).\n", ::PsGetCurrentProcessId()));

	::ExReleaseRundownProtection(&g_rundown_protection);
}