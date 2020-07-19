#pragma once

#include <windows.h>
#include <exception>
#include <vector>
#include <cstddef>

const size_t FUNCTION_SIZE = 25;

const char TRAMPOLINE_SHELLCODE[] = "\x48\xb8\x00\x00\x00\x00\x00\x00\x00\x00"
									"\xff\xe0";

class InlineHook
{
public:
	InlineHook(void* original_function, void* hook_function);
	~InlineHook();

	InlineHook(const InlineHook&) = delete;
	InlineHook& operator=(const InlineHook&) = delete;

	void* get_original_function() { return original_function_; }
	void* get_duplicate_function() { return duplicate_function_; }

private:
	void* original_function_;
	void* hook_function_;
	void* duplicate_function_;

	void duplicate_original();
	void insert_trampoline();
	void restore_original();
};