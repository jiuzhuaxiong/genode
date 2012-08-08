/*
 * \brief  Simple Driver for the ARM Cortex A9
 * \author Martin stein
 * \date   2011-11-03
 */

/*
 * Copyright (C) 2011-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#ifndef _INCLUDE__DRIVERS__CPU__CORTEX_A9__CORE_H_
#define _INCLUDE__DRIVERS__CPU__CORTEX_A9__CORE_H_

/* Genode includes */
#include <util/register.h>
#include <util/mmio.h>
#include <drivers/board.h>
#include <drivers/cpu/cortex_a9/timer.h>

namespace Genode
{
	class Section_table;

	/**
	 * Cortex A9 driver
	 */
	struct Cortex_a9
	{
		enum
		{
			/* common */
			DATA_ACCESS_ALIGNM = 4,
			CLK = Board::CORTEX_A9_CLOCK, /* CPU interface clock */
			PERIPH_CLK = CLK,             /* clock for CPU internal components */
			MIN_PAGE_SIZE_LOG2 = 12,
			MAX_PAGE_SIZE_LOG2 = 20,
			HIGHEST_EXCEPTION_ENTRY = 0xffff0000,

			/* interrupt controller */
			PL390_DISTRIBUTOR_MMIO_BASE = Board::CORTEX_A9_PRIVATE_MEM_BASE + 0x1000,
			PL390_DISTRIBUTOR_MMIO_SIZE = 0x1000,
			PL390_CPU_MMIO_BASE = Board::CORTEX_A9_PRIVATE_MEM_BASE + 0x100,
			PL390_CPU_MMIO_SIZE = 0x100,

			/* timer */
			PRIVATE_TIMER_MMIO_BASE = Board::CORTEX_A9_PRIVATE_MEM_BASE + 0x600,
			PRIVATE_TIMER_MMIO_SIZE = 0x10,
			PRIVATE_TIMER_IRQ = 29,
			PRIVATE_TIMER_CLK = PERIPH_CLK
		};

		/**
		 * CPU local timer module
		 */
		class Private_timer : public Cortex_a9_timer<PRIVATE_TIMER_CLK>
		{
			public:

				enum { IRQ = PRIVATE_TIMER_IRQ };

				/**
				 * Constructor
				 */
				Private_timer() :
					Cortex_a9_timer<PRIVATE_TIMER_CLK>(PRIVATE_TIMER_MMIO_BASE)
				{ }
		};

		typedef Cortex_a9_timer<PERIPH_CLK> Timer;

		/**
		 * Common parts of fault status registers
		 */
		struct Fsr : Register<32>
		{
			/**
			 * Fault status encoding
			 */
			enum Fault_status {
				SECTION_TRANSLATION_FAULT = 5,
				PAGE_TRANSLATION_FAULT    = 7,
			};

			struct Fs_3_0 : Bitfield<0, 4> { }; /* fault status bits [3:0] */
			struct Fs_4 : Bitfield<10, 1> { };  /* fault status bits [4] */
		};

		/**
		 * Instruction fault status register
		 */
		struct Ifsr : Fsr
		{
			/**
			 * Read register
			 */
			static access_t read() {
				access_t v;
				asm volatile ("mrc p15, 0, %[v], c5, c0, 1\n" : [v]"=r"(v) :: );
				return v;
			}

			/**
			 * Read fault status
			 */
			static Fault_status fault_status() {
				access_t const v = read();
				return (Fault_status)(Fs_3_0::get(v) |
				                     (Fs_4::get(v) << Fs_3_0::WIDTH));
			}
		};

		/**
		 * Instruction fault address register
		 */
		struct Ifar : Register<32>
		{
			/**
			 * Read register
			 */
			static access_t read() {
				access_t v;
				asm volatile ("mrc p15, 0, %[v], c6, c0, 2\n" : [v]"=r"(v) :: );
				return v;
			}
		};

		/**
		 * Data fault status register
		 */
		struct Dfsr : Fsr
		{
			struct Wnr : Bitfield<11, 1> { }; /* write not read bit */

			/**
			 * Read register
			 */
			static access_t read() {
				access_t v;
				asm volatile ("mrc p15, 0, %[v], c5, c0, 0\n" : [v]"=r"(v) :: );
				return v;
			}

			/**
			 * Read fault status
			 */
			static Fault_status fault_status() {
				access_t const v = read();
				return (Fault_status)(Fs_3_0::get(v) | (4<<Fs_4::get(v)));
			}
		};

