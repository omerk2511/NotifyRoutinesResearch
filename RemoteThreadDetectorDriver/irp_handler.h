#pragma once

#include <ntddk.h>

class IrpHandler
{
public:
	IrpHandler(PIRP irp, NTSTATUS status = STATUS_SUCCESS, ULONG_PTR information = 0);
	~IrpHandler();

	void set_status(NTSTATUS status);
	NTSTATUS get_status() const;

	void set_information(ULONG_PTR information);
	ULONG_PTR get_information() const;

	PIO_STACK_LOCATION get_stack_location() const;

	PIRP operator->() const;

private:
	PIRP irp_;
};