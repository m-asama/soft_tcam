/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#include <bitset>
#include <iostream>
#include <iomanip>

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "soft_tcam.h"

int
main(int argc, char *argv[])
{
	soft_tcam::soft_tcam<std::uint64_t, 64> *tcam;
	const std::uint64_t *result;
	struct rusage ru1, ru2;
	std::uint64_t find_counter = 0;
	double fps;

	const int lim = 256;

	if (argc != 2) {
		std::cout << std::endl
			  << "usage:" << std::endl
			  << "        $ " << argv[0] << " sort" << std::endl
			  << std::endl
			  << "where:" << std::endl
			  << "           sort := [ \"none\" | \"best\" | \"worst\" ]" << std::endl
			  << std::endl;
		exit(1);
	}

	tcam = new soft_tcam::soft_tcam<std::uint64_t, 64>();

	std::cout << "Inserting entries...";

	getrusage(RUSAGE_SELF, &ru1);

	for (std::uint64_t i = 0; i < lim; ++i) {
		for (std::uint64_t j = 0; j < lim; ++j) {
			std::bitset<64> d((i << 52) + (j << 20)), m(0xffff0000ffff0000);
			tcam->insert(d, m, 1, std::uint64_t((i << 52) + (j << 20)));
		}
	}

	getrusage(RUSAGE_SELF, &ru2);

	std::cout << "done." << std::endl;

	/*
	tcam->dump();
	*/

	std::cout << "Allocated soft_tcam_node = "
		  << soft_tcam::soft_tcam_node<std::uint64_t, 64>::get_alloc_counter()
		  << " ( " << (soft_tcam::soft_tcam_node<std::uint64_t, 64>::get_alloc_counter()
					  * sizeof(soft_tcam::soft_tcam_node<std::uint64_t, 64>)) << " bytes)"
		  << std::endl;
	std::cout << "Allocated soft_tcam_entry = "
		  << soft_tcam::soft_tcam_entry<std::uint64_t, 64>::get_alloc_counter()
		  << " ( " << (soft_tcam::soft_tcam_entry<std::uint64_t, 64>::get_alloc_counter()
					  * sizeof(soft_tcam::soft_tcam_entry<std::uint64_t, 64>)) << " bytes)"
		  << std::endl;

	std::bitset<64> k(0x0123456789abcdef);

	soft_tcam::soft_tcam<std::uint64_t, 64>::clear_access_counter();

	for (std::uint64_t j = 0; j < lim; ++j) {
		for (std::uint64_t i = 0; i < lim; ++i) {
			k = (i << 52) + (j << 20);
			result = tcam->find(k);
		}
	}

	if (!strncmp(argv[1], "best", 5)) {
		soft_tcam::soft_tcam<std::uint64_t, 64>::sort_best();
	} else if (!strncmp(argv[1], "worst", 6)) {
		soft_tcam::soft_tcam<std::uint64_t, 64>::sort_worst();
	} else if (!strncmp(argv[1], "none", 5)) {
	} else {
		std::cout << "sort arg error" << std::endl;
		exit(1);
	}

	for (std::uint64_t i = 0; i < lim; ++i) {
		for (std::uint64_t j = 0; j < lim; ++j) {
			k = (i << 52) + (j << 20);
			result = tcam->find(k);
		}
	}

	std::cout << "Finding entries...";

	getrusage(RUSAGE_SELF, &ru1);

	for (std::uint64_t c = 0; c < 1000; ++c) {
		for (std::uint64_t i = 0; i < lim; ++i) {
			for (std::uint64_t j = 0; j < lim; ++j) {
				k = (i << 52) + (j << 20);
				result = tcam->find(k);
				/*
				std::cout << std::setw(16)
					  << std::setfill('0')
					  << std::hex
					  << ((i << 52) + (j << 20))
					  << " : ";
				if (result != nullptr) {
					std::cout << std::setw(16)
						  << std::setfill('0')
						  << std::hex
						  << *result
						  << std::endl;
				} else {
					std::cout << "nullptr" << std::endl;
				}
				*/
				++find_counter;
			}
		}
	}

	getrusage(RUSAGE_SELF, &ru2);

	std::cout << "done." << std::endl;

	timersub(&ru2.ru_utime, &ru1.ru_utime, &ru2.ru_utime);
	fps = ru2.ru_utime.tv_usec;
	fps /= 1000000;
	fps += ru2.ru_utime.tv_sec;
	fps = find_counter / fps;

	std::cout << "Find counter = "
		  << find_counter
		  << std::endl;
	std::cout << "Find per second = "
		  << std::fixed << fps
		  << std::endl;

	return 0;
}

