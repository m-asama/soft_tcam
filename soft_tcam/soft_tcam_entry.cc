/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#include "soft_tcam_entry.h"

namespace soft_tcam {

	template <class T, size_t size>
	soft_tcam_entry<T, size>::soft_tcam_entry()
	{
		m_priority = 0;
		m_next = nullptr;
		m_prev = nullptr;
		m_node = nullptr;
		m_access_counter = 0;

		m_list_next = s_list_head;
		s_list_head = this;
	}

	template <class T, size_t size>
	soft_tcam_entry<T, size>::~soft_tcam_entry()
	{
		soft_tcam_entry<T, size> *curr, *prev;

		if (s_list_head == this) {
			s_list_head = m_list_next;
			return;
		}

		prev = s_list_head;
		curr = s_list_head->m_list_next;
		while (curr != nullptr) {
			if (curr == this) {
				prev->m_list_next = m_list_next;
				return;
			}
			prev = curr;
			curr = curr->m_list_next;
		}
	}

	template<class T, size_t size>
	void *
	soft_tcam_entry<T, size>::operator new(size_t s)
	{
		++s_alloc_counter;
		return malloc(s);
	}

	template<class T, size_t size>
	void
	soft_tcam_entry<T, size>::operator delete(void *p)
	{
		--s_alloc_counter;
		free(p);
	}

	template <class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_priority(std::uint32_t priority)
	{
		m_priority = priority;
	}

	template <class T, size_t size>
	std::uint32_t
	soft_tcam_entry<T, size>::get_priority()
	{
		++m_access_counter;
		return m_priority;
	}

	template <class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_object(const T object)
	{
		m_object = object;
	}

	template <class T, size_t size>
	const T &
	soft_tcam_entry<T, size>::get_object()
	{
		++m_access_counter;
		return m_object;
	}

	template <class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_next(soft_tcam_entry<T, size> *next)
	{
		m_next = next;
	}

	template <class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam_entry<T, size>::get_next()
	{
		++m_access_counter;
		return m_next;
	}

	template <class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_prev(soft_tcam_entry<T, size> *prev)
	{
		m_prev = prev;
	}

	template <class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam_entry<T, size>::get_prev()
	{
		++m_access_counter;
		return m_prev;
	}

	template <class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_node(soft_tcam_node<T, size> *node)
	{
		m_node = node;
	}

	template <class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_entry<T, size>::get_node()
	{
		++m_access_counter;
		return m_node;
	}

	template<class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_list_next(soft_tcam_entry<T, size> *list_next)
	{
		m_list_next = list_next;
	}

	template<class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam_entry<T, size>::get_list_next()
	{
		return m_list_next;
	}

	template<class T, size_t size>
	void
	soft_tcam_entry<T, size>::set_access_counter(std::uint64_t access_counter)
	{
		m_access_counter = access_counter;
	}

	template<class T, size_t size>
	std::uint64_t
	soft_tcam_entry<T, size>::get_access_counter()
	{
		return m_access_counter;
	}

	template<class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam_entry<T, size>::get_list_head()
	{
		return s_list_head;
	}

	template<class T, size_t size>
	std::uint64_t
	soft_tcam_entry<T, size>::get_alloc_counter()
	{
		return s_alloc_counter;
	}

	template<class T, size_t size>
		soft_tcam_entry<T, size> *soft_tcam_entry<T, size>::s_list_head = nullptr;
	template<class T, size_t size>
		std::uint64_t soft_tcam_entry<T, size>::s_alloc_counter = 0;

}

