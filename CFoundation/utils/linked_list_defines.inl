#pragma once

#define LINK_ENTRY(type_name, key_type) \
	struct type_name { \
		key_type key; \
		type_name *next; \
		type_name *prev; \
	};

#define LINKED_LIST(list_type_name, link_type_name) \
	struct list_type_name { \
		link_type_name *entries; \
		unsigned count; \
		link_type_name *head; \
		link_type_name *tail; \
	};

#define LINK_NEW(entry, _key, _list) \
	auto &entry = (_list).entries[(_list).count++]; \
	{ \
		entry.key = (_key); \
		entry.next = 0; entry.prev = 0; \
		if ((_list).tail != &entry) { \
			(_list).tail->next = &entry; \
			entry.prev = (_list).tail; \
			(_list).tail = &entry; \
		} \
	}

#define LINK_ADD_BEFORE(_link, _cursor, _list) \
	{ \
		if ((_list).tail == &(_link)){ \
			if ((_link).prev){ \
				(_list).tail = (_link).prev; \
				(_list).tail->next = 0; \
			} \
		} \
		auto *prev = (_cursor)->prev; \
		if (prev) { \
			prev->next = &(_link); \
			(_link).prev = prev; \
		} else { \
			(_list).head = &(_link); \
			(_list).head->prev = 0; \
		} \
		(_cursor)->prev = &(_link); \
		(_link).next = (_cursor); \
	}
