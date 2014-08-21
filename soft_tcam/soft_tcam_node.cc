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

		m_list_next = s_list_head;
		s_list_head = this;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size>::~soft_tcam_node()
	{
		soft_tcam_node<T, size> *curr, *prev;

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
	void
	soft_tcam_node<T, size>::set_data(const std::bitset<size> &data)
	{
		m_data = data;
	}

	template<class T, size_t size>
	const std::bitset<size> &
	soft_tcam_node<T, size>::get_data()
	{
		++m_access_counter;
		return m_data;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_mask(const std::bitset<size> &mask)
	{
		m_mask = mask;
	}

	template<class T, size_t size>
	const std::bitset<size> &
	soft_tcam_node<T, size>::get_mask()
	{
		++m_access_counter;
		return m_mask;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_position(std::uint32_t position)
	{
		m_position = position;
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
	void
	soft_tcam_node<T, size>::set_entry_head(soft_tcam_entry<T, size> *entry_head)
	{
		m_entries = entry_head;
	}

	template<class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam_node<T, size>::get_entry_head()
	{
		++m_access_counter;
		return m_entries;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_node<T, size>::get_list_next()
	{
		return m_list_next;
	}

	template<class T, size_t size>
	void
	soft_tcam_node<T, size>::set_access_counter(std::uint64_t access_counter)
	{
		m_access_counter = access_counter;
	}

	template<class T, size_t size>
	std::uint64_t
	soft_tcam_node<T, size>::get_access_counter()
	{
		return m_access_counter;
	}

	template<class T, size_t size>
	int
	soft_tcam_node<T, size>::insert_entry(soft_tcam_entry<T, size> *entry)
	{
		soft_tcam_entry<T, size> *curr, *prev;

		if (entry == nullptr) {
			std::cerr << "insert_entry: entry is nullptr." << std::endl;
			return -1;
		}

		if (entry->get_node() != nullptr) {
			std::cerr << "insert_entry: entry has already inserted?" << std::endl;
			return -1;
		}

		entry->set_node(this);

		if (m_entries == nullptr) {
			entry->set_next(nullptr);
			entry->set_prev(nullptr);
			m_entries = entry;
			return 0;
		}

		if (entry->get_priority() > m_entries->get_priority()) {
			entry->set_next(m_entries);
			entry->set_prev(nullptr);
			m_entries->set_prev(entry);
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
		entry->set_prev(prev);
		if (curr != nullptr) {
			curr->set_prev(entry);
		}
		prev->set_next(entry);

		return 0;
	}

	template<class T, size_t size>
	int
	soft_tcam_node<T, size>::erase_entry(soft_tcam_entry<T, size> *entry)
	{
		soft_tcam_entry<T, size> *curr, *prev;

		if ((m_entries == nullptr) || (entry == nullptr)) {
			std::cerr << "erase_entry: m_entries or entry is nullptr" << std::endl;
			return -1;
		}

		if (m_entries == entry) {
			m_entries = entry->get_next();
			if (m_entries != nullptr) {
				m_entries->set_prev(nullptr);
			}
			entry->set_next(nullptr);
			entry->set_prev(nullptr);
			entry->set_node(nullptr);
			delete entry;
			return 0;
		}

		prev = m_entries;
		curr = m_entries->get_next();
		while (curr != nullptr) {
			if (curr == entry) {
				prev->set_next(entry->get_next());
				if (entry->get_next() != nullptr) {
					entry->get_next()->set_prev(prev);
				}
				entry->set_next(nullptr);
				entry->set_prev(nullptr);
				entry->set_node(nullptr);
				delete entry;
				return 0;
			}
			prev = curr;
			curr = curr->get_next();
		}

		std::cerr << "erase_entry: ???" << std::endl;
		return -1;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam_node<T, size>::get_list_head()
	{
		return s_list_head;
	}

	template<class T, size_t size>
	std::uint64_t
	soft_tcam_node<T, size>::get_alloc_counter()
	{
		return s_alloc_counter;
	}

	template<class T, size_t size>
		soft_tcam_node<T, size> *soft_tcam_node<T, size>::s_list_head = nullptr;
	template<class T, size_t size>
		std::uint64_t soft_tcam_node<T, size>::s_alloc_counter = 0;

}