		/**
		 * Data fault address register
		 */
		struct Dfar : Register<32>
		{
			/**
			 * Read register
			 */
			static access_t read() {
				access_t v;
				asm volatile ("mrc p15, 0, %[v], c6, c0, 0\n" : [v]"=r"(v) :: );
				return v;
			}
		};

		/**
		 * Process identification register
		 */
		struct Contextidr : Register<32>
		{
			struct Asid   : Bitfield<0,8>  { }; /* ID part used by MMU */
			struct Procid : Bitfield<8,24> { }; /* ID part used by debug/trace */

			/**
			 * Write whole register
			 */
			static void write(access_t const v)
			{
				asm volatile ("mcr p15, 0, %[v], c13, c0, 1\n" :: [v]"r"(v) : );
			}
		};

		/**
		 * A system control register
		 */
		struct Sctlr : Register<32>
		{
			struct M    : Bitfield<0,1> { };  /* MMU enable bit */
			struct C    : Bitfield<2,1> { };  /* cache enable bit */
			struct I    : Bitfield<12,1> { }; /* instruction cache enable bit */
			struct V    : Bitfield<13,1> { }; /* exception vectors bit */

			/**
			 * Read whole register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("mrc p15, 0, %[v], c1, c0, 0\n" : [v]"=r"(v) :: );
				return v;
			};

			/**
			 * Write whole register
			 */
			static void write(access_t const v)
			{
				asm volatile ("mcr p15, 0, %[v], c1, c0, 0\n" :: [v]"r"(v) : );
			}
		};

		/**
		 * The translation table base control register
		 */
		struct Ttbcr : Register<32>
		{
			/**********************
			 ** Always available **
			 **********************/

			struct N : Bitfield<0,3>  /* base address width */
			{ };

			/********************************************
			 ** Only available with security extension **
			 ********************************************/

			struct Pd0 : Bitfield<4,1> { }; /* translation table walk disable bit for TTBR0 */
			struct Pd1 : Bitfield<5,1> { }; /* translation table walk disable bit for TTBR1 */

			/**
			 * Read whole register, only in privileged CPU mode
			 */
			static access_t read();

			/**
			 * Write whole register, only in privileged CPU mode
			 */
			static void write(access_t const v)
			{
				asm volatile ("mcr p15, 0, %[v], c2, c0, 2" :: [v]"r"(v) : );
			}
		};

		/**
		 * The domain access control register
		 */
		struct Dacr : Register<32>
		{
			enum Dx_values { NO_ACCESS = 0, CLIENT = 1, MANAGER = 3 };

			/**
			 * Access values for the 16 available domains
			 */
			struct D0  : Bitfield<0,2>   { };
			struct D1  : Bitfield<2,2>   { };
			struct D2  : Bitfield<4,2>   { };
			struct D3  : Bitfield<6,2>   { };
			struct D4  : Bitfield<8,2>   { };
			struct D5  : Bitfield<10,2>  { };
			struct D6  : Bitfield<12,2>  { };
			struct D7  : Bitfield<14,2>  { };
			struct D8  : Bitfield<16,2>  { };
			struct D9  : Bitfield<18,2>  { };
			struct D10 : Bitfield<20,2>  { };
			struct D11 : Bitfield<22,2>  { };
			struct D12 : Bitfield<24,2>  { };
			struct D13 : Bitfield<26,2>  { };
			struct D14 : Bitfield<28,2>  { };
			struct D15 : Bitfield<30,2>  { };

			/**
			 * Write whole register, only in privileged CPU mode
			 */
			static void write(access_t const v)
			{
				asm volatile ("mcr p15, 0, %[v], c3, c0, 0" :: [v]"r"(v) : );
			}
		};

		/**
		 * Translation table base register 0
		 *
		 * Typically for process specific spaces, references first level
		 * table with a size between 128B and 16KB according to 'Ttbcr::N'.
		 */
		struct Ttbr0 : Register<32>
		{
			/**********************
			 ** Always available **
			 **********************/

			struct S   : Bitfield<1,1>   { }; /* shareable bit */
			struct Rgn : Bitfield<3,2>        /* region bits */
			{
				enum { OUTER_NON_CACHEABLE                = 0b00,
				       OUTER_WBACK_WALLOCATE_CACHEABLE    = 0b01,
				       OUTER_WTHROUGH_CACHEABLE           = 0b10,
				       OUTER_WBACK_NO_WALLCOATE_CACHEABLE = 0b11,
				};
			};

			struct Nos          : Bitfield<5,1>   { }; /* not outer shareable bit */
			struct Base_address : Bitfield<14,18> { }; /* translation table base address
			                                              (Driver supports only 16KB alignment) */

