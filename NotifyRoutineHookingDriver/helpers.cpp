#include "helpers.h"

void test_create_thread_notify_routine(HANDLE, HANDLE, BOOLEAN) { }

undocumented::PSP_CALLBACK_OBJECT** helpers::find_PspCreateThreadNotifyRoutine()
{
	ULONG modules_size{ };

	auto status = ::AuxKlibQueryModuleInformation(
		&modules_size,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		nullptr
	);

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not query module information.\n"));
		return nullptr;
	}

	if (modules_size == 0) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not find any modules.\n"));
		return nullptr;
	}

	auto modules = reinterpret_cast<AUX_MODULE_EXTENDED_INFO*>(
		::ExAllocatePoolWithTag(PagedPool, modules_size, config::kDriverTag));

	auto modules_amount = modules_size / sizeof(AUX_MODULE_EXTENDED_INFO);

	if (!modules) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not allocate memory.\n"));
		return nullptr;
	}

	RtlZeroMemory(modules, modules_size);

	status = ::AuxKlibQueryModuleInformation(
		&modules_size,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		modules
	);

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not query module information.\n"));
		return nullptr;
	}

	char ntos_name[] = "ntoskrnl.exe";
	void* ntos_base = nullptr;

	for (size_t i = 0; i < modules_amount; i++) {
		auto module = &modules[i];
		auto module_name = reinterpret_cast<char*>(module->FullPathName) + module->FileNameOffset;

		auto equal_bytes = ::RtlCompareMemory(ntos_name, module_name, sizeof(ntos_name));

		if (equal_bytes == sizeof(ntos_name)) {
			ntos_base = module->BasicInfo.ImageBase;
			break;
		}
	}

	delete modules;

	if (!ntos_base) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not find ntoskrnl.exe.\n"));
		return nullptr;
	}

	auto pe_base = reinterpret_cast<char*>(ntos_base);

	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(pe_base);
	auto nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(pe_base + dos_header->e_lfanew);

	auto current_section_header = reinterpret_cast<PIMAGE_SECTION_HEADER>(
		pe_base + sizeof(IMAGE_DOS_HEADER) + sizeof(ULONG) +
		sizeof(IMAGE_FILE_HEADER) + nt_headers->FileHeader.SizeOfOptionalHeader);

	char data_section_name[] = ".data";
	auto data_section_name_length = sizeof(data_section_name);

	void* data_base = nullptr;
	size_t data_size = 0;

	for (auto i = 0; i < nt_headers->FileHeader.NumberOfSections; i++) {
		auto current_section_name = reinterpret_cast<char*>(current_section_header->Name);
		auto current_section_name_length = ::strlen(current_section_name) + 1;

		if (current_section_name_length == data_section_name_length) {
			auto match = ::RtlCompareMemory(current_section_name, data_section_name,
				current_section_name_length);

			if (match == current_section_name_length) {
				data_base = pe_base + current_section_header->VirtualAddress;
				data_size = current_section_header->Misc.VirtualSize;

				break;
			}
		}

		current_section_header++;
	}

	if (!data_base || data_size == 0) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not find ntoskrnl.exe data section.\n"));
		return nullptr;
	}

	void* data_end = reinterpret_cast<char*>(data_base) + data_size;

	status = ::PsSetCreateThreadNotifyRoutine(test_create_thread_notify_routine);

	if (!NT_SUCCESS(status)) {
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not set a create thread notify routine.\n"));
		return nullptr;
	}

	auto test_callback = new (NonPagedPool, config::kDriverTag) undocumented::PSP_CALLBACK_OBJECT;

	if (!test_callback) {
		::PsRemoveCreateThreadNotifyRoutine(test_create_thread_notify_routine);
		KdPrint(("[-] Failed finding PspCreateThreadNotifyRoutine: could not allocate non-paged pool memory.\n"));
		return nullptr;
	}

	RtlZeroMemory(test_callback, sizeof(undocumented::PSP_CALLBACK_OBJECT));

	undocumented::PSP_CALLBACK_OBJECT** PspCreateThreadNotifyRoutine{ };

	for (auto current = reinterpret_cast<undocumented::PSP_CALLBACK_OBJECT**>(data_base); current < data_end; current++) {
		MM_COPY_ADDRESS checked_address = { 0 };
		checked_address.VirtualAddress = reinterpret_cast<undocumented::PSP_CALLBACK_OBJECT*>(
			*reinterpret_cast<ULONG64*>(current) & 0xfffffffffffffff0);

		size_t bytes_copied{ };

		status = ::MmCopyMemory(
			test_callback,
			checked_address,
			24,
			MM_COPY_MEMORY_VIRTUAL,
			&bytes_copied
		);

		if (NT_SUCCESS(status)) {
			if (test_callback->notify_routine == reinterpret_cast<PVOID>(&test_create_thread_notify_routine)) {
				PspCreateThreadNotifyRoutine = current;

				while (*(PspCreateThreadNotifyRoutine - 1) != nullptr) {
					PspCreateThreadNotifyRoutine--;
				}

				break;
			}
		}

		RtlZeroMemory(test_callback, sizeof(undocumented::PSP_CALLBACK_OBJECT));
	}

	::PsRemoveCreateThreadNotifyRoutine(test_create_thread_notify_routine);
	delete test_callback;

	return PspCreateThreadNotifyRoutine;
}

char* helpers::get_name_of_owning_driver(void* address)
{
	ULONG modules_size{ };

	auto status = ::AuxKlibQueryModuleInformation(
		&modules_size,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		nullptr
	);

	if (!NT_SUCCESS(status)) {
		return nullptr;
	}

	if (modules_size == 0) {
		return nullptr;
	}

	auto modules = reinterpret_cast<AUX_MODULE_EXTENDED_INFO*>(
		::ExAllocatePoolWithTag(PagedPool, modules_size, config::kDriverTag));

	auto modules_amount = modules_size / sizeof(AUX_MODULE_EXTENDED_INFO);

	if (!modules) {
		return nullptr;
	}

	RtlZeroMemory(modules, modules_size);

	status = ::AuxKlibQueryModuleInformation(
		&modules_size,
		sizeof(AUX_MODULE_EXTENDED_INFO),
		modules
	);

	if (!NT_SUCCESS(status)) {
		return nullptr;
	}

	char* driver_name = nullptr;

	for (size_t i = 0; i < modules_amount; i++) {
		auto module = &modules[i];

		auto module_base = reinterpret_cast<void*>(module->BasicInfo.ImageBase);
		auto module_end = reinterpret_cast<void*>(reinterpret_cast<char*>(module_base) + module->ImageSize);

		if (address >= module_base && address < module_end) {
			driver_name = reinterpret_cast<char*>(
				::ExAllocatePoolWithTag(PagedPool, AUX_KLIB_MODULE_PATH_LEN, config::kDriverTag));
			RtlZeroMemory(driver_name, AUX_KLIB_MODULE_PATH_LEN);

			if (driver_name) {
				auto module_name = reinterpret_cast<char*>(module->FullPathName) + module->FileNameOffset;
				auto module_name_size = AUX_KLIB_MODULE_PATH_LEN - module->FileNameOffset;

				::memcpy_s(driver_name, module_name_size, module_name, module_name_size);
			}

			break;
		}
	}

	::ExFreePoolWithTag(modules, config::kDriverTag);

	return driver_name;
}