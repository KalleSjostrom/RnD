	/* Statistics
		Each space keeps track of current and maximum system memory
		obtained via MORECORE or MMAP. */

	/* Locking
		If USE_LOCKS is defined, the "mutex" lock is acquired and released
		around every public call using this mspace. */

struct malloc_state {

	/* Bin maps
		There is one bit map for small bins ("smallmap") and one for
		treebins ("treemap).  Each bin sets its bit when non-empty, and
		clears the bit when empty.  Bit operations are then used to avoid
		bin-by-bin searching -- nearly all "search" is done without ever
		looking at bins that won't be selected.  The bit maps
		conservatively use 32 bits per map word, even if on 64bit system.
		For a good description of some of the bit-based techniques used
		here, see Henry S. Warren Jr's book "Hacker's Delight" (and
		supplement at http://hackersdelight.org/). Many of these are
		intended to reduce the branchiness of paths through malloc etc, as
		well as to reduce the number of memory locations read or written.*/
	binmap_t   smallmap;
	binmap_t   treemap;

	/* SmallBins
		An array of bin headers for free chunks.  These bins hold chunks
		with sizes less than MIN_LARGE_SIZE bytes. Each bin contains
		chunks of all the same size, spaced 8 bytes apart.  To simplify
		use in double-linked lists, each bin header acts as a malloc_chunk
		pointing to the real first node, if it exists (else pointing to
		itself).  This avoids special-casing for headers.  But to avoid
		waste, we allocate only the fd/bk pointers of bins, and then use
		repositioning tricks to treat these as the fields of a chunk.*/
	mchunkptr  smallbins[(NSMALLBINS+1)*2];

	/* TreeBins
		Treebins are pointers to the roots of trees holding a range of
		sizes. There are 2 equally spaced treebins for each power of two
		from TREE_SHIFT to TREE_SHIFT+16. The last bin holds anything
		larger. */
	tbinptr    treebins[NTREEBINS];

	/* Address check support
		The least_addr field is the least address ever obtained from
		MORECORE or MMAP. Attempted frees and reallocs of any address less
		than this are trapped (unless INSECURE is defined). */
	char*      least_addr; // Don't really need this, only used for ok_address. Could use seg->base instead?

	/* Designated victim (dv)
		This is the preferred chunk for servicing small requests that
		don't have exact fits.  It is normally the chunk split off most
		recently to service another small request.  Its size is cached in
		dvsize. The link fields of this chunk are not maintained since it
		is not kept in a bin.*/
	mchunkptr  dv; // Pretty sure I can zero this out
	size_t     dvsize;

	/* Top
		The topmost chunk of the currently active segment. Its size is
		cached in topsize.  The actual size of topmost space is
		topsize+TOP_FOOT_SIZE, which includes space reserved for adding
		fenceposts and segment records if necessary when getting more
		space from the system.  The size at which to autotrim top is
		cached from mparams in trim_check, except that it is disabled if
		an autotrim fails. */
	mchunkptr  top; // This should be the final chunkptr when we reload, update accordingly.
	size_t     topsize;

	/* Magic tag
		A cross-check field that should always hold same value as mparams.magic. */
	size_t     magic;

	/* Segments
		A list of segments headed by an embedded malloc_segment record
		representing the initial space. */
	msegment   seg;
};
