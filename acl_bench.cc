/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#include <bitset>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <utility>

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "soft_tcam.h"

static const std::uint64_t bench_count = 100000000;
static const std::uint64_t warmup_count = 1000;

class sequential_acl {
public:
	int insert(const std::bitset<32> &data, const std::bitset<32> &mask,
			std::uint32_t priority, const std::uint32_t &object) {
		acl.push_back(std::pair<std::bitset<32>, std::uint32_t>(data, object));
		return 0;
	}
	const std::uint32_t *find(const std::bitset<32> &key) {
		for (auto it = acl.begin(); it != acl.end(); ++it) {
			if (it->first == key) {
				return &it->second;
			}
		}
		return nullptr;
	}
private:
	std::vector<std::pair<std::bitset<32>, std::uint32_t>> acl;
};

static int
load_acl(std::vector<std::bitset<32>> &acls, const char *acl_path, std::uint64_t count)
{
	struct in_addr ina;
	// struct in6_addr in6a;
	std::ifstream acl_file;
	std::string line;
	std::bitset<32> k;

	acl_file.open(acl_path);
	if (acl_file.fail()) {
		std::cout << acl_path << " open failed." << std::endl;
		exit(1);
	}

	std::cout << "Loading acl...";
	std::cout.flush();

	while (getline(acl_file, line) && (count > 0)) {
		if (inet_pton(AF_INET, line.c_str(), &ina) <= 0) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
		k = ntohl(ina.s_addr);
		acls.push_back(k);
		--count;
	}

	std::cout << "done." << std::endl;

	return 0;
}

