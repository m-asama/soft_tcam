/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#ifndef SOFT_TCAM_H
#define SOFT_TCAM_H

#include <cstdint>
#include <bitset>
#include <stack>

#include "soft_tcam_node.h"
#include "soft_tcam_entry.h"

namespace soft_tcam {

	template<class T, size_t size>
	class soft_tcam {

	public:

		/*
		 * ctor
		 */
		soft_tcam();

		/*
		 * dtor
		 */
		virtual ~soft_tcam();

		/*
		 * new operator overload
		 */
		void *operator new(size_t s);

		/*
		 * delete operator overload
		 */
		void operator delete(void *p);

		/*
		 * insert
		 */
		int insert(const std::bitset<size> &data, const std::bitset<size> &mask, std::uint32_t priority,
				const T &object);

		/*
		 * erase
		 */
		int erase(const std::bitset<size> &data, const std::bitset<size> &mask, std::uint32_t priority,
				const T &object);

		/*
		 * find
		 */
		const T *find(const std::bitset<size> &key);

		/*
		 * dump
		 */
		void dump();

		/*
		 * sort best
		 */
		static void sort_best();

		/*
		 * soft worst
		 */
		static void sort_worst();

		/*
		 * clear access counter
		 */
		static void clear_access_counter();

		/*
		 * dump_access_counter
		 */
		static void dump_access_counter();

	private:

		soft_tcam_node<T, size> *m_root;
		soft_tcam<T, size> *m_list_next;

		void destroy_node(soft_tcam_node<T, size> *node);
		int insert_between(soft_tcam_node<T, size> *less, soft_tcam_node<T, size> *more,
				soft_tcam_node<T, size> *node);
		int erase_node(soft_tcam_node<T, size> *node);
		soft_tcam_node<T, size> *find_nearest_node(const std::bitset<size> &data,
				const std::bitset<size> &mask);
		soft_tcam_entry<T, size> *find_entry(const std::bitset<size> &key);
		void dump_node(soft_tcam_node<T, size> *node, int depth);

		static soft_tcam<T, size> *s_list_head;
		static std::uint64_t s_alloc_counter;

	};

}

#include "soft_tcam.cc"

#endif // SOFT_TCAM_H

