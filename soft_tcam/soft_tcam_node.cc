/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#include "soft_tcam_node.h"

namespace soft_tcam {

	template<class T, size_t size>
	soft_tcam_node<T, size>::soft_tcam_node(const std::bitset<size> &data, const std::bitset<size> &mask,
			const std::uint32_t position) :
		m_data(data), m_mask(mask), m_position(position)

	{
		m_n0 = nullptr;
		m_n1 = nullptr;
		m_ndc = nullptr;
		m_parent = nullptr;
		m_entries = nullptr;
		m_access_counter = 0;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size>::~soft_tcam_node()
	{
	}

	template<class T, size_t size>
	void *
	soft_tcam_node<T, size>::operator new(size_t s)
	{
		++s_alloc_counter;
		return malloc(s);
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::operator delete(void *p)
	{
		--s_alloc_counter;
		free(p);
	}

	template<class T, size_t size>
	const std::bitset<size> &
	soft_tcam_node<T, size>::get_data()
	{
		++m_access_counter;
		return m_data;
	}

	template<class T, size_t size>
	const std::bitset<size> &
	soft_tcam_node<T, size>::get_mask()
	{
		++m_access_counter;
		return m_mask;
	}

	template<class T, size_t size>
	std::uint32_t
	soft_tcam_node<T, size>::get_position()
	{
		++m_access_counter;
		return m_position;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_n0(soft_tcam_node<T, size> *n0)
	{
		m_n0 = n0;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_node<T, size>::get_n0()
	{
		++m_access_counter;
		return m_n0;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_n1(soft_tcam_node<T, size> *n1)
	{
		m_n1 = n1;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_node<T, size>::get_n1()
	{
		++m_access_counter;
		return m_n1;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_ndc(soft_tcam_node<T, size> *ndc)
	{
		m_ndc = ndc;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_node<T, size>::get_ndc()
	{
		++m_access_counter;
		return m_ndc;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_parent(soft_tcam_node<T, size> *parent)
	{
		m_parent = parent;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_node<T, size>::get_parent()
	{
		++m_access_counter;
		return m_parent;
	}

	template<class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam_node<T, size>::get_entry_head()
	{
		++m_access_counter;
		return m_entries;
	}

	template<class T, size_t size>
	int
	soft_tcam_node<T, size>::insert_entry(soft_tcam_entry<T, size> *entry)
	{
		soft_tcam_entry<T, size> *curr, *prev;

		if (entry == nullptr) {
			return -1;
		}

		entry->set_node(this);

		if ((m_entries == nullptr)
		 || (entry->get_priority() > m_entries->get_priority())) {
			entry->set_next(m_entries);
			m_entries = entry;
			return 0;
		}

		prev = m_entries;
		curr = m_entries->get_next();
		while (curr != nullptr) {
			if (curr->get_priority() < entry->get_priority()) {
				break;
			}
			prev = curr;
			curr = curr->get_next();
		}
		entry->set_next(prev->get_next());
		prev->set_next(entry);

		return 0;
	}

	template<class T, size_t size>
	int
	soft_tcam_node<T, size>::erase_entry(soft_tcam_entry<T, size> *entry)
	{
		soft_tcam_entry<T, size> *curr, *prev;

		if ((m_entries == nullptr) || (entry == nullptr)) {
			return -1;
		}

		if (m_entries == entry) {
			m_entries = entry->get_next();
			entry->set_next(nullptr);
			entry->set_node(nullptr);
			delete entry;
			return 0;
		}

		prev = m_entries;
		curr = m_entries->get_next();
		while (curr != nullptr) {
			if (curr == entry) {
				prev->set_next(entry->get_next());
				entry->set_next(nullptr);
				entry->set_node(nullptr);
				delete entry;
				return 0;
			}
			prev = curr;
			curr = curr->get_next();
		}

		return -1;
	}

	template<class T, size_t size>
	std::uint64_t
	soft_tcam_node<T, size>::get_alloc_counter()
	{
		return s_alloc_counter;
	}

	template<class T, size_t size>
		std::uint64_t soft_tcam_node<T, size>::s_alloc_counter = 0;

}