			/***********************************************
			 ** Only available without security extension **
			 ***********************************************/

			struct C : Bitfield<0,1>   { }; /* cacheable bit */

			/********************************************
			 ** Only available with security extension **
			 ********************************************/

			struct Irgn_1 : Bitfield<0,1> /* inner region bit 0 */
			{
				enum { INNER_NON_CACHEABLE                = 0b0,
				       INNER_WBACK_WALLOCATE_CACHEABLE    = 0b0,
				       INNER_WTHROUGH_CACHEABLE           = 0b1,
				       INNER_WBACK_NO_WALLCOATE_CACHEABLE = 0b1,
				};
			};

			struct Irgn_0 : Bitfield<6,1> /* inner region bit 1 */
			{
				enum { INNER_NON_CACHEABLE                = 0b0,
				       INNER_WBACK_WALLOCATE_CACHEABLE    = 0b1,
				       INNER_WTHROUGH_CACHEABLE           = 0b0,
				       INNER_WBACK_NO_WALLCOATE_CACHEABLE = 0b1,
				};
			};

			/**
			 * Read whole register, only in privileged CPU mode
			 */
			static access_t read();

			/**
			 * Write whole register, only in privileged CPU mode
			 */
			static void write(access_t const v)
			{
				asm volatile ("mcr p15, 0, %[v], c2, c0, 0" :: [v]"r"(v) : );
			}
		};

		/**
		 * A current program status register
		 */
		struct Cpsr : Register<32>
		{
			struct M : Bitfield<0,5>       /* processor mode */
			{
				enum {                     /* <Privileged>, <Description> */
					USER       = 0b10000,  /* 0, application code */
					FIQ        = 0b10001,  /* 1, entered at fast interrupt */
					IRQ        = 0b10010,  /* 1, entered at normal interrupt */
					SUPERVISOR = 0b10011,  /* 1, most kernel code */
					MONITOR    = 0b10110,  /* 1, a secure mode, switch sec./non-sec. */
					ABORT      = 0b10111,  /* 1, entered at aborts */
					UNDEFINED  = 0b11011,  /* 1, entered at instruction-related error */
					SYSTEM     = 0b11111,  /* 1, applications that require privileged */
				};
			};
			struct F : Bitfield<6,1> { };  /* fast interrupt request disable */
			struct I : Bitfield<7,1> { };  /* interrupt request disable */
			struct A : Bitfield<8,1> { };  /* asynchronous abort disable */

			/**
			 * Read whole register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("mrs %[v], cpsr\n" : [v] "=r" (v) : : );
				return v;
			}

			/**
			 * Write whole register
			 */
			static void write(access_t & v)
			{
				asm volatile ("msr cpsr, %[v]\n" : : [v] "r" (v) : );
			}
		};

		/**
		 * Secure configuration register
		 */
		struct Scr : Register<32>
		{
			struct Ns : Bitfield<0, 1> { }; /* non secure bit */

			/**
			 * Read whole register
			 */
			static access_t read()
			{
				access_t v;
				asm volatile ("mrc p15, 0, %[v], c1, c1, 0" : [v]"=r"(v) ::);
				return v;
			}
		};

		/**
		 * An execution state
		 */
		struct Context
		{
			enum {
				MAX_GPR = 15,
				MAX_CPU_EXCEPTION = 7,
			};

			/**
			 * Native exception types
			 */
			enum Cpu_exception {
				RESET = 1,
				UNDEFINED_INSTRUCTION = 2,
				SUPERVISOR_CALL = 3,
				PREFETCH_ABORT = 4,
				DATA_ABORT = 5,
				INTERRUPT_REQUEST = 6,
				FAST_INTERRUPT_REQUEST = 7,
			};

			/* general purpose register backups, offsets 0*4 .. 15*4 */
			uint32_t r0,  r1,  r2,  r3,  r4, r5, r6, r7,
			         r8,  r9, r10, r11, r12, r13, r14, r15;

			uint32_t psr; /* program status register backup, offset 16*4 */
			uint32_t contextidr; /* contextidr register backup, offset 17*4 */
			uint32_t cpu_exception; /* native type of last exception,
			                         * offset 18*4 */
			uint32_t section_table; /* base address of applied section table,
			                         * offset 19*4 */

			/**
			 * Read a general purpose register
			 *
			 * \param   id  ID of the targeted register
			 * \param   r   Holds register value if this returns 1
			 */
			bool get_gpr(unsigned id, unsigned & r) const
			{
				if (id > MAX_GPR) return 0;
				r = *(&r0 + id);
				return 1;
			}

