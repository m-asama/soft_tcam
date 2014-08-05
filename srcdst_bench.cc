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

void
print_time(const struct timeval &tv1, const struct timeval &tv2, const struct rusage &ru1, const struct rusage &ru2)
{
	struct timeval tv;
	struct rusage ru;
	double r, u, s;

	timersub(&tv2, &tv1, &tv);
	timersub(&ru2.ru_utime, &ru1.ru_utime, &ru.ru_utime);
	timersub(&ru2.ru_stime, &ru1.ru_stime, &ru.ru_stime);

	r = tv.tv_usec;
	r /= 1000000;
	r += tv.tv_sec;

	u = ru.ru_utime.tv_usec;
	u /= 1000000;
	u += ru.ru_utime.tv_sec;

	s = ru.ru_stime.tv_usec;
	s /= 1000000;
	s += ru.ru_stime.tv_sec;

	std::cout << "   real " << std::fixed << r << std::endl
		  << "   user " << std::fixed << u << std::endl
		  << "    sys " << std::fixed << s << std::endl;
}

int
main(int argc, char *argv[])
{
	soft_tcam::soft_tcam<std::uint64_t, 64> test;
	const std::uint64_t *result;
	struct timeval tv1, tv2;
	struct rusage ru1, ru2;
	std::uint64_t find_counter = 0;
	double fps;

	std::cout << "Inserting entries...";

	gettimeofday(&tv1, (struct timezone *)NULL);
	getrusage(RUSAGE_SELF, &ru1);

	for (std::uint64_t i = 0; i < 4096; ++i) {
		for (std::uint64_t j = 0; j < 4096; ++j) {
			std::bitset<64> d((i << 52) + (j << 20)), m(0xffff0000ffff0000);
			test.insert(d, m, 1, std::uint64_t((i << 52) + (j << 20)));
		}
	}

	gettimeofday(&tv2, (struct timezone *)NULL);
	getrusage(RUSAGE_SELF, &ru2);

	std::cout << "done." << std::endl;

	print_time(tv1, tv2, ru1, ru2);

	/*
	test.dump();
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

	std::cout << "Finding entries...";

	gettimeofday(&tv1, (struct timezone *)NULL);
	getrusage(RUSAGE_SELF, &ru1);

	std::bitset<64> k(0x0123456789abcdef);
	for (std::uint64_t j = 0; j < 4096; ++j) {
		for (std::uint64_t i = 0; i < 4096; ++i) {
			k = (i << 52) + (j << 20);
			result = test.find(k);
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

	gettimeofday(&tv2, (struct timezone *)NULL);
	getrusage(RUSAGE_SELF, &ru2);

	std::cout << "done." << std::endl;

	print_time(tv1, tv2, ru1, ru2);

	timersub(&tv2, &tv1, &tv2);
	fps = tv2.tv_usec;
	fps /= 1000000;
	fps += tv2.tv_sec;
	fps = find_counter / fps;

	std::cout << "Find counter = "
		  << find_counter
		  << std::endl;
	std::cout << "Find per second = "
		  << std::fixed << fps
		  << std::endl;

	return 0;
}

