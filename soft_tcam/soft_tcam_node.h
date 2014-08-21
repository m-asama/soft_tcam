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
		 * data setter
		 */
		void set_data(const std::bitset<size> &data);

		/*
		 * data getter
		 */
		const std::bitset<size> &get_data();

		/*
		 * mask setter
		 */
		void set_mask(const std::bitset<size> &mask);

		/*
		 * mask getter
		 */
		const std::bitset<size> &get_mask();

		/*
		 * position setter
		 */
		void set_position(std::uint32_t position);

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
		 * set_entry_head
		 */
		void set_entry_head(soft_tcam_entry<T, size> *entry_head);

		/*
		 * get_entry_head
		 */
		soft_tcam_entry<T, size> *get_entry_head();

		/*
		 * get_list_next
		 */
		soft_tcam_node<T, size> *get_list_next();

		/*
		 * set_access_counter
		 */
		void set_access_counter(std::uint64_t access_counter);

		/*
		 * get_access_counter
		 */
		std::uint64_t get_access_counter();

		/*
		 * insert_entry
		 */
		int insert_entry(soft_tcam_entry<T, size> *entry);

		/*
		 * erase_entry
		 */
		int erase_entry(soft_tcam_entry<T, size> *entry);

		/*
		 * get_list_head
		 */
		static soft_tcam_node<T, size> *get_list_head();

		/*
		 * get_alloc_counter
		 */
		static std::uint64_t get_alloc_counter();

	private:

		std::bitset<size> m_data;
		std::bitset<size> m_mask;
		std::uint32_t m_position;
		soft_tcam_node<T, size> *m_n0;
		soft_tcam_node<T, size> *m_n1;
		soft_tcam_node<T, size> *m_ndc;
		soft_tcam_node<T, size> *m_parent;
		soft_tcam_entry<T, size> *m_entries;
		soft_tcam_node<T, size> *m_list_next;
		std::uint64_t m_access_counter;

		static soft_tcam_node<T, size> *s_list_head;
		static std::uint64_t s_alloc_counter;

	};

}

#include "soft_tcam_node.cc"

#endif // SOFT_TCAM_NODE_H

