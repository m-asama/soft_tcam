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

#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "soft_tcam.h"

static const std::uint64_t bench_count = 10000000;
static const std::uint64_t warmup_count = 1000;

static int
load_fullroute(soft_tcam::soft_tcam<std::uint32_t, 32> &tcam, const char *fullroute_path)
{
	struct in_addr ina;
	// struct in6_addr in6a;
	std::ifstream fullroute_file;
	std::string line;
	char buf[1024 + 1];
	char *plens;
	int plen;
	std::bitset<32> d, m;

	fullroute_file.open(fullroute_path);
	if (fullroute_file.fail()) {
		std::cout << fullroute_path <<  " open failed." << std::endl;
		exit(1);
	}

	std::cout << "Loading fullroute...";
	std::cout.flush();

	while (getline(fullroute_file, line)) {
		if (line.length() >= 1024) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
		std::strcpy(buf, line.c_str());
		std::strtok(buf, "/");
		plens = std::strtok(nullptr, "/");
		if (plens == nullptr) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
		plen = atoi(plens);
		if (inet_pton(AF_INET, buf, &ina) <= 0) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
		if ((plen == 0) && (ina.s_addr != 0)) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
		d = ntohl(ina.s_addr);
		m = (0xffffffff << (32 - plen));
		if (tcam.insert(d, m, plen, ntohl(ina.s_addr)) != 0) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
	}

	std::cout << "done." << std::endl;

	return 0;
}

static int
load_flow(std::vector<std::bitset<32>> &flows, const char *flow_path)
{
	struct in_addr ina;
	// struct in6_addr in6a;
	std::ifstream flow_file;
	std::string line;
	std::bitset<32> k;

	flow_file.open(flow_path);
	if (flow_file.fail()) {
		std::cout << flow_path << " open failed." << std::endl;
		exit(1);
	}

	std::cout << "Loading flow...";
	std::cout.flush();

	while (getline(flow_file, line)) {
		if (inet_pton(AF_INET, line.c_str(), &ina) <= 0) {
			std::cout << "skip: " << line << std::endl;
			continue;
		}
		k = ntohl(ina.s_addr);
		flows.push_back(k);
	}

	std::cout << "done." << std::endl;

	return 0;
}

int
main(int argc, char *argv[])
{
	soft_tcam::soft_tcam<std::uint32_t, 32> *tcam;
	std::vector<std::bitset<32>> flows;
	const std::uint32_t *result;
	struct rusage ru1, ru2;
	std::uint64_t find_counter = 0;
	double fps;

	if (argc != 5) {
		std::cout << std::endl
			  << "usage:" << std::endl
			  << "        $ " << argv[0] << " fullroute learningflow targetaddr sort" << std::endl
			  << std::endl
			  << "where:" << std::endl
			  << "      fullroute := Containing full route file (Ex. fullroute.sample)" << std::endl
			  << "   learningflow := Containing learing flow file (Ex. learningflow.sample)" << std::endl
			  << "     targetaddr := Target IPv4 address (Ex. 192.168.1.1)" << std::endl
			  << "           sort := [ \"none\" | \"best\" | \"worst\" ]" << std::endl
			  << std::endl;
		exit(1);
	}

	tcam = new soft_tcam::soft_tcam<std::uint32_t, 32>();

	load_fullroute(*tcam, argv[1]);
	load_flow(flows, argv[2]);

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

	soft_tcam::soft_tcam<std::uint32_t, 32>::clear_access_counter();

	for (auto it = flows.begin(); it != flows.end(); ++it) {
		tcam->find(*it);
	}

	if (!strncmp(argv[4], "best", 5)) {
		soft_tcam::soft_tcam<std::uint32_t, 32>::sort_best();
	} else if (!strncmp(argv[4], "worst", 6)) {
		soft_tcam::soft_tcam<std::uint32_t, 32>::sort_worst();
	} else if (!strncmp(argv[4], "none", 5)) {
	} else {
		std::cout << "sort arg error" << std::endl;
		exit(1);
	}

	struct in_addr ina;
	std::bitset<32> k;
	if (inet_pton(AF_INET, argv[3], &ina) <= 0) {
		std::cout << "inet_pton error" << std::endl;
		exit(1);
	}
	k = ntohl(ina.s_addr);

	for (std::uint64_t i = 0; i < warmup_count; ++i) {
		tcam->find(k);
	}

	getrusage(RUSAGE_SELF, &ru1);

	for (std::uint64_t i = 0; i < bench_count; ++i) {
		result = tcam->find(k);
		++find_counter;
	}

	getrusage(RUSAGE_SELF, &ru2);

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

	// tcam->dump();

	return 0;
}