			/**
			 * Override a general purpose register
			 *
			 * \param   id  ID of the targeted register
			 * \param   r   Has been written to register if this returns 1
			 */
			bool set_gpr(unsigned id, unsigned const r)
			{
				if (id > MAX_GPR) return 0;
				*(&r0 + id) = r;
				return 1;
			}

			/***************
			 ** Accessors **
			 ***************/

			void software_tlb(Section_table * const st) {
				section_table = (addr_t)st; }

			Section_table * software_tlb() const {
				return (Section_table *)section_table; }

			void instruction_ptr(addr_t const p) { r15 = p; }

			addr_t instruction_ptr() const { return r15; }

			void return_ptr(addr_t const p) { r14 = p; }

			void stack_ptr(addr_t const p) { r13 = p; }

			void protection_domain(unsigned const id) { contextidr = id; }
		};

		/**
		 * An usermode execution state
		 */
		struct User_context : Context
		{
			/**
			 * Constructor
			 */
			User_context()
			{
				/* Execute in usermode with IRQ's enabled and FIQ's and
				 * asynchronous aborts disabled */
				psr = Cpsr::M::bits(Cpsr::M::USER) | Cpsr::F::bits(1) |
				      Cpsr::I::bits(0) | Cpsr::A::bits(1);
			}

			/***************************************************
			 ** Communication between user and context holder **
			 ***************************************************/

			void user_arg_0(unsigned const arg) { r0 = arg; }
			void user_arg_1(unsigned const arg) { r1 = arg; }
			void user_arg_2(unsigned const arg) { r2 = arg; }
			void user_arg_3(unsigned const arg) { r3 = arg; }
			void user_arg_4(unsigned const arg) { r4 = arg; }
			void user_arg_5(unsigned const arg) { r5 = arg; }
			void user_arg_6(unsigned const arg) { r6 = arg; }
			void user_arg_7(unsigned const arg) { r7 = arg; }
			unsigned user_arg_0() const { return r0; }
			unsigned user_arg_1() const { return r1; }
			unsigned user_arg_2() const { return r2; }
			unsigned user_arg_3() const { return r3; }
			unsigned user_arg_4() const { return r4; }
			unsigned user_arg_5() const { return r5; }
			unsigned user_arg_6() const { return r6; }
			unsigned user_arg_7() const { return r7; }

			/**
			 * Determine wich type of exception occured on this context lastly
			 *
			 * \return  0  If the exception is unknown by the kernel
			 *          1  If the exception is an interrupt
			 *          2  If the exception is a pagefault
			 *          3  If the exception is a syscall
			 */
			unsigned exception() const
			{
				/* map all CPU-exception types to kernel-exception types */
				enum { INVALID = 0, INTERRUPT = 1, PAGEFAULT = 2, SYSCALL = 3 };
				static unsigned cpu_excpt_to_excpt[MAX_CPU_EXCEPTION + 1] = {
					INVALID,   /* 0 */
					INVALID,   /* 1 */
					INVALID,   /* 2 */
					SYSCALL,   /* 3 */
					PAGEFAULT, /* 4 */
					PAGEFAULT, /* 5 */
					INTERRUPT, /* 6 */
					INVALID    /* 7 */
				};
				/* determine exception type */
				if (cpu_exception > MAX_CPU_EXCEPTION) return INVALID;
				return cpu_excpt_to_excpt[cpu_exception];
			}

			/**
			 * Does a pagefault exist and originate from a lack of translation?
			 *
			 * \param  va  Holds the virtual fault-address if this
			 *             function returns 1
			 * \param  w   Indicates wether the fault was caused by a write
			 *             access if this function returns 1
			 */
			bool translation_miss(addr_t & va, bool & w) const
			{
				/* determine fault type */
				switch (cpu_exception)
				{
				case PREFETCH_ABORT: {

					/* check if fault was caused by a translation miss */
					Ifsr::Fault_status const fs = Ifsr::fault_status();
					if(fs == Ifsr::SECTION_TRANSLATION_FAULT ||
					   fs == Ifsr::PAGE_TRANSLATION_FAULT)
					{
						/* fetch fault data */
						w = 0;
						va = Ifar::read();
						return 1;
					}
					return 0; }
				case DATA_ABORT: {

					/* check if fault was caused by translation miss */
					Dfsr::Fault_status const fs = Dfsr::fault_status();
					if(fs == Dfsr::SECTION_TRANSLATION_FAULT ||
					   fs == Dfsr::PAGE_TRANSLATION_FAULT)
					{
						/* fetch fault data */
						Dfsr::access_t const dfsr = Dfsr::read();
						w = Dfsr::Wnr::get(dfsr);
						va = Dfar::read();
						return 1;
					}
					return 0; }
				default: return 0;
				}
			}
		};

