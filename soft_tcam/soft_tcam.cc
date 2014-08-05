/*
 * Author:
 * 	Masakazu Asama <m-asama@ginzado.co.jp>
 */

#include <iostream>
#include <stack>
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
	}

	template<class T, size_t size>
	soft_tcam<T, size>::~soft_tcam()
	{
		if (m_root != nullptr) {
			destroy_node(m_root);
			m_root = nullptr;
		}
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
		std::cout << " entry->priority  entry->node      entry            entry->pointer   ..."
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
	bool
	soft_tcam<T, size>::key_is_in_data_and_mask(const std::bitset<size> &data, const std::bitset<size> &mask,
			const std::bitset<size> &key)
	{
		bool result = true;

		for (std::uint32_t i = 0; i < size; ++i) {
			if ((mask[i] == 1) && (key[i] != data[i])) {
				result = false;
			}
		}

		return result;
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
		soft_tcam_node<T, size> *node, *temp;
		std::stack<soft_tcam_node<T, size> *> context;

		node = m_root;
retry:
		while (node != nullptr) {
			if ((node->get_position() == size)
			 && (key_is_in_data_and_mask(node->get_data(), node->get_mask(), key))) {
				soft_tcam_entry<T, size> *temp_entry = node->get_entry_head();
				if ((entry == nullptr)
				 || (temp_entry->get_priority() > entry->get_priority())) {
					entry = temp_entry;
				}
			}
			temp = nullptr;
			if (key[node->get_position()] == 0) {
				temp = node->get_n0();
			}
			if (key[node->get_position()] == 1) {
				temp = node->get_n1();
			}
			if (node->get_ndc() != nullptr) {
				if (temp != nullptr) {
					context.push(node->get_ndc());
				} else {
					temp = node->get_ndc();
				}
			}
			node = temp;
		}
		if (!context.empty()) {
			node = context.top();
			context.pop();
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
			snprintf(buf4, 256, " %016lx %016lx %016lx %016lx",
					(unsigned long)entry->get_priority(),
					(unsigned long)entry->get_node(),
					(unsigned long)entry,
					(unsigned long)entry->get_object());
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

}

