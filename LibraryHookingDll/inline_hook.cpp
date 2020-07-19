#include "inline_hook.h"

InlineHook::InlineHook(void* original_function, void* hook_function)
	: original_function_(original_function),
	hook_function_(hook_function)
{
	duplicate_original();
	insert_trampoline();
}

InlineHook::~InlineHook()
{
	try {
		restore_original();
	} catch (...) {
		// ignore
	}
}

void InlineHook::duplicate_original()
{
	duplicate_function_ = ::VirtualAlloc(
		nullptr,
		FUNCTION_SIZE,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READWRITE
	);

	if (!duplicate_function_) {
		throw std::exception("Could not allocate memory for the duplicate function.");
	}

	::memcpy_s(duplicate_function_, FUNCTION_SIZE, original_function_, FUNCTION_SIZE);
}

void InlineHook::insert_trampoline()
{
	std::vector<std::byte> jump_shellcode;

	for (const auto byte : TRAMPOLINE_SHELLCODE)
		jump_shellcode.push_back(static_cast<std::byte>(byte));

	::memcpy_s(&jump_shellcode[2], sizeof(void*), &hook_function_, sizeof(void*));

	DWORD old_protect;

	auto success = ::VirtualProtect(
		original_function_,
		jump_shellcode.size(),
		PAGE_EXECUTE_READWRITE,
		&old_protect
	);

	if (!success) {
		throw std::exception("Could not make the function memory writable.");
	}

	::memcpy_s(
		original_function_,
		jump_shellcode.size(),
		jump_shellcode.data(),
		jump_shellcode.size()
	);

	::VirtualProtect(
		original_function_,
		jump_shellcode.size(),
		old_protect,
		nullptr
	);
}

void InlineHook::restore_original()
{
	DWORD old_protect;

	auto success = ::VirtualProtect(
		original_function_,
		FUNCTION_SIZE,
		PAGE_EXECUTE_READWRITE,
		&old_protect
	);

	if (!success) {
		throw std::exception("Could not make the function memory writable.");
	}

	::memcpy_s(
		original_function_,
		FUNCTION_SIZE,
		duplicate_function_,
		FUNCTION_SIZE
	);

	::VirtualProtect(
		original_function_,
		FUNCTION_SIZE,
		old_protect,
		nullptr
	);

	::VirtualFree(
		duplicate_function_,
		0,
		MEM_RELEASE
	);
}