		/**
		 * Enable interrupt requests
		 */
		static void enable_irqs()
		{
			Cpsr::access_t cpsr = Cpsr::read();
			Cpsr::I::clear(cpsr);
			Cpsr::write(cpsr);
		}

		/**
		 * Set CPU exception entry to a given address
		 *
		 * \return   0  exception entry set to the given address
		 *          <0  otherwise
		 */
		static int exception_entry_at(addr_t a)
		{
			Sctlr::access_t sctlr = Sctlr::read();
			switch (a) {
			case 0x0:
				Sctlr::V::clear(sctlr);
				break;
			case 0xffff0000:
				Sctlr::V::set(sctlr);
				break;
			default:
				return -1;
			}
			Sctlr::write(sctlr);
			return 0;
		}

		/**
		 * Are we in secure mode?
		 */
		static bool secure_mode_active()
		{
			if (!Board::CORTEX_A9_SECURITY_EXTENSION) return 0;
			if (Cpsr::M::get(Cpsr::read()) != Cpsr::M::MONITOR)
			{
				return !Scr::Ns::get(Scr::read());
			}
			return 1;
		}

		/**
		 * Enable the MMU
		 *
		 * \param  section_table  section translation table of the initial
		 *                        address space this function switches to
		 * \param  process_id     process ID of the initial address space
		 */
		static void enable_mmu (Section_table * const section_table,
		                        unsigned long const process_id)
		{
			/* initialize domains */
			Dacr::write (Dacr::D0::bits (Dacr::CLIENT)
			           | Dacr::D1::bits (Dacr::NO_ACCESS)
			           | Dacr::D2::bits (Dacr::NO_ACCESS)
			           | Dacr::D3::bits (Dacr::NO_ACCESS)
			           | Dacr::D4::bits (Dacr::NO_ACCESS)
			           | Dacr::D5::bits (Dacr::NO_ACCESS)
			           | Dacr::D6::bits (Dacr::NO_ACCESS)
			           | Dacr::D7::bits (Dacr::NO_ACCESS)
			           | Dacr::D8::bits (Dacr::NO_ACCESS)
			           | Dacr::D9::bits (Dacr::NO_ACCESS)
			           | Dacr::D10::bits (Dacr::NO_ACCESS)
			           | Dacr::D11::bits (Dacr::NO_ACCESS)
			           | Dacr::D12::bits (Dacr::NO_ACCESS)
			           | Dacr::D13::bits (Dacr::NO_ACCESS)
			           | Dacr::D14::bits (Dacr::NO_ACCESS)
			           | Dacr::D15::bits (Dacr::NO_ACCESS));

			/* switch process ID */
			Contextidr::write(process_id);

			/* install section table */
			Ttbr0::write (Ttbr0::Base_address::masked ((addr_t)section_table));
			Ttbcr::write (Ttbcr::N::bits(0)
			            | Ttbcr::Pd0::bits(0)
			            | Ttbcr::Pd1::bits(0) );

			/* enable MMU without instruction-, data-, or unified caches */
			Sctlr::access_t sctlr = Sctlr::read();
			Sctlr::M::set(sctlr);
			Sctlr::I::clear(sctlr);
			Sctlr::C::clear(sctlr);
			Sctlr::write(sctlr);
			flush_branch_prediction();
		}

		/**
		 * Invalidate all entries of the branch predictor array
		 *
		 * Must be inline to avoid dependence on the branch predictor.
		 */
		__attribute__((always_inline)) inline static void flush_branch_prediction()
		{
			asm volatile ("mcr p15, 0, r0, c7, c5, 6\n"
			              "isb");
		}

		/**
		 * Invalidate at least all TLB entries regarding a specific process
		 *
		 * \param  process_id  ID of the targeted process
		 */
		static void flush_tlb_by_pid (unsigned const process_id)
		{
			asm volatile ("mcr p15, 0, %[asid], c8, c7, 2 \n"
			              :: [asid]"r"(Contextidr::Asid::masked(process_id)) : );
			flush_branch_prediction();
		}
	};
}

#endif /* _INCLUDE__DRIVERS__CPU__CORTEX_A9__CORE_H_ */