int
main(int argc, char *argv[])
{
	soft_tcam::soft_tcam<std::uint32_t, 32> *tcam;
	sequential_acl *sacl;
	std::uint64_t priority;
	std::vector<std::bitset<32>> acls;
	const std::uint32_t *result;
	struct rusage ru1, ru2;
	std::uint64_t find_counter = 0;
	std::uint64_t load_num;
	double fps;
	std::bitset<32> k;

	if (argc != 3) {
		std::cout << std::endl
			  << "usage:" << std::endl
			  << "        $ " << argv[0] << " acl load_num" << std::endl
			  << std::endl
			  << "where:" << std::endl
			  << "            acl := Containing ACL file (Ex. acl.sample)" << std::endl
			  << "       load_num := Load count of ACL (Ex. 1000)" << std::endl
			  << std::endl;
		exit(1);
	}

	load_num = atoi(argv[2]);
	load_acl(acls, argv[1], load_num);

	tcam = new soft_tcam::soft_tcam<std::uint32_t, 32>();
	priority = std::numeric_limits<std::uint64_t>::max();
	for (auto it = acls.begin(); it != acls.end(); ++it) {
		if (tcam->insert(*it, 0xffffffff, priority, it->to_ulong()) != 0) {
			std::cout << "tcam load skip: " << *it << std::endl;
			continue;
		}
		--priority;
	}
	std::cout << "Allocated soft_tcam_node = "
		  << soft_tcam::soft_tcam_node<std::uint32_t, 32>::get_alloc_counter()
		  << " ( " << (soft_tcam::soft_tcam_node<std::uint32_t, 32>::get_alloc_counter()
					  * sizeof(soft_tcam::soft_tcam_node<std::uint32_t, 32>)) << " bytes)"
		  << std::endl;
	std::cout << "Allocated soft_tcam_entry = "
		  << soft_tcam::soft_tcam_entry<std::uint32_t, 32>::get_alloc_counter()
		  << " ( " << (soft_tcam::soft_tcam_entry<std::uint32_t, 32>::get_alloc_counter()
					  * sizeof(soft_tcam::soft_tcam_entry<std::uint32_t, 32>)) << " bytes)"
		  << std::endl;

	sacl = new sequential_acl;
	priority = std::numeric_limits<std::uint64_t>::max();
	for (auto it = acls.begin(); it != acls.end(); ++it) {
		if (sacl->insert(*it, 0xffffffff, priority, it->to_ulong()) != 0) {
			std::cout << "sacl load skip: " << *it << std::endl;
			continue;
		}
		--priority;
	}

	{
		std::cout << "### tcam && acl first entry" << std::endl;

		k = *acls.begin();

		soft_tcam::soft_tcam<std::uint32_t, 32>::clear_access_counter();
		for (std::uint64_t i = 0; i < warmup_count; ++i) {
			result = tcam->find(k);
			++find_counter;
		}
		soft_tcam::soft_tcam<std::uint32_t, 32>::sort_best();
		for (std::uint64_t i = 0; i < warmup_count; ++i) {
			result = tcam->find(k);
			++find_counter;
		}

		find_counter = 0;
		getrusage(RUSAGE_SELF, &ru1);
		for (std::uint64_t i = 0; i < bench_count; ++i) {
			result = tcam->find(k);
			if (result == nullptr) {
				exit(1);
			}
			if (*result != k.to_ulong()) {
				std::cout << "miss-match " << *result << ":" << k.to_ulong() << std::endl;
				exit(1);
			}
			++find_counter;
		}
		getrusage(RUSAGE_SELF, &ru2);
		timersub(&ru2.ru_utime, &ru1.ru_utime, &ru2.ru_utime);
		fps = ru2.ru_utime.tv_usec;
		fps /= 1000000;
		fps += ru2.ru_utime.tv_sec;
		fps = find_counter / fps;
		std::cout << "Find counter = " << find_counter << std::endl;
		std::cout << "Find per second = " << std::fixed << fps << std::endl;
	}

	{
		std::cout << "### tcam && acl last entry" << std::endl;

		k = *(acls.end() - 1);

		soft_tcam::soft_tcam<std::uint32_t, 32>::clear_access_counter();
		for (std::uint64_t i = 0; i < warmup_count; ++i) {
			result = tcam->find(k);
			++find_counter;
		}
		soft_tcam::soft_tcam<std::uint32_t, 32>::sort_best();
		for (std::uint64_t i = 0; i < warmup_count; ++i) {
			result = tcam->find(k);
			++find_counter;
		}

		find_counter = 0;
		getrusage(RUSAGE_SELF, &ru1);
		for (std::uint64_t i = 0; i < bench_count; ++i) {
			result = tcam->find(k);
			if (result == nullptr) {
				exit(1);
			}
			if (*result != k.to_ulong()) {
				std::cout << "miss-match " << *result << ":" << k.to_ulong() << std::endl;
				exit(1);
			}
			++find_counter;
		}
		getrusage(RUSAGE_SELF, &ru2);
		timersub(&ru2.ru_utime, &ru1.ru_utime, &ru2.ru_utime);
		fps = ru2.ru_utime.tv_usec;
		fps /= 1000000;
		fps += ru2.ru_utime.tv_sec;
		fps = find_counter / fps;
		std::cout << "Find counter = " << find_counter << std::endl;
		std::cout << "Find per second = " << std::fixed << fps << std::endl;
	}

	{
		std::cout << "### sacl && acl first entry" << std::endl;

		k = *acls.begin();

		for (std::uint64_t i = 0; i < warmup_count; ++i) {
			result = sacl->find(k);
			++find_counter;
		}

		find_counter = 0;
		getrusage(RUSAGE_SELF, &ru1);
		for (std::uint64_t i = 0; i < bench_count; ++i) {
			result = sacl->find(k);
			if (result == nullptr) {
				exit(1);
			}
			if (*result != k.to_ulong()) {
				std::cout << "miss-match " << *result << ":" << k.to_ulong() << std::endl;
				exit(1);
			}
			++find_counter;
		}
		getrusage(RUSAGE_SELF, &ru2);
		timersub(&ru2.ru_utime, &ru1.ru_utime, &ru2.ru_utime);
		fps = ru2.ru_utime.tv_usec;
		fps /= 1000000;
		fps += ru2.ru_utime.tv_sec;
		fps = find_counter / fps;
		std::cout << "Find counter = " << find_counter << std::endl;
		std::cout << "Find per second = " << std::fixed << fps << std::endl;
	}

	{
		std::cout << "### sacl && acl last entry" << std::endl;

		k = *(acls.end() - 1);

		for (std::uint64_t i = 0; i < warmup_count; ++i) {
			result = sacl->find(k);
			++find_counter;
		}

		find_counter = 0;
		getrusage(RUSAGE_SELF, &ru1);
		for (std::uint64_t i = 0; i < bench_count; ++i) {
			result = sacl->find(k);
			if (result == nullptr) {
				exit(1);
			}
			if (*result != k.to_ulong()) {
				std::cout << "miss-match " << *result << ":" << k.to_ulong() << std::endl;
				exit(1);
			}
			++find_counter;
		}
		getrusage(RUSAGE_SELF, &ru2);
		timersub(&ru2.ru_utime, &ru1.ru_utime, &ru2.ru_utime);
		fps = ru2.ru_utime.tv_usec;
		fps /= 1000000;
		fps += ru2.ru_utime.tv_sec;
		fps = find_counter / fps;
		std::cout << "Find counter = " << find_counter << std::endl;
		std::cout << "Find per second = " << std::fixed << fps << std::endl;
	}

}
