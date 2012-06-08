#include <base/cancelable_lock.h>

using namespace Genode;

void Cancelable_lock::Applicant::wake_up()
{}

void Cancelable_lock::lock()
{}

void Cancelable_lock::unlock()
{}

Cancelable_lock::Cancelable_lock(Genode::Cancelable_lock::State) :
	_last_applicant(0), _owner(0)
{}
