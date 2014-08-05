Soft TCAM
=========

TCAM Emulation Library

## これはなに？

TCAM っぽい機能を実現するライブラリです。

## どうやって使うの？

以下のように使います。

`
#include <cstdint>
#include <bitset>
#include <iostream>

#include "soft_tcam.h"

int
main(int argc, char *argv[])
{
	soft_tcam::soft_tcam<std::uint32_t, 32> tcam;
	std::bitset<32> data, mask, key;
	std::uint32_t priority;
	const std::uint32_t *result;

	data		= 0x12340000;
	mask		= 0x0000ffff;
	priority	= 0x00000010;
	tcam.insert(data, mask, priority, 1);

	data		= 0x12000000;
	mask		= 0x00ffffff;
	priority	= 0x00000008;
	tcam.insert(data, mask, priority, 2);

	key		= 0x12345678;
	result = tcam.find(key);
	std::cout << "Find result: ";
	if (result != nullptr) {
		std::cout << *result;
	} else {
		std::cout << "nullptr";
	}
	std::cout << std::endl;

	data		= 0x12340000;
	mask		= 0x0000ffff;
	priority	= 0x00000010;
	tcam.erase(data, mask, priority, 1);

	data		= 0x12000000;
	mask		= 0x00ffffff;
	priority	= 0x00000008;
	tcam.erase(data, mask, priority, 2);

	return 0;
}
`

