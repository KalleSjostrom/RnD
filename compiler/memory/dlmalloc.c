/* Version identifier to allow people to support multiple versions */
#ifndef DLMALLOC_VERSION
#define DLMALLOC_VERSION 20806
#endif /* DLMALLOC_VERSION */

/* The maximum possible size_t value has all bits set */
#define MAX_SIZE_T           (~(size_t)0)
#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)(2 * sizeof(void *)))
#endif  /* MALLOC_ALIGNMENT */
#ifndef FOOTERS
#define FOOTERS 0
#endif  /* FOOTERS */
#ifndef ABORT
#define ABORT  abort()
#endif  /* ABORT */
#ifndef ABORT_ON_ASSERT_FAILURE
#define ABORT_ON_ASSERT_FAILURE 1
#endif  /* ABORT_ON_ASSERT_FAILURE */
#ifndef PROCEED_ON_ERROR
#define PROCEED_ON_ERROR 0
#endif  /* PROCEED_ON_ERROR */

#ifndef INSECURE
#define INSECURE 0
#endif  /* INSECURE */
#ifndef MALLOC_FAILURE_ACTION
#define MALLOC_FAILURE_ACTION  errno = ENOMEM;
#endif  /* MALLOC_FAILURE_ACTION */
#ifndef USE_BUILTIN_FFS
#define USE_BUILTIN_FFS 0
#endif  /* USE_BUILTIN_FFS */
#ifndef USE_DEV_RANDOM
#define USE_DEV_RANDOM 0
#endif  /* USE_DEV_RANDOM */

/*
	Try to persuade compilers to inline. The most critical functions for
	inlining are defined as macros, so these aren't used for them.
*/
#ifndef FORCEINLINE
	#if defined(__GNUC__)
#define FORCEINLINE __inline __attribute__ ((always_inline))
	#elif defined(_MSC_VER)
		#define FORCEINLINE __forceinline
	#endif
#endif
#ifndef NOINLINE
	#if defined(__GNUC__)
		#define NOINLINE __attribute__ ((noinline))
	#elif defined(_MSC_VER)
		#define NOINLINE __declspec(noinline)
	#else
		#define NOINLINE
	#endif
#endif

/*
	mspace is an opaque type representing an independent
	region of space that supports mspace_malloc, etc.
*/
typedef void* mspace;

/*
	========================================================================
	To make a fully customizable malloc.h header file, cut everything
	above this line, put into file malloc.h, edit to suit, and #include it
	on the next line, as well as in programs that use this malloc.
	========================================================================
*/

/*------------------------------ internal #includes ---------------------- */
#ifdef _MSC_VER
#pragma warning( disable : 4146 ) /* no "unsigned" warnings */
#endif /* _MSC_VER */
#ifdef DEBUG
#if ABORT_ON_ASSERT_FAILURE
#undef assert
#define assert(x) if(!(x)) ABORT
#else /* ABORT_ON_ASSERT_FAILURE */
#include <assert.h>
#endif /* ABORT_ON_ASSERT_FAILURE */
#else  /* DEBUG */
#ifndef assert
#define assert(x)
#endif
#define DEBUG 0
#endif /* DEBUG */
#ifndef LACKS_STDLIB_H
// #include <stdlib.h>      /* for abort() */
#endif /* LACKS_STDLIB_H */
#ifndef LACKS_UNISTD_H
// #include <unistd.h>     /* for sbrk, sysconf */
#else /* LACKS_UNISTD_H */
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__)
extern void*     sbrk(ptrdiff_t);
#endif /* FreeBSD etc */
#endif /* LACKS_UNISTD_H */

/* Declarations for bit scanning on win32 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
unsigned char _BitScanForward(unsigned long *index, unsigned long mask);
unsigned char _BitScanReverse(unsigned long *index, unsigned long mask);
#ifdef __cplusplus
}
#endif /* __cplusplus */

// #define BitScanForward _BitScanForward
// #define BitScanReverse _BitScanReverse
// #pragma intrinsic(_BitScanForward)
// #pragma intrinsic(_BitScanReverse)
// #endif /* BitScanForward */
// #endif /* defined(_MSC_VER) && _MSC_VER>=1300 */

/* ------------------- size_t and alignment properties -------------------- */

/* The byte and bit size of a size_t */
#define SIZE_T_SIZE         (sizeof(size_t))
#define SIZE_T_BITSIZE      (sizeof(size_t) << 3)

/* Some constants coerced to size_t */
/* Annoying but necessary to avoid errors on some platforms */
#define SIZE_T_ZERO         ((size_t)0)
#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZE_T_FOUR         ((size_t)4)
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define FOUR_SIZE_T_SIZES   (SIZE_T_SIZE<<2)
#define SIX_SIZE_T_SIZES    (FOUR_SIZE_T_SIZES+TWO_SIZE_T_SIZES)
#define HALF_MAX_SIZE_T     (MAX_SIZE_T / 2U)

/* The bit mask value corresponding to MALLOC_ALIGNMENT */
#define CHUNK_ALIGN_MASK    (MALLOC_ALIGNMENT - SIZE_T_ONE)

/* True if address a has acceptable alignment */
#define is_aligned(A)       (((size_t)((A)) & (CHUNK_ALIGN_MASK)) == 0)

/* the number of bytes to offset an address to align it */
#define align_offset(A)\
 ((((size_t)(A) & CHUNK_ALIGN_MASK) == 0)? 0 :\
	((MALLOC_ALIGNMENT - ((size_t)(A) & CHUNK_ALIGN_MASK)) & CHUNK_ALIGN_MASK))

/* -----------------------  Chunk representations ------------------------ */
struct malloc_chunk {
	size_t               prev_foot;  /* Size of previous chunk (if free).  */
	size_t               head;       /* Size and inuse bits. */
	struct malloc_chunk* fd;         /* double links -- used only if free. */
	struct malloc_chunk* bk;
};

