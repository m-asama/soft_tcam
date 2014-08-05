/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#ifndef SOFT_TCAM_NODE_H
#define SOFT_TCAM_NODE_H

#include <cstdint>
#include <bitset>

namespace soft_tcam {

	template<class T, size_t size> class soft_tcam_entry;

	template<class T, size_t size>
	class soft_tcam_node {

	public:

		/*
		 * ctor
		 */
		soft_tcam_node(const std::bitset<size> &data, const std::bitset<size> &mask,
				const std::uint32_t position);

		/*
		 * dtor
		 */
		virtual ~soft_tcam_node();

		/*
		 * new operator overload
		 */
		void *operator new(size_t s);

		/*
		 * delete operator overload
		 */
		void operator delete(void *p);

		/*
		 * data getter
		 */
		const std::bitset<size> &get_data();

		/*
		 * mask getter
		 */
		const std::bitset<size> &get_mask();

		/*
		 * position getter
		 */
		std::uint32_t get_position();

		/*
		 * n0 setter
		 */
		void set_n0(soft_tcam_node<T, size> *n0);

		/*
		 * n0 getter
		 */
		soft_tcam_node<T, size> *get_n0();

		/*
		 * n1 setter
		 */
		void set_n1(soft_tcam_node<T, size> *n1);

		/*
		 * n1 getter
		 */
		soft_tcam_node<T, size> *get_n1();

		/*
		 * ndc setter
		 */
		void set_ndc(soft_tcam_node<T, size> *ndc);

		/*
		 * ndc getter
		 */
		soft_tcam_node<T, size> *get_ndc();

		/*
		 * parent setter
		 */
		void set_parent(soft_tcam_node<T, size> *parent);

		/*
		 * parent getter
		 */
		soft_tcam_node<T, size> *get_parent();

		/*
		 * get_entry_head
		 */
		soft_tcam_entry<T, size> *get_entry_head();

		/*
		 * insert_entry
		 */
		int insert_entry(soft_tcam_entry<T, size> *entry);

		/*
		 * erase_entry
		 */
		int erase_entry(soft_tcam_entry<T, size> *entry);

		/*
		 * get_alloc_counter
		 */
		static std::uint64_t get_alloc_counter();

	private:

		const std::bitset<size> m_data;
		const std::bitset<size> m_mask;
		const std::uint32_t m_position;
		soft_tcam_node<T, size> *m_n0;
		soft_tcam_node<T, size> *m_n1;
		soft_tcam_node<T, size> *m_ndc;
		soft_tcam_node<T, size> *m_parent;
		soft_tcam_entry<T, size> *m_entries;
		std::uint64_t m_access_counter;

		static std::uint64_t s_alloc_counter;

	};

}

#include "soft_tcam_node.cc"

#endif // SOFT_TCAM_NODE_H

