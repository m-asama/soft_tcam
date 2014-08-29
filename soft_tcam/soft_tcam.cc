/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>
#include <map>
#include <queue>
#include <cstdlib>
#include <cstdio>

#include "soft_tcam_node.h"
#include "soft_tcam_entry.h"

#include "soft_tcam.h"

namespace soft_tcam {

	template<class T, size_t size>
	soft_tcam<T, size>::soft_tcam()
	{
		m_root = nullptr;

		m_list_next = s_list_head;
		s_list_head = this;
	}

	template<class T, size_t size>
	soft_tcam<T, size>::~soft_tcam()
	{
		soft_tcam<T, size> *curr, *prev;

		if (m_root != nullptr) {
			destroy_node(m_root);
			m_root = nullptr;
		}

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
	soft_tcam<T, size>::operator new(size_t s)
	{
		++s_alloc_counter;
		return malloc(s);
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::operator delete(void *p)
	{
		--s_alloc_counter;
		free(p);
	}

	template<class T, size_t size>
	int
	soft_tcam<T, size>::insert(const std::bitset<size> &data, const std::bitset<size> &mask, std::uint32_t priority,
			const T &object)
	{
		soft_tcam_entry<T, size> *entry;
		soft_tcam_node<T, size> *node, *nearest, *temp;
		std::uint64_t i;

		for (i = 0; i < size; ++i) {
			if ((mask[i] == 0) && (data[i] == 1)) {
				std::cerr << "insert: data/mask error." << std::endl;
				return -1;
			}
		}

		entry = new soft_tcam_entry<T, size>();
		entry->set_priority(priority);
		entry->set_object(object);

		if (m_root == nullptr) {
			node = new soft_tcam_node<T, size>(data, mask, size);
			node->insert_entry(entry);
			entry->set_node(node);
			m_root = node;
			return 0;
		}

		nearest = find_nearest_node(data, mask);

		if (nearest == nullptr) {
			node = new soft_tcam_node<T, size>(data, mask, size);
			node->insert_entry(entry);
			entry->set_node(node);
			return insert_between(nullptr, m_root, node);
		}

		if (nearest->get_position() == size) {
			nearest->insert_entry(entry);
			entry->set_node(nearest);
			return 0;
		}

		node = new soft_tcam_node<T, size>(data, mask, size);
		node->insert_entry(entry);
		entry->set_node(node);

		if (mask[nearest->get_position()] == 0) {
			temp = nearest->get_ndc();
			if (temp == nullptr) {
				nearest->set_ndc(node);
				node->set_parent(nearest);
				return 0;
			}
		} else if (data[nearest->get_position()] == 0) {
			temp = nearest->get_n0();
			if (temp == nullptr) {
				nearest->set_n0(node);
				node->set_parent(nearest);
				return 0;
			}
		} else {
			temp = nearest->get_n1();
			if (temp == nullptr) {
				nearest->set_n1(node);
				node->set_parent(nearest);
				return 0;
			}
		}

		return insert_between(nearest, temp, node);
	}

	template<class T, size_t size>
	int
	soft_tcam<T, size>::erase(const std::bitset<size> &data, const std::bitset<size> &mask, std::uint32_t priority,
			const T &object)
	{
		soft_tcam_node<T, size> *node;
		soft_tcam_entry<T, size> *entry;
		bool found = false;

		node = find_nearest_node(data, mask);
		if ((node == nullptr)
		 || (node->get_position() != size)) {
			std::cerr << "erase: node not found." << std::endl;
			return -1;
		}

		entry = node->get_entry_head();
		while (entry != nullptr) {
			if ((entry->get_priority() == priority)
			 && (entry->get_object() == object)) {
				found = true;
				node->erase_entry(entry);
				break;
			}
			entry = entry->get_next();
		}

		if (!found) {
			std::cerr << "erase: entry not found." << std::endl;
			return -1;
		}

		if (node->get_entry_head() == nullptr) {
			erase_node(node);
		}

		return 0;
	}

	template<class T, size_t size>
	const T *
	soft_tcam<T, size>::find(const std::bitset<size> &key)
	{
		const T *p = nullptr;
		soft_tcam_entry<T, size> *entry;

		entry = find_entry(key);
		if (entry != nullptr) {
			p = &entry->get_object();
		}

		return p;
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::dump()
	{
		std::cout << " depth            data ";
		for (int i = 5; i < size; ++i) { std::cout << " "; }
		std::cout << " parent           node             node->n0         node->n1         node->ndc"
			  << std::endl;
		std::cout << " node->position   mask ";
		for (int i = 5; i < size; ++i) { std::cout << " "; }
		std::cout << " entry->priority  entry->node      entry            ..."
			  << std::endl;
		if (m_root != nullptr)
			dump_node(m_root, 0);
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::destroy_node(soft_tcam_node<T, size> *node)
	{
		soft_tcam_entry<T, size> *entry;

		entry = node->get_entry_head();
		while (entry != nullptr) {
			soft_tcam_entry<T, size> *temp = entry->get_next();
			node->erase_entry(entry);
			entry = temp;
		}

		if (node->get_n0() != nullptr) {
			destroy_node(node->get_n0());
		}
		if (node->get_n1() != nullptr) {
			destroy_node(node->get_n1());
		}
		if (node->get_ndc() != nullptr) {
			destroy_node(node->get_ndc());
		}

		node->set_n0(nullptr);
		node->set_n1(nullptr);
		node->set_ndc(nullptr);
		node->set_parent(nullptr);
		delete node;
	}

	template<class T, size_t size>
	int
	soft_tcam<T, size>::insert_between(soft_tcam_node<T, size> *less, soft_tcam_node<T, size> *more,
			soft_tcam_node<T, size> *node)
	{
		soft_tcam_node<T, size> *temp;
		std::bitset<size> data, mask;
		std::uint32_t position;

		data.reset();
		mask.reset();
		position = 0;

		for (std::uint32_t i = 0; i < size; ++i) {
			if ((more->get_data()[i] != node->get_data()[i])
			 || (more->get_mask()[i] != node->get_mask()[i])) {
				break;
			}
			data.set(i, node->get_data()[i]);
			mask.set(i, node->get_mask()[i]);
			position = i + 1;
		}

		temp = new soft_tcam_node<T, size>(data, mask, position);

		if (less == nullptr) {
			m_root = temp;
		} else {
			if (temp->get_mask()[less->get_position()] == 0) {
				less->set_ndc(temp);
			} else if (temp->get_data()[less->get_position()] == 0) {
				less->set_n0(temp);
			} else {
				less->set_n1(temp);
			}
			temp->set_parent(less);
		}

		if (more->get_mask()[temp->get_position()] == 0) {
			temp->set_ndc(more);
		} else if (more->get_data()[temp->get_position()] == 0) {
			temp->set_n0(more);
		} else {
			temp->set_n1(more);
		}
		more->set_parent(temp);

		if (node->get_mask()[temp->get_position()] == 0) {
			temp->set_ndc(node);
		} else if (node->get_data()[temp->get_position()] == 0) {
			temp->set_n0(node);
		} else {
			temp->set_n1(node);
		}
		node->set_parent(temp);

		return 0;
	}

	template<class T, size_t size>
	int
	soft_tcam<T, size>::erase_node(soft_tcam_node<T, size> *node)
	{
		soft_tcam_node<T, size> *parent;
		bool has_child;

		if ((node->get_n1() != nullptr)
		 || (node->get_n0() != nullptr)
		 || (node->get_ndc() != nullptr)
		 || (node->get_entry_head() != nullptr)) {
			std::cerr << "erase_node: node in use." << std::endl;
			return -1;
		}

		parent = node->get_parent();
		if (parent == nullptr) {
			m_root = nullptr;
			node->set_n0(nullptr);
			node->set_n1(nullptr);
			node->set_ndc(nullptr);
			node->set_parent(nullptr);
			delete node;
			return 0;
		}

		has_child = false;
		if (parent->get_n0() != nullptr) {
			if (parent->get_n0() == node) {
				parent->set_n0(nullptr);
			} else {
				has_child = true;
			}
		}
		if (parent->get_n1() != nullptr) {
			if (parent->get_n1() == node) {
				parent->set_n1(nullptr);
			} else {
				has_child = true;
			}
		}
		if (parent->get_ndc() != nullptr) {
			if (parent->get_ndc() == node) {
				parent->set_ndc(nullptr);
			} else {
				has_child = true;
			}
		}

		node->set_n0(nullptr);
		node->set_n1(nullptr);
		node->set_ndc(nullptr);
		node->set_parent(nullptr);
		delete node;

		if (!has_child) {
			erase_node(parent);
		}

		return 0;
	}

	template<class T, size_t size>
	soft_tcam_node<T, size> *
	soft_tcam<T, size>::find_nearest_node(const std::bitset<size> &data, const std::bitset<size> &mask)
	{
		soft_tcam_node<T, size> *node, *temp;
		std::uint32_t position;

		if (m_root == nullptr) {
			return nullptr;
		}

		position = 0;
		node = m_root;
		while (node != nullptr) {
			for (std::uint32_t i = position; i < node->get_position(); ++i) {
				if ((node->get_data()[i] != data[i])
				 || (node->get_mask()[i] != mask[i])) {
					return node->get_parent();
				}
			}
			if (node->get_position() == size) {
				return node;
			}
			position = node->get_position();
			if (mask[position] == 0) {
				temp = node->get_ndc();
			} else if (data[position] == 0) {
				temp = node->get_n0();
			} else {
				temp = node->get_n1();
			}
			if (temp == nullptr) {
				return node;
			}
			node = temp;
		}

		return nullptr;
	}

	template<class T, size_t size>
	soft_tcam_entry<T, size> *
	soft_tcam<T, size>::find_entry(const std::bitset<size> &key)
	{
		soft_tcam_entry<T, size> *entry = nullptr;
		soft_tcam_node<T, size> *node, *temp_node;
		soft_tcam_node<T, size> *stack_node[size], **stack_node_ptr = &stack_node[0];
		size_t stack_size[size], *stack_size_ptr = &stack_size[0];
		size_t prev, curr;

		prev = 0;
		node = m_root;
retry:
		while (node != nullptr) {
			bool match = true;
			const std::bitset<size> &data = node->get_data();
			const std::bitset<size> &mask = node->get_mask();
			curr = node->get_position();
			for (size_t i = prev; i < curr; ++i) {
				if ((mask[i] == 1) && (key[i] != data[i])) {
					match = false;
					break;
				}
			}
			if (match == false) {
				break;
			}
			if (curr == size) {
				soft_tcam_entry<T, size> *temp_entry = node->get_entry_head();
				if ((entry == nullptr)
				 || (temp_entry->get_priority() > entry->get_priority())) {
					entry = temp_entry;
				}
			}
			temp_node = nullptr;
			if (key[curr] == 0) {
				temp_node = node->get_n0();
			}
			if (key[curr] == 1) {
				temp_node = node->get_n1();
			}
			if (node->get_ndc() != nullptr) {
				if (temp_node != nullptr) {
					*stack_node_ptr = node->get_ndc();
					*stack_size_ptr = curr;
					++stack_node_ptr;
					++stack_size_ptr;
				} else {
					temp_node = node->get_ndc();
				}
			}
			prev = curr;
			node = temp_node;
		}
		if (stack_node_ptr != &stack_node[0]) {
			--stack_node_ptr;
			--stack_size_ptr;
			prev = *stack_size_ptr;
			node = *stack_node_ptr;
			goto retry;
		}

		return entry;
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::dump_node(soft_tcam_node<T, size> *node, int depth)
	{
		char buf1[19];
		char buf2[19];
		char buf3[256];
		char buf4[256];
		soft_tcam_entry<T, size> *entry;

		if (node == nullptr) {
			return;
		}

		for (int i = 0; i < 18; ++i) {
			buf1[i] = ' ';
		}
		if (depth + 1 < 17) {
			buf1[depth + 1] = 'X';
		} else {
			buf1[16] = '_';
		}
		buf1[18] = '\0';

		snprintf(buf2, 19, " %16d ", node->get_position());

		snprintf(buf3, 256, " %016lx %016lx %016lx %016lx %016lx",
			       	(unsigned long)node->get_parent(),
				(unsigned long)node,
				(unsigned long)node->get_n0(),
				(unsigned long)node->get_n1(),
				(unsigned long)node->get_ndc());

		std::cout << " ----------------------";
		for (int i = 5; i < size; ++i) { std::cout << "-"; }
		std::cout << " ------------------------------------------------------------------------------------ "
			  << std::endl;
		std::cout << buf1
			  << node->get_data().to_string()
			  << buf3
			  << std::endl;
		std::cout << buf2
			  << node->get_mask().to_string();
		entry = node->get_entry_head();
		while (entry != nullptr) {
			snprintf(buf4, 256, " %016lx %016lx %016lx",
					(unsigned long)entry->get_priority(),
					(unsigned long)entry->get_node(),
					(unsigned long)entry);
			std::cout << buf4;
			entry = entry->get_next();
		}
		std::cout << std::endl;

		if (node->get_n0() != nullptr) {
			dump_node(node->get_n0(), depth + 1);
		}
		if (node->get_n1() != nullptr) {
			dump_node(node->get_n1(), depth + 1);
		}
		if (node->get_ndc() != nullptr) {
			dump_node(node->get_ndc(), depth + 1);
		}
	}

	template<class T, size_t size>
	static bool comp_node_by_memory_address(soft_tcam_node<T, size> * &l, soft_tcam_node<T, size> * &r)
	{
		return (l < r);
	}

	template<class T, size_t size>
	static bool comp_node_by_access_counter(soft_tcam_node<T, size> * &l, soft_tcam_node<T, size> * &r)
	{
		return (l->get_access_counter() > r->get_access_counter());
	}

	template<class T, size_t size>
	static bool comp_entry_by_memory_address(soft_tcam_entry<T, size> * &l, soft_tcam_entry<T, size> * &r)
	{
		return (l < r);
	}

	template<class T, size_t size>
	static bool comp_entry_by_access_counter(soft_tcam_entry<T, size> * &l, soft_tcam_entry<T, size> * &r)
	{
		return (l->get_access_counter() > r->get_access_counter());
	}

	template<class T, size_t size>
	static void
	sort_nodes(std::vector<soft_tcam_node<T, size> *> &nv1, std::vector<soft_tcam_node<T, size> *> &nv2,
			std::map<soft_tcam_node<T, size> *, soft_tcam_node<T, size> *> &nm,
			std::map<soft_tcam_entry<T, size> *, soft_tcam_entry<T, size> *> &em)
	{
		std::cerr << "Sorting nodes...";
		std::bitset<size> *data = new std::bitset<size>[nv1.size()];
		std::bitset<size> *mask = new std::bitset<size>[nv1.size()];
		std::uint32_t *position = new std::uint32_t[nv1.size()];
		soft_tcam_node<T, size> **n0 = new soft_tcam_node<T, size> *[nv1.size()];
		soft_tcam_node<T, size> **n1 = new soft_tcam_node<T, size> *[nv1.size()];
		soft_tcam_node<T, size> **ndc = new soft_tcam_node<T, size> *[nv1.size()];
		soft_tcam_node<T, size> **parent = new soft_tcam_node<T, size> *[nv1.size()];
		soft_tcam_entry<T, size> **entries = new soft_tcam_entry<T, size> *[nv1.size()];
		std::uint64_t *access_counter = new std::uint64_t[nv1.size()];
		for (std::uint32_t i = 0; i < nv1.size(); ++i) {
			data[i] = nv1[i]->get_data();
			mask[i] = nv1[i]->get_mask();
			position[i] = nv1[i]->get_position();
			n0[i] = nv1[i]->get_n0();
			n1[i] = nv1[i]->get_n1();
			ndc[i] = nv1[i]->get_ndc();
			parent[i] = nv1[i]->get_parent();
			entries[i] = nv1[i]->get_entry_head();
			access_counter[i] = nv1[i]->get_access_counter();
		}
		for (std::uint32_t i = 0; i < nv1.size(); ++i) {
			nv2[i]->set_data(data[i]);
			nv2[i]->set_mask(mask[i]);
			nv2[i]->set_position(position[i]);
			nv2[i]->set_n0(nm.at(n0[i]));
			nv2[i]->set_n1(nm.at(n1[i]));
			nv2[i]->set_ndc(nm.at(ndc[i]));
			nv2[i]->set_parent(nm.at(parent[i]));
			nv2[i]->set_entry_head(em.at(entries[i]));
			nv2[i]->set_access_counter(access_counter[i]);
		}
		delete[] data;
		delete[] mask;
		delete[] position;
		delete[] n0;
		delete[] n1;
		delete[] ndc;
		delete[] parent;
		delete[] entries;
		delete[] access_counter;
		std::cerr << "done." << std::endl;
	}

	template<class T, size_t size>
	static void
	sort_entries(std::vector<soft_tcam_entry<T, size> *> &ev1, std::vector<soft_tcam_entry<T, size> *> &ev2,
			std::map<soft_tcam_node<T, size> *, soft_tcam_node<T, size> *> &nm,
			std::map<soft_tcam_entry<T, size> *, soft_tcam_entry<T, size> *> &em)
	{
		std::cerr << "Sorting entries...";
		std::uint32_t *priority = new std::uint32_t[ev1.size()];
		T *object = new T[ev1.size()];
		soft_tcam_entry<T, size> **next = new soft_tcam_entry<T, size> *[ev1.size()];
		soft_tcam_entry<T, size> **prev = new soft_tcam_entry<T, size> *[ev1.size()];
		soft_tcam_node<T, size> **node = new soft_tcam_node<T, size> *[ev1.size()];
		std::uint64_t *access_counter = new std::uint64_t[ev1.size()];
		for (std::uint32_t i = 0; i < ev1.size(); ++i) {
			priority[i] = ev1[i]->get_priority();
			object[i] = ev1[i]->get_object();
			next[i] = ev1[i]->get_next();
			prev[i] = ev1[i]->get_prev();
			node[i] = ev1[i]->get_node();
			access_counter[i] = ev1[i]->get_access_counter();
		}
		for (std::uint32_t i = 0; i < ev1.size(); ++i) {
			ev2[i]->set_priority(priority[i]);
			ev2[i]->set_object(object[i]);
			ev2[i]->set_next(em.at(next[i]));
			ev2[i]->set_prev(em.at(prev[i]));
			ev2[i]->set_node(nm.at(node[i]));
			ev2[i]->set_access_counter(access_counter[i]);
		}
		delete[] priority;
		delete[] object;
		delete[] next;
		delete[] prev;
		delete[] node;
		delete[] access_counter;
		std::cerr << "done." << std::endl;
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::sort_best()
	{
		std::vector<soft_tcam_node<T, size> *> nv1, nv2;
		std::vector<soft_tcam_entry<T, size> *> ev1, ev2;
		std::map<soft_tcam_node<T, size> *, soft_tcam_node<T, size> *> nm;
		std::map<soft_tcam_entry<T, size> *, soft_tcam_entry<T, size> *> em;
		soft_tcam_node<T, size> *node;
		soft_tcam_entry<T, size> *entry;
		soft_tcam<T, size> *tcam;

		std::cerr << "Making sorted node index...";
		node = soft_tcam_node<T, size>::get_list_head();
		while (node != nullptr) {
			nv1.push_back(node);
			node = node->get_list_next();
		}
		nv2 = nv1;
		std::sort(nv1.begin(), nv1.end(), comp_node_by_access_counter<T, size>);
		std::sort(nv2.begin(), nv2.end(), comp_node_by_memory_address<T, size>);
		for (std::uint32_t i = 0; i < nv1.size(); ++i) {
			nm.insert(std::make_pair(nv1[i], nv2[i]));
		}
		nm.insert(std::make_pair(nullptr, nullptr));
		std::cerr << "done." << std::endl;

		std::cerr << "Making sorted entry index...";
		entry = soft_tcam_entry<T, size>::get_list_head();
		while (entry != nullptr) {
			ev1.push_back(entry);
			entry = entry->get_list_next();
		}
		ev2 = ev1;
		std::sort(ev1.begin(), ev1.end(), comp_entry_by_access_counter<T, size>);
		std::sort(ev2.begin(), ev2.end(), comp_entry_by_memory_address<T, size>);
		for (std::uint32_t i = 0; i < ev1.size(); ++i) {
			em.insert(std::make_pair(ev1[i], ev2[i]));
		}
		em.insert(std::make_pair(nullptr, nullptr));
		std::cerr << "done." << std::endl;

		/*
		for (std::uint32_t i = 0; i < nv1.size(); ++i) {
			std::cerr << "nv1[" << i << "]:" << nv1[i]
				  << " <=> "
				  << "nv2[" << i << "]:" << nv2[i]
				  << " " << nv1[i]->get_access_counter()
				  << std::endl;
		}
		*/

		tcam = s_list_head;
		while (tcam != nullptr) {
			tcam->m_root = nm.at(tcam->m_root);
			tcam = tcam->m_list_next;
		}
		sort_nodes(nv1, nv2, nm, em);
		sort_entries(ev1, ev2, nm, em);
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::sort_worst()
	{
		std::vector<soft_tcam_node<T, size> *> nv1, nv2;
		std::vector<soft_tcam_entry<T, size> *> ev1, ev2;
		std::map<soft_tcam_node<T, size> *, soft_tcam_node<T, size> *> nm;
		std::map<soft_tcam_entry<T, size> *, soft_tcam_entry<T, size> *> em;
		std::map<std::uint64_t, std::queue<soft_tcam_node<T, size> *>> nvm;
		std::map<std::uint64_t, std::queue<soft_tcam_entry<T, size> *>> evm;
		soft_tcam_node<T, size> *node;
		soft_tcam_entry<T, size> *entry;
		soft_tcam<T, size> *tcam;

		std::cerr << "Making sorted node index...";
		node = soft_tcam_node<T, size>::get_list_head();
		while (node != nullptr) {
			nv1.push_back(node);
			node = node->get_list_next();
		}
		nv2 = nv1;
		std::sort(nv1.begin(), nv1.end(), comp_node_by_access_counter<T, size>);
		std::sort(nv2.begin(), nv2.end(), comp_node_by_memory_address<T, size>);
		for (std::uint32_t i = 0; i < nv2.size(); ++i) {
			std::uint64_t k = reinterpret_cast<std::uint64_t>(nv2[i]) / 4096;
			if (nvm.count(k) > 0) {
				nvm.at(k).push(nv2[i]);
			} else {
				std::queue<soft_tcam_node<T, size> *> q;
				q.push(nv2[i]);
				nvm.insert(std::make_pair(k, q));
			}
		}
		std::uint32_t nremain = nv2.size();
		nv2.clear();
		while (nremain > 0) {
			for (auto it = nvm.begin(); it != nvm.end(); ++it) {
				if (!it->second.empty()) {
					nv2.push_back(it->second.front());
					it->second.pop();
					--nremain;
				}
			}
		}
		for (std::uint32_t i = 0; i < nv1.size(); ++i) {
			nm.insert(std::make_pair(nv1[i], nv2[i]));
		}
		nm.insert(std::make_pair(nullptr, nullptr));
		std::cerr << "done." << std::endl;

		std::cerr << "Making sorted entry index...";
		entry = soft_tcam_entry<T, size>::get_list_head();
		while (entry != nullptr) {
			ev1.push_back(entry);
			entry = entry->get_list_next();
		}
		ev2 = ev1;
		std::sort(ev1.begin(), ev1.end(), comp_entry_by_access_counter<T, size>);
		std::sort(ev2.begin(), ev2.end(), comp_entry_by_memory_address<T, size>);
		for (std::uint32_t i = 0; i < ev2.size(); ++i) {
			std::uint64_t k = reinterpret_cast<std::uint64_t>(ev2[i]) / 4096;
			if (evm.count(k) > 0) {
				evm.at(k).push(ev2[i]);
			} else {
				std::queue<soft_tcam_entry<T, size> *> q;
				q.push(ev2[i]);
				evm.insert(std::make_pair(k, q));
			}
		}
		std::uint32_t eremain = ev2.size();
		ev2.clear();
		while (eremain > 0) {
			for (auto it = evm.begin(); it != evm.end(); ++it) {
				if (!it->second.empty()) {
					ev2.push_back(it->second.front());
					it->second.pop();
					--eremain;
				}
			}
		}
		for (std::uint32_t i = 0; i < ev1.size(); ++i) {
			em.insert(std::make_pair(ev1[i], ev2[i]));
		}
		em.insert(std::make_pair(nullptr, nullptr));
		std::cerr << "done." << std::endl;

		/*
		for (std::uint32_t i = 0; i < nv1.size(); ++i) {
			std::cerr << "nv1[" << i << "]:" << nv1[i]
				  << " <=> "
				  << "nv2[" << i << "]:" << nv2[i]
				  << " " << nv1[i]->get_access_counter()
				  << std::endl;
		}
		for (std::uint32_t i = 0; i < ev1.size(); ++i) {
			std::cerr << "ev1[" << i << "]:" << ev1[i]
				  << " <=> "
				  << "ev2[" << i << "]:" << ev2[i]
				  << " " << ev1[i]->get_access_counter()
				  << std::endl;
		}
		*/

		tcam = s_list_head;
		while (tcam != nullptr) {
			tcam->m_root = nm.at(tcam->m_root);
			tcam = tcam->m_list_next;
		}
		sort_nodes(nv1, nv2, nm, em);
		sort_entries(ev1, ev2, nm, em);
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::clear_access_counter()
	{
		soft_tcam_node<T, size> *node;
		soft_tcam_entry<T, size> *entry;

		node = soft_tcam_node<T, size>::get_list_head();
		while (node != nullptr) {
			node->set_access_counter(0);
			node = node->get_list_next();
		}

		entry = soft_tcam_entry<T, size>::get_list_head();
		while (entry != nullptr) {
			entry->set_access_counter(0);
			entry = entry->get_list_next();
		}
	}

	template<class T, size_t size>
	void
	soft_tcam<T, size>::dump_access_counter()
	{
		soft_tcam_node<T, size> *node;
		soft_tcam_entry<T, size> *entry;
		std::uint64_t n = 0, e = 0;

		node = soft_tcam_node<T, size>::get_list_head();
		while (node != nullptr) {
			std::cout << "N\t" << node << "\t" << node->get_access_counter() << std::endl;
			n += node->get_access_counter();
			node = node->get_list_next();
		}

		entry = soft_tcam_entry<T, size>::get_list_head();
		while (entry != nullptr) {
			std::cout << "E\t" << entry << "\t" << entry->get_access_counter() << std::endl;
			e += entry->get_access_counter();
			entry = entry->get_list_next();
		}

		std::cout << " node total access  : " << n << std::endl;
		std::cout << " entry total access : " << e << std::endl;
	}

	template<class T, size_t size>
		soft_tcam<T, size> *soft_tcam<T, size>::s_list_head = nullptr;
	template<class T, size_t size>
		std::uint64_t soft_tcam<T, size>::s_alloc_counter = 0;

}