typedef struct malloc_chunk  mchunk;
typedef struct malloc_chunk* mchunkptr;
typedef struct malloc_chunk* sbinptr;  /* The type of bins of chunks */
typedef unsigned int bindex_t;         /* Described below */
typedef unsigned int binmap_t;         /* Described below */
typedef unsigned int flag_t;           /* The type of various bit flag sets */

/* ------------------- Chunks sizes and alignments ----------------------- */

#define MCHUNK_SIZE         (sizeof(mchunk))

#if FOOTERS
#define CHUNK_OVERHEAD      (TWO_SIZE_T_SIZES)
#else /* FOOTERS */
#define CHUNK_OVERHEAD      (SIZE_T_SIZE)
#endif /* FOOTERS */

/* The smallest size we can malloc is an aligned minimal chunk */
#define MIN_CHUNK_SIZE\
	((MCHUNK_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

/* conversion from malloc headers to user pointers, and back */
#define chunk2mem(p)        ((void*)((char*)(p)       + TWO_SIZE_T_SIZES))
#define mem2chunk(mem)      ((mchunkptr)((char*)(mem) - TWO_SIZE_T_SIZES))
/* chunk associated with aligned address A */
#define align_as_chunk(A)   (mchunkptr)((A) + align_offset(chunk2mem(A)))

/* Bounds on request (not chunk) sizes. */
#define MAX_REQUEST         ((-MIN_CHUNK_SIZE) << 2)
#define MIN_REQUEST         (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)

/* pad request bytes into a usable size */
#define pad_request(req) \
	 (((req) + CHUNK_OVERHEAD + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)

inline size_t estimate_unpadded_request(size_t padded_request) {
	// We can't get back the exact unpadded request since it's aligned
	// However, we can remove the chunk overhead and always be within MALLOC_ALIGNMENT bytes,
	// i.e. pad_request() of the return value must be equal to the input.
	size_t chunk_overhead = CHUNK_OVERHEAD;
	assert(padded_request >= chunk_overhead); // , "The input request is to small to have been padded! (padded_request=%lu)", padded_request);
	size_t unpadded = padded_request - chunk_overhead;
	assert(pad_request(unpadded) == padded_request);
	return unpadded;
}

/* pad request, checking for minimum (but not maximum) */
#define request2size(req) \
	(((req) < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(req))


/* ------------------ Operations on head and foot fields ----------------- */

/*
	The head field of a chunk is or'ed with PINUSE_BIT when previous
	adjacent chunk in use, and or'ed with CINUSE_BIT if this chunk is in
	use, unless mmapped, in which case both bits are cleared.
*/

#define PINUSE_BIT          (SIZE_T_ONE)
#define CINUSE_BIT          (SIZE_T_TWO)
#define INUSE_BITS          (PINUSE_BIT|CINUSE_BIT)
#define FLAG_BITS           (PINUSE_BIT|CINUSE_BIT)

/* Head value for fenceposts */
#define FENCEPOST_HEAD      (INUSE_BITS|SIZE_T_SIZE)

/* extraction of fields from head words */
#define cinuse(p)           ((p)->head & CINUSE_BIT)
#define pinuse(p)           ((p)->head & PINUSE_BIT)
#define is_inuse(p)         (((p)->head & INUSE_BITS) != PINUSE_BIT)

#define chunksize(p)        ((p)->head & ~(FLAG_BITS))

#define clear_pinuse(p)     ((p)->head &= ~PINUSE_BIT)

/* Treat space at ptr +/- offset as a chunk */
#define chunk_plus_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))
#define chunk_minus_offset(p, s) ((mchunkptr)(((char*)(p)) - (s)))

/* Ptr to next or previous physical malloc_chunk. */
#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->head & ~FLAG_BITS)))
#define prev_chunk(p) ((mchunkptr)( ((char*)(p)) - ((p)->prev_foot) ))

/* extract next chunk's pinuse bit */
#define next_pinuse(p)  ((next_chunk(p)->head) & PINUSE_BIT)

/* Get/set size at footer */
#define get_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot)
#define set_foot(p, s)  (((mchunkptr)((char*)(p) + (s)))->prev_foot = (s))

/* Set size, pinuse bit, and foot */
#define set_size_and_pinuse_of_free_chunk(p, s)\
	((p)->head = (s|PINUSE_BIT), set_foot(p, s))

/* Set size, pinuse bit, foot, and clear next pinuse */
#define set_free_with_pinuse(p, s, n)\
	(clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s))

/* Get the internal overhead associated with chunk p */
#define overhead_for(p) (CHUNK_OVERHEAD)

/* ---------------------- Overlaid data structures ----------------------- */
struct malloc_tree_chunk {
	/* The first four fields must be compatible with malloc_chunk */
	size_t                    prev_foot;
	size_t                    head;
	struct malloc_tree_chunk* fd;
	struct malloc_tree_chunk* bk;

	struct malloc_tree_chunk* child[2];
	struct malloc_tree_chunk* parent;
	bindex_t                  index;
};

typedef struct malloc_tree_chunk  tchunk;
typedef struct malloc_tree_chunk* tchunkptr;
typedef struct malloc_tree_chunk* tbinptr; /* The type of bins of trees */

/* A little helper macro for trees */
#define leftmost_child(t) ((t)->child[0] != 0? (t)->child[0] : (t)->child[1])

/* ---------------------------- malloc_state ----------------------------- */
/* Bin types, widths and sizes */
#define NSMALLBINS        (32U)
#define NTREEBINS         (32U)
#define SMALLBIN_SHIFT    (3U)
#define SMALLBIN_WIDTH    (SIZE_T_ONE << SMALLBIN_SHIFT)
#define TREEBIN_SHIFT     (8U)
#define MIN_LARGE_SIZE    (SIZE_T_ONE << TREEBIN_SHIFT)
#define MAX_SMALL_SIZE    (MIN_LARGE_SIZE - SIZE_T_ONE)
#define MAX_SMALL_REQUEST (MAX_SMALL_SIZE - CHUNK_ALIGN_MASK - CHUNK_OVERHEAD)

#if 0
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
}
#endif

struct malloc_state {
	binmap_t   smallmap;
	binmap_t   treemap;
	size_t     dvsize;
	size_t     topsize;
	char*      least_addr;
	mchunkptr  dv;
	mchunkptr  top;
	size_t     magic;
	mchunkptr  smallbins[(NSMALLBINS+1)*2];
	tbinptr    treebins[NTREEBINS];
};

typedef struct malloc_state*    mstate;

/* ------------- Global malloc_state and malloc_params ------------------- */

/*
	malloc_params holds global properties, including those that can be
	dynamically set using mallopt. There is a single instance, mparams,
	initialized in init_mparams. Note that the non-zeroness of "magic"
	also serves as an initialization flag.
*/

struct malloc_params {
	size_t magic;
};

static struct malloc_params mparams;

/* Ensure mparams initialized */
#define ensure_initialization() (void)(mparams.magic != 0 || init_mparams())

#define is_initialized(M)  ((M)->top != 0)

/* -------------------------------  Hooks -------------------------------- */
/*
	CORRUPTION_ERROR_ACTION is triggered upon detected bad addresses.
	USAGE_ERROR_ACTION is triggered on detected bad frees and
	reallocs. The argument p is an address that might have triggered the
	fault. It is ignored by the two predefined actions, but might be
	useful in custom actions that try to help diagnose errors.
*/

#if PROCEED_ON_ERROR

/* A count of the number of corruption errors causing resets */
int malloc_corruption_error_count;

/* default corruption action */
static void reset_on_error(mstate m);

#define CORRUPTION_ERROR_ACTION(m)  reset_on_error(m)
#define USAGE_ERROR_ACTION(m, p)

#else /* PROCEED_ON_ERROR */

#ifndef CORRUPTION_ERROR_ACTION
#define CORRUPTION_ERROR_ACTION(m) ABORT
#endif /* CORRUPTION_ERROR_ACTION */

#ifndef USAGE_ERROR_ACTION
#define USAGE_ERROR_ACTION(m,p) ABORT
#endif /* USAGE_ERROR_ACTION */

#endif /* PROCEED_ON_ERROR */

#define check_free_chunk(M,P)
#define check_inuse_chunk(M,P)
#define check_malloced_chunk(M,P,N)
#define check_malloc_state(M)
#define check_top_chunk(M,P)

/* ---------------------------- Indexing Bins ---------------------------- */
#define is_small(s)         (((s) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)      (bindex_t)((s)  >> SMALLBIN_SHIFT)
#define small_index2size(i) ((i)  << SMALLBIN_SHIFT)
#define MIN_SMALL_INDEX     (small_index(MIN_CHUNK_SIZE))

/* addressing by index. See above about smallbin repositioning */
#define smallbin_at(M, i)   ((sbinptr)((char*)&((M)->smallbins[(i)<<1])))
#define treebin_at(M,i)     (&((M)->treebins[i]))

/* assign tree index for size S to variable I. Use x86 asm if possible  */
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define compute_tree_index(S, I)\
{\
	unsigned int X = S >> TREEBIN_SHIFT;\
	if (X == 0)\
		I = 0;\
	else if (X > 0xFFFF)\
		I = NTREEBINS-1;\
	else {\
		unsigned int K = (unsigned) sizeof(X)*__CHAR_BIT__ - 1 - (unsigned) __builtin_clz(X); \
		I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
	}\
}

#elif defined (__INTEL_COMPILER)
#define compute_tree_index(S, I)\
{\
	size_t X = S >> TREEBIN_SHIFT;\
	if (X == 0)\
		I = 0;\
	else if (X > 0xFFFF)\
		I = NTREEBINS-1;\
	else {\
		unsigned int K = _bit_scan_reverse (X); \
		I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
	}\
}

#elif defined(_MSC_VER) && _MSC_VER>=1300
#define compute_tree_index(S, I)\
{\
	size_t X = S >> TREEBIN_SHIFT;\
	if (X == 0)\
		I = 0;\
	else if (X > 0xFFFF)\
		I = NTREEBINS-1;\
	else {\
		unsigned int K;\
		_BitScanReverse((unsigned long *) &K, (unsigned long) X);\
		I =  (bindex_t)((K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1)));\
	}\
}

#else /* GNUC */
#define compute_tree_index(S, I)\
{\
	size_t X = S >> TREEBIN_SHIFT;\
	if (X == 0)\
		I = 0;\
	else if (X > 0xFFFF)\
		I = NTREEBINS-1;\
	else {\
		unsigned int Y = (unsigned int)X;\
		unsigned int N = ((Y - 0x100) >> 16) & 8;\
		unsigned int K = (((Y <<= N) - 0x1000) >> 16) & 4;\
		N += K;\
		N += K = (((Y <<= K) - 0x4000) >> 16) & 2;\
		K = 14 - N + ((Y <<= K) >> 15);\
		I = (K << 1) + ((S >> (K + (TREEBIN_SHIFT-1)) & 1));\
	}\
}
#endif /* GNUC */

/* Bit representing maximum resolved size in a treebin at i */
#define bit_for_tree_index(i) \
	 (i == NTREEBINS-1)? (SIZE_T_BITSIZE-1) : (((i) >> 1) + TREEBIN_SHIFT - 2)

/* Shift placing maximum resolved bit in a treebin at i as sign bit */
#define leftshift_for_tree_index(i) \
	 ((i == NTREEBINS-1)? 0 : \
		((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2)))

/* The size of the smallest chunk held in bin with index i */
#define minsize_for_tree_index(i) \
	 ((SIZE_T_ONE << (((i) >> 1) + TREEBIN_SHIFT)) |  \
	 (((size_t)((i) & SIZE_T_ONE)) << (((i) >> 1) + TREEBIN_SHIFT - 1)))


/* ------------------------ Operations on bin maps ----------------------- */

/* bit corresponding to given index */
#define idx2bit(i)              ((binmap_t)(1) << (i))

/* Mark/Clear bits with given index */
#define mark_smallmap(M,i)      ((M)->smallmap |=  idx2bit(i))
#define clear_smallmap(M,i)     ((M)->smallmap &= ~idx2bit(i))
#define smallmap_is_marked(M,i) ((M)->smallmap &   idx2bit(i))

#define mark_treemap(M,i)       ((M)->treemap  |=  idx2bit(i))
#define clear_treemap(M,i)      ((M)->treemap  &= ~idx2bit(i))
#define treemap_is_marked(M,i)  ((M)->treemap  &   idx2bit(i))

/* isolate the least set bit of a bitmap */
#define least_bit(x)         ((x) & -(x))

/* mask with all bits to left of least bit of x on */
#define left_bits(x)         ((x<<1) | -(x<<1))

/* mask with all bits to left of or equal to least bit of x on */
#define same_or_left_bits(x) ((x) | -(x))

/* index corresponding to given bit. Use x86 asm if possible */

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
#define compute_bit2idx(X, I)\
{\
	unsigned int J;\
	J = __builtin_ctz(X); \
	I = (bindex_t)J;\
}

#elif defined (__INTEL_COMPILER)
#define compute_bit2idx(X, I)\
{\
	unsigned int J;\
	J = _bit_scan_forward (X); \
	I = (bindex_t)J;\
}

#elif defined(_MSC_VER) && _MSC_VER>=1300
#define compute_bit2idx(X, I)\
{\
	unsigned int J;\
	_BitScanForward((unsigned long *) &J, X);\
	I = (bindex_t)J;\
}

#elif USE_BUILTIN_FFS
#define compute_bit2idx(X, I) I = ffs(X)-1

#else
#define compute_bit2idx(X, I)\
{\
	unsigned int Y = X - 1;\
	unsigned int K = Y >> (16-4) & 16;\
	unsigned int N = K;        Y >>= K;\
	N += K = Y >> (8-3) &  8;  Y >>= K;\
	N += K = Y >> (4-2) &  4;  Y >>= K;\
	N += K = Y >> (2-1) &  2;  Y >>= K;\
	N += K = Y >> (1-0) &  1;  Y >>= K;\
	I = (bindex_t)(N + Y);\
}
#endif /* GNUC */


/* ----------------------- Runtime Check Support ------------------------- */

/*
	For security, the main invariant is that malloc/free/etc never
	writes to a static address other than malloc_state, unless static
	malloc_state itself has been corrupted, which cannot occur via
	malloc (because of these checks). In essence this means that we
	believe all pointers, sizes, maps etc held in malloc_state, but
	check all of those linked or offsetted from other embedded data
	structures.  These checks are interspersed with main code in a way
	that tends to minimize their run-time cost.

	When FOOTERS is defined, in addition to range checking, we also
	verify footer fields of inuse chunks, which can be used guarantee
	that the mstate controlling malloc/free is intact.  This is a
	streamlined version of the approach described by William Robertson
	et al in "Run-time Detection of Heap-based Overflows" LISA'03
	http://www.usenix.org/events/lisa03/tech/robertson.html The footer
	of an inuse chunk holds the xor of its mstate and a random seed,
	that is checked upon calls to free() and realloc().  This is
	(probabalistically) unguessable from outside the program, but can be
	computed by any code successfully malloc'ing any chunk, so does not
	itself provide protection against code that has already broken
	security through some other means.  Unlike Robertson et al, we
	always dynamically check addresses of all offset chunks (previous,
	next, etc). This turns out to be cheaper than relying on hashes.
*/

#if !INSECURE
/* Check if address a is at least as high as any from MORECORE */
#define ok_address(M, a) ((char*)(a) >= (M)->least_addr)
/* Check if address of next chunk n is higher than base chunk p */
#define ok_next(p, n)    ((char*)(p) < (char*)(n))
/* Check if p has inuse status */
#define ok_inuse(p)     is_inuse(p)
/* Check if p has its pinuse bit on */
#define ok_pinuse(p)     pinuse(p)

#else /* !INSECURE */
#define ok_address(M, a) (1)
#define ok_next(b, n)    (1)
#define ok_inuse(p)      (1)
#define ok_pinuse(p)     (1)
#endif /* !INSECURE */

#if (FOOTERS && !INSECURE)
/* Check if (alleged) mstate m has expected magic field */
#define ok_magic(M)      ((M)->magic == mparams.magic)
#else  /* (FOOTERS && !INSECURE) */
#define ok_magic(M)      (1)
#endif /* (FOOTERS && !INSECURE) */

/* In gcc, use __builtin_expect to minimize impact of checks */
#if !INSECURE
#if defined(__GNUC__) && __GNUC__ >= 3
#define RTCHECK(e)  __builtin_expect(e, 1)
#else /* GNUC */
#define RTCHECK(e)  (e)
#endif /* GNUC */
#else /* !INSECURE */
#define RTCHECK(e)  (1)
#endif /* !INSECURE */

/* macros to set up inuse chunks with or without footers */

#if !FOOTERS

#define mark_inuse_foot(M,p,s)

/* Macros for setting head/foot of chunks */

/* Set cinuse bit and pinuse bit of next chunk */
#define set_inuse(M,p,s)\
	((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
	((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set cinuse and pinuse of this chunk and pinuse of next chunk */
#define set_inuse_and_pinuse(M,p,s)\
	((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
	((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT)

/* Set size, cinuse and pinuse bit of this chunk */
#define set_size_and_pinuse_of_inuse_chunk(M, p, s)\
	((p)->head = (s|PINUSE_BIT|CINUSE_BIT))

#else /* FOOTERS */

/* Set foot of inuse chunk to be xor of mstate and seed */
#define mark_inuse_foot(M,p,s)\
	(((mchunkptr)((char*)(p) + (s)))->prev_foot = ((size_t)(M) ^ mparams.magic))

#define get_mstate_for(p)\
	((mstate)(((mchunkptr)((char*)(p) +\
		(chunksize(p))))->prev_foot ^ mparams.magic))

#define set_inuse(M,p,s)\
	((p)->head = (((p)->head & PINUSE_BIT)|s|CINUSE_BIT),\
	(((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT), \
	mark_inuse_foot(M,p,s))

#define set_inuse_and_pinuse(M,p,s)\
	((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
	(((mchunkptr)(((char*)(p)) + (s)))->head |= PINUSE_BIT),\
 mark_inuse_foot(M,p,s))

#define set_size_and_pinuse_of_inuse_chunk(M, p, s)\
	((p)->head = (s|PINUSE_BIT|CINUSE_BIT),\
	mark_inuse_foot(M, p, s))

#endif /* !FOOTERS */

/* ---------------------------- setting mparams -------------------------- */

/* Initialize mparams */
static int init_mparams(void) {
	if (mparams.magic == 0) {
		size_t magic;

		/* Sanity-check configuration:
			 size_t must be unsigned and as wide as pointer type.
			 ints must be at least 4 bytes.
			 alignment must be at least 8.
			 Alignment, min chunk size, and page size must all be powers of 2.
		*/
		if ((sizeof(size_t) != sizeof(char*)) ||
				(MAX_SIZE_T < MIN_CHUNK_SIZE)  ||
				(sizeof(int) < 4)  ||
				(MALLOC_ALIGNMENT < (size_t)8U) ||
				((MALLOC_ALIGNMENT & (MALLOC_ALIGNMENT-SIZE_T_ONE)) != 0) ||
				((MCHUNK_SIZE      & (MCHUNK_SIZE-SIZE_T_ONE))      != 0))
			ABORT;

		{
			magic = (size_t)&magic ^ (size_t)0x55555555U;
			magic |= (size_t)8U;    /* ensure nonzero */
			magic &= ~(size_t)7U;   /* improve chances of fault for bad values */
			/* Until memory modes commonly available, use volatile-write */
			(*(volatile size_t *)(&(mparams.magic))) = magic;
		}
	}

	return 1;
}

/* ----------------------- Operations on smallbins ----------------------- */

/*
	Various forms of linking and unlinking are defined as macros.  Even
	the ones for trees, which are very long but have very short typical
	paths.  This is ugly but reduces reliance on inlining support of
	compilers.
*/

/* Link a free chunk into a smallbin  */
#define insert_small_chunk(M, P, S) {\
	bindex_t I  = small_index(S);\
	mchunkptr B = smallbin_at(M, I);\
	mchunkptr F = B;\
	assert(S >= MIN_CHUNK_SIZE);\
	if (!smallmap_is_marked(M, I))\
		mark_smallmap(M, I);\
	else if (RTCHECK(ok_address(M, B->fd)))\
		F = B->fd;\
	else {\
		CORRUPTION_ERROR_ACTION(M);\
	}\
	B->fd = P;\
	F->bk = P;\
	P->fd = F;\
	P->bk = B;\
}

/* Unlink a chunk from a smallbin  */
#define unlink_small_chunk(M, P, S) {\
	mchunkptr F = P->fd;\
	mchunkptr B = P->bk;\
	bindex_t I = small_index(S);\
	assert(P != B);\
	assert(P != F);\
	assert(chunksize(P) == small_index2size(I));\
	if (RTCHECK(F == smallbin_at(M,I) || (ok_address(M, F) && F->bk == P))) { \
		if (B == F) {\
			clear_smallmap(M, I);\
		}\
		else if (RTCHECK(B == smallbin_at(M,I) ||\
										 (ok_address(M, B) && B->fd == P))) {\
			F->bk = B;\
			B->fd = F;\
		}\
		else {\
			CORRUPTION_ERROR_ACTION(M);\
		}\
	}\
	else {\
		CORRUPTION_ERROR_ACTION(M);\
	}\
}

/* Unlink the first chunk from a smallbin */
#define unlink_first_small_chunk(M, B, P, I) {\
	mchunkptr F = P->fd;\
	assert(P != B);\
	assert(P != F);\
	assert(chunksize(P) == small_index2size(I));\
	if (B == F) {\
		clear_smallmap(M, I);\
	}\
	else if (RTCHECK(ok_address(M, F) && F->bk == P)) {\
		F->bk = B;\
		B->fd = F;\
	}\
	else {\
		CORRUPTION_ERROR_ACTION(M);\
	}\
}

/* Replace dv node, binning the old one */
/* Used only when dvsize known to be small */
#define replace_dv(M, P, S) {\
	size_t DVS = M->dvsize;\
	assert(is_small(DVS));\
	if (DVS != 0) {\
		mchunkptr DV = M->dv;\
		insert_small_chunk(M, DV, DVS);\
	}\
	M->dvsize = S;\
	M->dv = P;\
}

/* ------------------------- Operations on trees ------------------------- */

/* Insert chunk into tree */
#define insert_large_chunk(M, X, S) {\
	tbinptr* H;\
	bindex_t I;\
	compute_tree_index(S, I);\
	H = treebin_at(M, I);\
	X->index = I;\
	X->child[0] = X->child[1] = 0;\
	if (!treemap_is_marked(M, I)) {\
		mark_treemap(M, I);\
		*H = X;\
		X->parent = (tchunkptr)H;\
		X->fd = X->bk = X;\
	}\
	else {\
		tchunkptr T = *H;\
		size_t K = S << leftshift_for_tree_index(I);\
		for (;;) {\
			if (chunksize(T) != S) {\
				tchunkptr* C = &(T->child[(K >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1]);\
				K <<= 1;\
				if (*C != 0)\
					T = *C;\
				else if (RTCHECK(ok_address(M, C))) {\
					*C = X;\
					X->parent = T;\
					X->fd = X->bk = X;\
					break;\
				}\
				else {\
					CORRUPTION_ERROR_ACTION(M);\
					break;\
				}\
			}\
			else {\
				tchunkptr F = T->fd;\
				if (RTCHECK(ok_address(M, T) && ok_address(M, F))) {\
					T->fd = F->bk = X;\
					X->fd = F;\
					X->bk = T;\
					X->parent = 0;\
					break;\
				}\
				else {\
					CORRUPTION_ERROR_ACTION(M);\
					break;\
				}\
			}\
		}\
	}\
}

/*
	Unlink steps:

	1. If x is a chained node, unlink it from its same-sized fd/bk links
		 and choose its bk node as its replacement.
	2. If x was the last node of its size, but not a leaf node, it must
		 be replaced with a leaf node (not merely one with an open left or
		 right), to make sure that lefts and rights of descendents
		 correspond properly to bit masks.  We use the rightmost descendent
		 of x.  We could use any other leaf, but this is easy to locate and
		 tends to counteract removal of leftmosts elsewhere, and so keeps
		 paths shorter than minimally guaranteed.  This doesn't loop much
		 because on average a node in a tree is near the bottom.
	3. If x is the base of a chain (i.e., has parent links) relink
		 x's parent and children to x's replacement (or null if none).
*/

#define unlink_large_chunk(M, X) {\
	tchunkptr XP = X->parent;\
	tchunkptr R;\
	if (X->bk != X) {\
		tchunkptr F = X->fd;\
		R = X->bk;\
		if (RTCHECK(ok_address(M, F) && F->bk == X && R->fd == X)) {\
			F->bk = R;\
			R->fd = F;\
		}\
		else {\
			CORRUPTION_ERROR_ACTION(M);\
		}\
	}\
	else {\
		tchunkptr* RP;\
		if (((R = *(RP = &(X->child[1]))) != 0) ||\
				((R = *(RP = &(X->child[0]))) != 0)) {\
			tchunkptr* CP;\
			while ((*(CP = &(R->child[1])) != 0) ||\
						 (*(CP = &(R->child[0])) != 0)) {\
				R = *(RP = CP);\
			}\
			if (RTCHECK(ok_address(M, RP)))\
				*RP = 0;\
			else {\
				CORRUPTION_ERROR_ACTION(M);\
			}\
		}\
	}\
	if (XP != 0) {\
		tbinptr* H = treebin_at(M, X->index);\
		if (X == *H) {\
			if ((*H = R) == 0) \
				clear_treemap(M, X->index);\
		}\
		else if (RTCHECK(ok_address(M, XP))) {\
			if (XP->child[0] == X) \
				XP->child[0] = R;\
			else \
				XP->child[1] = R;\
		}\
		else\
			CORRUPTION_ERROR_ACTION(M);\
		if (R != 0) {\
			if (RTCHECK(ok_address(M, R))) {\
				tchunkptr C0, C1;\
				R->parent = XP;\
				if ((C0 = X->child[0]) != 0) {\
					if (RTCHECK(ok_address(M, C0))) {\
						R->child[0] = C0;\
						C0->parent = R;\
					}\
					else\
						CORRUPTION_ERROR_ACTION(M);\
				}\
				if ((C1 = X->child[1]) != 0) {\
					if (RTCHECK(ok_address(M, C1))) {\
						R->child[1] = C1;\
						C1->parent = R;\
					}\
					else\
						CORRUPTION_ERROR_ACTION(M);\
				}\
			}\
			else\
				CORRUPTION_ERROR_ACTION(M);\
		}\
	}\
}

/* Relays to large vs small bin operations */

#define insert_chunk(M, P, S)\
	if (is_small(S)) insert_small_chunk(M, P, S)\
	else { tchunkptr TP = (tchunkptr)(P); insert_large_chunk(M, TP, S); }

#define unlink_chunk(M, P, S)\
	if (is_small(S)) unlink_small_chunk(M, P, S)\
	else { tchunkptr TP = (tchunkptr)(P); unlink_large_chunk(M, TP); }


/* Relays to internal calls to malloc/free from realloc, memalign etc */

#define internal_malloc(m, b) mspace_malloc(m, b)
#define internal_free(m, mem) mspace_free(m,mem);

/* -------------------------- mspace management -------------------------- */

/* Initialize top chunk and its size */
static void init_top(mstate m, mchunkptr p, size_t psize) {
	/* Ensure alignment */
	size_t offset = align_offset(chunk2mem(p));
	p = (mchunkptr)((char*)p + offset);
	psize -= offset;

	m->top = p;
	m->topsize = psize;
	p->head = psize | PINUSE_BIT;
}

/* Initialize bins for a new mstate that is otherwise zeroed out */
static void init_bins(mstate m) {
	/* Establish circular links for smallbins */
	bindex_t i;
	for (i = 0; i < NSMALLBINS; ++i) {
		sbinptr bin = smallbin_at(m,i);
		bin->fd = bin->bk = bin;
	}
}

/* ---------------------------- malloc --------------------------- */

/* allocate a large request from the best fitting chunk in a treebin */
static void* tmalloc_large(mstate m, size_t nb) {
	tchunkptr v = 0;
	size_t rsize = -nb; /* Unsigned negation */
	tchunkptr t;
	bindex_t idx;
	compute_tree_index(nb, idx);
	if ((t = *treebin_at(m, idx)) != 0) {
		/* Traverse tree for this bin looking for node with size == nb */
		size_t sizebits = nb << leftshift_for_tree_index(idx);
		tchunkptr rst = 0;  /* The deepest untaken right subtree */
		for (;;) {
			tchunkptr rt;
			size_t trem = chunksize(t) - nb;
			if (trem < rsize) {
				v = t;
				if ((rsize = trem) == 0)
					break;
			}
			rt = t->child[1];
			t = t->child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
			if (rt != 0 && rt != t)
				rst = rt;
			if (t == 0) {
				t = rst; /* set t to least subtree holding sizes > nb */
				break;
			}
			sizebits <<= 1;
		}
	}
	if (t == 0 && v == 0) { /* set t to root of next non-empty treebin */
		binmap_t leftbits = left_bits(idx2bit(idx)) & m->treemap;
		if (leftbits != 0) {
			bindex_t i;
			binmap_t leastbit = least_bit(leftbits);
			compute_bit2idx(leastbit, i);
			t = *treebin_at(m, i);
		}
	}

	while (t != 0) { /* find smallest of tree or subtree */
		size_t trem = chunksize(t) - nb;
		if (trem < rsize) {
			rsize = trem;
			v = t;
		}
		t = leftmost_child(t);
	}

	/*  If dv is a better fit, return 0 so malloc will use it */
	if (v != 0 && rsize < (size_t)(m->dvsize - nb)) {
		if (RTCHECK(ok_address(m, v))) { /* split */
			mchunkptr r = chunk_plus_offset(v, nb);
			assert(chunksize(v) == rsize + nb);
			if (RTCHECK(ok_next(v, r))) {
				unlink_large_chunk(m, v);
				if (rsize < MIN_CHUNK_SIZE)
					set_inuse_and_pinuse(m, v, (rsize + nb));
				else {
					set_size_and_pinuse_of_inuse_chunk(m, v, nb);
					set_size_and_pinuse_of_free_chunk(r, rsize);
					insert_chunk(m, r, rsize);
				}
				return chunk2mem(v);
			}
		}
		CORRUPTION_ERROR_ACTION(m);
	}
	return 0;
}

/* allocate a small request from the best fitting chunk in a treebin */
static void* tmalloc_small(mstate m, size_t nb) {
	tchunkptr t, v;
	size_t rsize;
	bindex_t i;
	binmap_t leastbit = least_bit(m->treemap);
	compute_bit2idx(leastbit, i);
	v = t = *treebin_at(m, i);
	rsize = chunksize(t) - nb;

	while ((t = leftmost_child(t)) != 0) {
		size_t trem = chunksize(t) - nb;
		if (trem < rsize) {
			rsize = trem;
			v = t;
		}
	}

	if (RTCHECK(ok_address(m, v))) {
		mchunkptr r = chunk_plus_offset(v, nb);
		assert(chunksize(v) == rsize + nb);
		if (RTCHECK(ok_next(v, r))) {
			unlink_large_chunk(m, v);
			if (rsize < MIN_CHUNK_SIZE)
				set_inuse_and_pinuse(m, v, (rsize + nb));
			else {
				set_size_and_pinuse_of_inuse_chunk(m, v, nb);
				set_size_and_pinuse_of_free_chunk(r, rsize);
				replace_dv(m, r, rsize);
			}
			return chunk2mem(v);
		}
	}

	CORRUPTION_ERROR_ACTION(m);
	// Unreachable code
	// return 0;
}

static mstate init_user_mstate(char* tbase, size_t tsize) {
	size_t msize = pad_request(sizeof(struct malloc_state));
	mchunkptr mn;
	mchunkptr msp = align_as_chunk(tbase);
	mstate m = (mstate)(chunk2mem(msp));
	memset(m, 0, msize);
	msp->head = (msize|INUSE_BITS);
	m->least_addr = tbase;
	m->magic = mparams.magic;
	init_bins(m);
	mn = next_chunk(mem2chunk(m));
	init_top(m, mn, (size_t)((tbase + tsize) - (char*)mn));
	check_top_chunk(m, m->top);
	return m;
}
static mspace create_mspace_with_base(void* base, size_t capacity) {
	mstate m = 0;
	size_t msize;
	ensure_initialization();
	msize = pad_request(sizeof(struct malloc_state));
	if (capacity > msize) {
		m = init_user_mstate((char*)base, capacity);
		// m->seg.sflags = EXTERN_BIT;
	}
	return (mspace)m;
}

static void* mspace_malloc(mspace msp, size_t bytes) {
	mstate ms = (mstate)msp;
	if (!ok_magic(ms)) {
		USAGE_ERROR_ACTION(ms,ms);
		// Unreachable code
		// return 0;
	}
	{
		void* mem;
		size_t nb;
		if (bytes <= MAX_SMALL_REQUEST) {
			bindex_t idx;
			binmap_t smallbits;
			nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
			idx = small_index(nb);
			smallbits = ms->smallmap >> idx;

			if ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
				mchunkptr b, p;
				idx += ~smallbits & 1;       /* Uses next bin if idx empty */
				b = smallbin_at(ms, idx);
				p = b->fd;
				assert(chunksize(p) == small_index2size(idx));
				unlink_first_small_chunk(ms, b, p, idx);
				set_inuse_and_pinuse(ms, p, small_index2size(idx));
				mem = chunk2mem(p);
				check_malloced_chunk(ms, mem, nb);
				goto postaction;
			}

			else if (nb > ms->dvsize) {
				if (smallbits != 0) { /* Use chunk in next nonempty smallbin */
					mchunkptr b, p, r;
					size_t rsize;
					bindex_t i;
					binmap_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
					binmap_t leastbit = least_bit(leftbits);
					compute_bit2idx(leastbit, i);
					b = smallbin_at(ms, i);
					p = b->fd;
					assert(chunksize(p) == small_index2size(i));
					unlink_first_small_chunk(ms, b, p, i);
					rsize = small_index2size(i) - nb;
					/* Fit here cannot be remainderless if 4byte sizes */
					if (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
						set_inuse_and_pinuse(ms, p, small_index2size(i));
					else {
						set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
						r = chunk_plus_offset(p, nb);
						set_size_and_pinuse_of_free_chunk(r, rsize);
						replace_dv(ms, r, rsize);
					}
					mem = chunk2mem(p);
					check_malloced_chunk(ms, mem, nb);
					goto postaction;
				}

				else if (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
					check_malloced_chunk(ms, mem, nb);
					goto postaction;
				}
			}
		}
		else if (bytes >= MAX_REQUEST)
			nb = MAX_SIZE_T; /* Too big to allocate. Force failure (in sys alloc) */
		else {
			nb = pad_request(bytes);
			if (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
				check_malloced_chunk(ms, mem, nb);
				goto postaction;
			}
		}

		if (nb <= ms->dvsize) {
			size_t rsize = ms->dvsize - nb;
			mchunkptr p = ms->dv;
			if (rsize >= MIN_CHUNK_SIZE) { /* split dv */
				mchunkptr r = ms->dv = chunk_plus_offset(p, nb);
				ms->dvsize = rsize;
				set_size_and_pinuse_of_free_chunk(r, rsize);
				set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
			}
			else { /* exhaust dv */
				size_t dvs = ms->dvsize;
				ms->dvsize = 0;
				ms->dv = 0;
				set_inuse_and_pinuse(ms, p, dvs);
			}
			mem = chunk2mem(p);
			check_malloced_chunk(ms, mem, nb);
			goto postaction;
		}

		else if (nb < ms->topsize) { /* Split top */
			size_t rsize = ms->topsize -= nb;
			mchunkptr p = ms->top;
			mchunkptr r = ms->top = chunk_plus_offset(p, nb);
			r->head = rsize | PINUSE_BIT;
			set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
			mem = chunk2mem(p);
			check_top_chunk(ms, ms->top);
			check_malloced_chunk(ms, mem, nb);
			goto postaction;
		}

		// No more memory!
		assert(false);
		mem = 0;

	postaction:
		return mem;
	}

	// Unreachable code
	// return 0;
}

static void mspace_free(mspace msp, void* mem) {
	if (mem != 0) {
		mchunkptr p  = mem2chunk(mem);
#if FOOTERS
		mstate fm = get_mstate_for(p);
		(void)msp; /* placate people compiling -Wunused */
#else /* FOOTERS */
		mstate fm = (mstate)msp;
#endif /* FOOTERS */
		if (!ok_magic(fm)) {
			USAGE_ERROR_ACTION(fm, p);
			return;
		}
		{
			check_inuse_chunk(fm, p);
			if (RTCHECK(ok_address(fm, p) && ok_inuse(p))) {
				size_t psize = chunksize(p);
				mchunkptr next = chunk_plus_offset(p, psize);
				if (!pinuse(p)) {
					size_t prevsize = p->prev_foot;

					mchunkptr prev = chunk_minus_offset(p, prevsize);
					psize += prevsize;
					p = prev;
					if (RTCHECK(ok_address(fm, prev))) { /* consolidate backward */
						if (p != fm->dv) {
							unlink_chunk(fm, p, prevsize);
						}
						else if ((next->head & INUSE_BITS) == INUSE_BITS) {
							fm->dvsize = psize;
							set_free_with_pinuse(p, psize, next);
							goto postaction;
						}
					}
					else
						goto erroraction;
				}

				if (RTCHECK(ok_next(p, next) && ok_pinuse(next))) {
					if (!cinuse(next)) {  /* consolidate forward */
						if (next == fm->top) {
							size_t tsize = fm->topsize += psize;
							fm->top = p;
							p->head = tsize | PINUSE_BIT;
							if (p == fm->dv) {
								fm->dv = 0;
								fm->dvsize = 0;
							}
							/*if (should_trim(fm, tsize))
								sys_trim(fm, 0);*/
							goto postaction;
						}
						else if (next == fm->dv) {
							size_t dsize = fm->dvsize += psize;
							fm->dv = p;
							set_size_and_pinuse_of_free_chunk(p, dsize);
							goto postaction;
						}
						else {
							size_t nsize = chunksize(next);
							psize += nsize;
							unlink_chunk(fm, next, nsize);
							set_size_and_pinuse_of_free_chunk(p, psize);
							if (p == fm->dv) {
								fm->dvsize = psize;
								goto postaction;
							}
						}
					}
					else
						set_free_with_pinuse(p, psize, next);

					if (is_small(psize)) {
						insert_small_chunk(fm, p, psize);
						check_free_chunk(fm, p);
					}
					else {
						tchunkptr tp = (tchunkptr)p;
						insert_large_chunk(fm, tp, psize);
						check_free_chunk(fm, p);
					}
					goto postaction;
				}
			}
		erroraction:
			USAGE_ERROR_ACTION(fm, p);
		postaction:
			;
		}
	}
}