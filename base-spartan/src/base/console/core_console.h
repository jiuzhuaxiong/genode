#include <spartan/syscalls.h>

#include <base/console.h>

namespace Genode
{

	class Core_console : public Console
	{
		private:
			enum {
				IO_PORT_SERIO_0	= 0x3F8,
			};
		public:
			Core_console()
			{
				Spartan::io_port_enable(IO_PORT_SERIO_0, 8);
			}

		protected:
			void _out_char(char c)
			{
				asm volatile ("outb %b0, %w1" : : "a" (c), 
					"Nd" (IO_PORT_SERIO_0));
			}

	};
}
