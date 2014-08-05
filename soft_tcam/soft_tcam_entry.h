/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#ifndef SOFT_TCAM_ENTRY_H
#define SOFT_TCAM_ENTRY_H

#include <cstdint>
#include <bitset>

namespace soft_tcam {

	template <class T, size_t size> class soft_tcam_node;

	template <class T, size_t size>
	class soft_tcam_entry {

	public:

		/*
		 * ctor
		 */	
		soft_tcam_entry();

		/*
		 * dtor
		 */
		virtual ~soft_tcam_entry();

		/*
		 * new operator overload
		 */
		void *operator new(size_t s);

		/*
		 * delete operator overload
		 */
		void operator delete(void *p);

		/*
		 * priority setter
		 */
		void set_priority(std::uint32_t priority);

		/*
		 * priority getter
		 */
		std::uint32_t get_priority();

		/*
		 * object setter
		 */
		void set_object(const T object);

		/*
		 * object getter
		 */
		const T &get_object();

		/*
		 * next setter
		 */
		void set_next(soft_tcam_entry<T, size> *next);

		/*
		 * next getter
		 */
		soft_tcam_entry<T, size> *get_next();

		/*
		 * node setter
		 */
		void set_node(soft_tcam_node<T, size> *node);

		/*
		 * node getter
		 */
		soft_tcam_node<T, size> *get_node();

		/*
		 * get_alloc_counter
		 */
		static std::uint64_t get_alloc_counter();

	private:

		std::uint32_t m_priority;
		T m_object;
		soft_tcam_entry<T, size> *m_next;
		soft_tcam_node<T, size> *m_node;
		std::uint64_t m_access_counter;

		static std::uint64_t s_alloc_counter;

	};

}

#include "soft_tcam_entry.cc"

#endif // SOFT_TCAM_ENTRY_H

