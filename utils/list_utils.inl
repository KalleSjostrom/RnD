struct Link {
	u16 next;
	u16 prev;
};

struct ListHeader {
	i16 first_active;
	u16 first_free;
};

inline void remove_from_list(Link *list, i16 id) {
	Link &link = list[id];

	list[link.next].prev = link.prev;
	list[link.prev].next = link.next;
}

inline void insert_in_list(Link *list, i16 at, i16 id) {
	Link &link = list[id];

	Link &left = list[at];
	Link &right = list[left.next];

	link.next = right.prev;
	link.prev = left.next;

	right.prev = id;
	left.next = id;
}

inline void initiate_list(Link *list, int max, ListHeader &header) {
	header.first_active = -1;
	header.first_free = 0;
	list[0].prev = max-1;
	list[0].next = 1;
	for (int i = 1; i < max-1; i++) {
		Link &link = list[i];
		link.prev = i-1;
		link.next = i+1;
	}
	list[max-1].prev = max-2;
	list[max-1].next = 0;
}

inline void activate_first_free(Link *list, ListHeader &header) {
	u16 handle = header.first_free;
	Link &link = list[handle];
	header.first_free = link.next;

	// Remove from freelist
	remove_from_list(list, handle);

	if (header.first_active > 0) { // If we have a first_active, insert the job in the active list.
		insert_in_list(list, header.first_active, handle);
	} else {
		// If we dont have a first_active list, create the head job that just points to itselft.
		link.next = link.prev = handle;
	}

	header.first_active = handle;
}

inline void free_active(Link *list, u16 active_id, ListHeader &header) {
	Link &link = list[active_id];
	header.first_active = link.prev == active_id ? -1 : link.prev;
	remove_from_list(list, active_id);
	insert_in_list(list, header.first_free, active_id);
	header.first_free = active_id;
}
