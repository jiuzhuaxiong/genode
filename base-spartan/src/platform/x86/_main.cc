/*
 * \brief   Startup code
 * \author  Christian Helmuth
 * \author  Christian Prochaska
 * \date    2006-04-12
 *
 * This is a stripped down version of $(GENODE_DIR)/base/src/platform/_main.cc
 * If you want to use this code and do not find it at 
 * $(GENODE_DIR)/base-spartan/src/platform/x86/_main.cc you should place it there :-)
 * The startup code simply calls the main() function.
 */

/*
 * Copyright (C) 2006-2012 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

extern int main(int argc, char **argv);

/**
 * Dummy default arguments for main function
 */
//static char  argv0[] = { '_', 'm', 'a', 'i', 'n', 0};
//static char *argv[1] = { argv0 };

/**
 *  * C entry function called by the crt0 startup code
 *   */
extern "C" int _main()
{
	/* call real main function */
//	int ret = main(genode_argc, genode_argv);
	int ret = main();

	return ret;
}
