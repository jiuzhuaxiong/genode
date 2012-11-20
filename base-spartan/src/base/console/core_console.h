/*
 * \brief  Spartan-specific implementation of Core_console
 * \author Tobias Börtitz
 * \date   2012-08-14
 */

/*
 * Copyright (C) 2010-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <base/console.h>

/* Spartan includes */
#include <spartan/syscalls.h>

namespace Genode
{

	class Core_console : public Console
	{
		private:
			enum {
				IO_PORT_SERIO_0	= 0x3F8,
			};

		public:
			Core_console() {
				Spartan::io_port_enable(IO_PORT_SERIO_0, 8);
			}

		protected:
			void _out_char(char c) {
				asm volatile ("outb %b0, %w1" : : "a" (c),
				              "Nd" (IO_PORT_SERIO_0));
			}

	};
}
