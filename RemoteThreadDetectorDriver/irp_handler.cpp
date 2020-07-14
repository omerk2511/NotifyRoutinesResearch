#include "irp_handler.h"

IrpHandler::IrpHandler(PIRP irp, NTSTATUS status, ULONG_PTR information)
	: irp_(irp)
{
	irp_->IoStatus.Status = status;
	irp_->IoStatus.Information = information;
}

IrpHandler::~IrpHandler()
{
	::IoCompleteRequest(irp_, IO_NO_INCREMENT);
}

void IrpHandler::set_status(NTSTATUS status)
{
	irp_->IoStatus.Status = status;
}

NTSTATUS IrpHandler::get_status() const
{
	return irp_->IoStatus.Status;
}

void IrpHandler::set_information(ULONG_PTR information)
{
	irp_->IoStatus.Information = information;
}

ULONG_PTR IrpHandler::get_information() const
{
	return irp_->IoStatus.Information;
}

PIO_STACK_LOCATION IrpHandler::get_stack_location() const
{
	return ::IoGetCurrentIrpStackLocation(irp_);
}

PIRP IrpHandler::operator->() const
{
	return irp_;
}