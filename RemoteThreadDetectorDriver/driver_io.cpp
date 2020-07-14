#include "driver_io.h"

NTSTATUS driver_io::create_close(PDEVICE_OBJECT, PIRP irp)
{
	IrpHandler irp_handler(irp);
	return irp_handler.get_status();
}

NTSTATUS driver_io::read(PDEVICE_OBJECT, PIRP irp)
{
	IrpHandler irp_handler(irp);

	auto stack = irp_handler.get_stack_location();
	auto length = stack->Parameters.Read.Length;

	auto buffer = ::MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);

	if (!buffer) {
		irp_handler.set_status(STATUS_INSUFFICIENT_RESOURCES);
		return irp_handler.get_status();
	}

	auto count = 0;

	while (count + sizeof(RemoteThreadCreation) <= length) {
		auto entry = g_remote_thread_creation_list->remove_remote_thread_creation();

		if (!entry) {
			break;
		}

		::memcpy(
			static_cast<char*>(buffer) + count,
			&entry->remote_thread_creation,
			sizeof(RemoteThreadCreation)
		);

		delete entry;
		count += sizeof(RemoteThreadCreation);
	}

	irp_handler.set_information(count);
	return irp_handler.get_status();
}