#ifndef _RSTL_VERSIONS_H
#define _RSTL_VERSIONS_H

// Prime 1 GC
#define RSTL_GM8EAB_00 0
#define RSTL_GM8E_00 1
#define RSTL_GM8E_01 2
#define RSTL_GM8E_48 3
#define RSTL_GM8E_02 4
#define RSTL_GM8P_00 5
#define RSTL_GM8J_DD 6
#define RSTL_GM8J_00 7

// Prime 2 GC
#define RSTL_P2ME 10 // v0.160 Echoes Bonus Disc
#define RSTL_D75E 11 // Multi-Game Demo Disc v20
#define RSTL_D84P 12 // EUR Interactive Multi-Game Demo Disc: November 2004
#define RSTL_G2ME 13
#define RSTL_G2MP 14
#define RSTL_G2MJ 15

// Prime 3 Standalone
#define RSTL_G2MEAB 20  // E3 2006 Prototype
#define RSTL_RM3E   21
#define RSTL_RM3P   22
#define RSTL_RM3J   23

// Wii de Asobu
#define RSTL_R3IJ 30
#define RSTL_R32J 31

// Trilogy
#define RSTL_R3ME_00 40
#define RSTL_R3MP_00 41

// Feature macros. Each is 1 when the selected RSTL_VERSION has the behavior and 0 otherwise; the
// sources test these instead of comparing RSTL_VERSION. The evidence for each boundary is in the
// research pages linked from Prime's shared-rstl.md.

// ============================================================================================
// Games and eras
// ============================================================================================

// Metroid Prime, including Wii de Asobu Prime and Prime Trilogy (told apart by the game's PRIME1).
#if defined(PRIME1) || RSTL_VERSION < RSTL_P2ME || RSTL_VERSION == RSTL_R3IJ
#define RSTL_PRIME1 1
#else
#define RSTL_PRIME1 0
#endif
// The GameCube Echoes demo discs.
#define RSTL_ECHOES_DEMO (RSTL_VERSION >= RSTL_P2ME && RSTL_VERSION <= RSTL_D84P)
// Every GameCube Echoes build, demos included.
#define RSTL_ECHOES_GC (RSTL_VERSION >= RSTL_P2ME && RSTL_VERSION <= RSTL_G2MJ)
// The Echoes-era library: GameCube Echoes and Corruption (prototype and retail).
#define RSTL_ECHOES (RSTL_VERSION >= RSTL_P2ME && RSTL_VERSION < RSTL_R3IJ)
// The Corruption prototype and retail. Retail (no build) provisionally takes the prototype forms;
// record a retail difference by excluding RSTL_RM3E/P/J from the macro concerned.
#define RSTL_CORRUPTION (RSTL_VERSION >= RSTL_G2MEAB && RSTL_VERSION <= RSTL_RM3J)
// Trilogy as built for Echoes, whose fill and `reserved_vector` details are Echoes-era.
#define RSTL_TRILOGY_ECHOES (RSTL_VERSION >= RSTL_R3ME_00 && !RSTL_PRIME1)

// ============================================================================================
// rstl: all containers
// ============================================================================================

// Containers and strings derive privately from `Alloc` instead of holding an `Alloc mAllocator`.
#define RSTL_ALLOCATOR_BASE_CLASS (RSTL_VERSION >= RSTL_RM3E)
// Containers allocate through the allocator instance instead of static `Alloc::` calls.
#define RSTL_ALLOCATOR_INSTANCE RSTL_ECHOES
// Stream constructors read element counts with `ReadInt32` instead of `Get(TGetType(0))`.
#define RSTL_STREAM_READ_INT32 (!RSTL_PRIME1)
// Preconditions are the empty `RSTL_PRECONDITION` statements instead of `RS_ASSERT` or no-op stubs.
#define RSTL_PRECONDITION_MACRO (RSTL_VERSION >= RSTL_P2ME)
// `vector::operator[]` and non-const `reserved_vector::operator[]` carry the index precondition.
#define RSTL_INDEX_PRECONDITION (RSTL_ECHOES && !RSTL_CORRUPTION)

// ============================================================================================
// rstl: allocators (rmemory_allocator, locked_cache_allocator, RstlExtras)
// ============================================================================================

// `rmemory_allocator::allocate(int)` is declared only (defined in RstlExtras.cpp), `deallocate`
// skips the null check, and `allocate2` exists.
#define RSTL_ALLOCATOR_EXTERN_ALLOCATE (RSTL_VERSION >= RSTL_P2ME)
// Wii Echoes exports the allocation entry point as `global_allocate`.
#define RSTL_ALLOCATOR_GLOBAL_ALLOCATE (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J)
// Allocation calls the out-of-line `rmemory_allocator::allocate(int)` (MWCC only).
#define RSTL_OUTLINED_ALLOCATE (RSTL_VERSION >= RSTL_GM8P_00)
// rstl_allocator.cpp defines `rmemory_allocator::allocate(int)` (GM8E_02 defines it unused).
#define RSTL_EMIT_OUTLINED_ALLOCATE (RSTL_VERSION >= RSTL_GM8E_02)
// The prototype's allocation expression, without Echoes' size local.
#define RSTL_ALLOCATOR_LEGACY_SPELLING (RSTL_CORRUPTION)
// `aligned_allocator::allocate(T*&, int)` calls `CMemory::Alloc` directly; no `allocate(int)`.
#define RSTL_ALIGNED_ALLOCATE_INLINE (RSTL_PRIME1 || RSTL_CORRUPTION)
// `locked_cache_allocator` takes a plain `int` mode instead of `EMode`.
#define RSTL_LOCKED_CACHE_RAW_MODE (RSTL_CORRUPTION)

// ============================================================================================
// rstl: construct.hpp and type_traits.hpp
// ============================================================================================

// `construct` and `destroy(T*)` go through `construction_policy` / `destruction_policy`.
#define RSTL_CONSTRUCTION_POLICY RSTL_ECHOES
// `construct`, `destroy` and `destroy_impl` are external `inline` instead of `static inline`.
#define RSTL_EXTERN_CONSTRUCT_FUNCTIONS RSTL_ECHOES
// `construct` also takes a mutable `T&`, and the placement-new policy has no null precondition.
#define RSTL_CONSTRUCT_MUTABLE_OVERLOAD                                                            \
  (RSTL_CORRUPTION || (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J))
// A non-inline declaration before the const `construct` gives its instantiations weak linkage.
#define RSTL_CONSTRUCT_EXTERN_DECL (RSTL_CORRUPTION)
// Bitwise construction copies a trailing word inline; the Echoes demo discs `memcpy` instead.
#define RSTL_BITWISE_CONSTRUCTION_TAIL_WORD (!RSTL_ECHOES_DEMO)
// `construct` extends its direct `int` overload to `uint`, `bool`, `float` and pointers, and
// `reserved_vector_traits` has scalar fill/push fast paths.
#define RSTL_SCALAR_TRAITS (RSTL_VERSION >= RSTL_R32J)
// Arithmetic types and pointers are trivially destructible and copied by assignment.
#define RSTL_TRIVIAL_SCALARS RSTL_ECHOES
// Pointer trivial-destructor traits (the prototype gets them from RSTL_SCALAR_TYPE_TRAITS).
#define RSTL_TRIVIAL_POINTERS                                                                      \
  ((RSTL_ECHOES && !RSTL_CORRUPTION) || RSTL_VERSION >= RSTL_R32J)
// `uint` destruction is trivial; kept apart from the pointer traits it currently follows.
#define RSTL_TRIVIAL_UINT_DESTRUCTION RSTL_TRIVIAL_POINTERS
// `float` destruction is trivial (every build except Prime).
#define RSTL_TRIVIAL_FLOAT_DESTRUCTION (!RSTL_PRIME1)
// `int`/`bool`/`ushort` destruction traits for non-Prime builds without RSTL_TRIVIAL_SCALARS.
#define RSTL_TRIVIAL_INT_BOOL_DESTRUCTION (!RSTL_PRIME1 && !RSTL_TRIVIAL_SCALARS)
// The pointer specializations follow the primary traits, `is_scalar_type` exists, and `uint`,
// `u64` and `s64` are trivially constructible.
#define RSTL_SCALAR_TYPE_TRAITS (RSTL_CORRUPTION)
// `range_destruction_checks` and the destruction loop that consults it exist.
#define RSTL_RANGE_DESTRUCTION_CHECKS (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// The direct `destroy(T*)` takes a `T* const`.
#define RSTL_DESTROY_CONST_PARAM (!RSTL_ECHOES)
// The direct `destroy(T*)` carries the (empty) null precondition.
#define RSTL_DESTROY_PRECONDITION (RSTL_VERSION < RSTL_GM8P_00)
// `destroy(T*)` and `destroy(begin, end)` forward to `destroy_impl`.
#define RSTL_DESTROY_VIA_IMPL (RSTL_CORRUPTION)
// A non-inline declaration gives the single-element `destroy_impl` helpers weak linkage.
#define RSTL_DESTROY_IMPL_EXTERN_DECL (RSTL_VERSION == RSTL_G2MEAB)
// `destroy(begin, end)` over iterators tests `begin != end` before looping.
#define RSTL_DESTROY_RANGE_GUARD (RSTL_VERSION >= RSTL_R3IJ)
// `destroy(T*, T*)` is a separate overload over pointer ranges.
#define RSTL_POINTER_RANGE_OVERLOADS (RSTL_ECHOES || RSTL_VERSION >= RSTL_R3IJ)
// Range algorithms check `begin <= end` and `n >= 0`, and `uninitialized_copy` has an unchecked
// pointer overload.
#define RSTL_RANGE_PRECONDITIONS RSTL_ECHOES
// The pointer overload of `uninitialized_copy` also checks `begin <= end`.
#define RSTL_POINTER_COPY_PRECONDITION (RSTL_CORRUPTION)
// Generic range copies advance the output parameter directly.
#define RSTL_COPY_RANGE_OUTPUT_PARAM (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J)
// `uninitialized_copy` steps `++cur, ++tmp` and `uninitialized_copy_n` pre-increments.
#define RSTL_PREINCREMENT_COPY_LOOPS (RSTL_ECHOES || RSTL_VERSION >= RSTL_R3IJ)
// `uninitialized_copy_n` counts down a `remaining` variable.
#define RSTL_COUNTED_COPY_N_LOOP (RSTL_ECHOES || RSTL_VERSION >= RSTL_R32J)
// ...and declares that counter before the iterator copies.
#define RSTL_COPY_N_COUNTER_FIRST (RSTL_VERSION >= RSTL_R3ME_00)
// `uninitialized_copy_n` starts with the `n >= 0` precondition.
#define RSTL_COPY_N_PRECONDITION (RSTL_CORRUPTION)
// Counted copies read through a const source (needs mutable overloads with the extern declaration).
#define RSTL_COPY_N_CONST_SOURCE (!RSTL_CONSTRUCT_EXTERN_DECL)
// `uninitialized_fill_n` fills through `construct` instead of a placement new.
#define RSTL_FILL_USES_CONSTRUCT RSTL_ECHOES
// `uninitialized_fill_n` has a pointer overload that assigns the elements.
#define RSTL_POINTER_FILL_OVERLOAD (RSTL_VERSION >= RSTL_R3IJ)
// `uninitialized_fill_n` takes builtin values by value and assigns them.
#define RSTL_SCALAR_FILL_HELPERS (RSTL_VERSION >= RSTL_R3IJ)
// The prototype passes builtin fill values by value only for `signed char`.
#define RSTL_BYTE_FILL_VALUE (RSTL_CORRUPTION)
// `uninitialized_fill_n` advances `dest` itself, and `bit_vector::insert` uses counted loops.
#define RSTL_COUNTED_FILL_LOOPS (RSTL_VERSION >= RSTL_R3ME_00)
// Container fills, `resize` and `assign` take builtin values and pointers by value.
#define RSTL_SCALAR_FILL_BY_VALUE (RSTL_VERSION >= RSTL_G2MEAB)
// Single-element vector insertion takes builtin values and pointers by value.
#define RSTL_VECTOR_INSERT_SCALAR_BY_VALUE (RSTL_CORRUPTION)

// ============================================================================================
// rstl: pair, auto_ptr, single_ptr, rc_ptr, ownership_transfer, copy_object
// ============================================================================================

// Pairs (except of auto_ptr or rc_ptr) are copied without the policy's precondition statement.
#define RSTL_PAIR_CONSTRUCTION_POLICY (RSTL_CONSTRUCTION_POLICY && !RSTL_CONSTRUCT_MUTABLE_OVERLOAD)
// `pair< int, float >` is explicitly kept non-trivial for the construction policies.
#define RSTL_PAIR_INT_FLOAT_POLICY (RSTL_CORRUPTION)
// `pair` also accepts a mutable second argument by reference.
#define RSTL_PAIR_MUTABLE_SECOND (RSTL_CORRUPTION)
// The `auto_ptr` pointer constructor defaults to null.
#define RSTL_AUTO_PTR_CONSTRUCT_NULL (RSTL_CORRUPTION)
// `auto_ptr` converts from `auto_ptr< U >` (the GameCube compiler rejects its template friend).
#define RSTL_AUTO_PTR_CONVERSION (RSTL_VERSION >= RSTL_R3IJ)
// `auto_ptr::release` returns null when it does not own the item.
#define RSTL_AUTO_PTR_CHECKED_RELEASE (RSTL_VERSION >= RSTL_G2MEAB)
// `single_ptr` has a const-ref copy constructor that transfers ownership.
#define RSTL_SINGLE_PTR_TRANSFER (RSTL_VERSION >= RSTL_P2ME)
// ...through a `mutable` pointer (Prime and the prototype).
#define RSTL_SINGLE_PTR_MUTABLE ((RSTL_PRIME1 || RSTL_CORRUPTION) && RSTL_SINGLE_PTR_TRANSFER)
// `~single_ptr` deletes through `get()` instead of `mPtr`.
#define RSTL_SINGLE_PTR_DELETE_VIA_GET (RSTL_VERSION >= RSTL_R3IJ)
// The pointer constructor has a default argument (also used by array constructor thunks).
#define RSTL_SINGLE_PTR_DEFAULT_ARGUMENT (RSTL_CORRUPTION)
// `single_ptr::operator->` returns a plain `T*` and const `operator*` returns `const T&`.
#define RSTL_SINGLE_PTR_PLAIN_ACCESS (RSTL_CORRUPTION)
// `rc_ptr` is the standalone implementation counting through `rc_ptr_private::sNull`.
#define RSTL_RC_PTR_PRIVATE_COUNT RSTL_ECHOES
// `rc_ptr` keeps `const T* mPtr` and an `int* mCount`, and has converting copies and `Cast<U>()`.
#define RSTL_RC_PTR_INLINE_COUNT (RSTL_VERSION >= RSTL_R3IJ)
// `~rc_ptr` carries the (empty) count precondition.
#define RSTL_RC_PTR_DTOR_PRECONDITION (!RSTL_CORRUPTION)
// `rc_ptr` accessors go through `GetPtr()` and `ReleaseData` deletes `mRefCount` directly.
#define RSTL_RC_PTR_GET_PTR_ACCESS (RSTL_PRIME1 || RSTL_CORRUPTION)
// `ncrc_ptr` has an explicit copy constructor with preconditions.
#define RSTL_NCRC_PTR_COPY_CTOR (RSTL_RC_PTR_PRIVATE_COUNT && !RSTL_CORRUPTION)
// `ownership_transfer::take_ownership` returns null when it does not own the object.
#define RSTL_OWNERSHIP_TRANSFER_CHECKED_TAKE                                                       \
  (RSTL_CORRUPTION || RSTL_VERSION >= RSTL_R3IJ)
// The copy owner allocates and clones in a separate member helper.
#define RSTL_COPY_OBJECT_CLONE_HELPER (RSTL_CORRUPTION)

// ============================================================================================
// rstl: iterators, math, functional and algorithm
// ============================================================================================

// `distance`, `advance` and their tag-dispatch helpers are `inline`.
#define RSTL_INLINE_ITERATOR_HELPERS (RSTL_VERSION >= RSTL_P2ME)
// `advance_iterator` is `inline`.
#define RSTL_INLINE_ADVANCE_ITERATOR                                                            \
  (RSTL_VERSION >= RSTL_P2ME && (!RSTL_CORRUPTION || RSTL_VERSION == RSTL_G2MEAB))
// `pointer_iterator` `operator+` / `operator-` copy the iterator and use `+=` / `-=`.
#define RSTL_POINTER_ITERATOR_COPY_ARITHMETIC (RSTL_VERSION >= RSTL_R3ME_00)
// `const_linear_iterator` builds `+`/`-` results directly, and `operator-` returns `long`.
#define RSTL_LINEAR_ITERATOR_DIRECT_ARITHMETIC (RSTL_CORRUPTION)
// `min_val` / `max_val` exist and take their arguments by value.
#define RSTL_MIN_MAX_VAL (RSTL_VERSION >= RSTL_P2ME)
// `less` declares its own typedefs instead of deriving `binary_function`; `less< uint >` exists.
#define RSTL_LESS_WITHOUT_BASE (RSTL_VERSION >= RSTL_R3ME_00)
// `__insertion_sort` takes a non-const `first` and the partition loop is `for (;;)`.
#define RSTL_SORT_ECHOES_LOOPS (RSTL_VERSION >= RSTL_P2ME)
// `__insertion_sort` computes `t1 = next - 1` (Wii steps `t1 = next; --t1;`).
#define RSTL_INSERTION_SORT_NEXT_MINUS_ONE RSTL_ECHOES
// The comparison sorts keep the comparison result in a bool local.
#define RSTL_SORT_LESS_LOCAL (!RSTL_CORRUPTION)
// `lower_bound` advances `start` itself with a loop-local iterator.
#define RSTL_LOWER_BOUND_ADVANCES_START (RSTL_VERSION >= RSTL_P2ME)
// `lower_bound` declares `halfDist` before its loop.
#define RSTL_LOWER_BOUND_HOISTED_HALF_DIST (!RSTL_PRIME1)
// The plain `lower_bound` declares `halfDist` and `it` inside its loop.
#define RSTL_LOWER_BOUND_LOOP_LOCALS (RSTL_CORRUPTION)
// The comparator `lower_bound` follows the comparator `sort`, before `__sort3`.
#define RSTL_LOWER_BOUND_CMP_BEFORE_SORT3 (RSTL_CORRUPTION)
// `binary_find` and `equal_range` keep named `lower` / `found` locals.
#define RSTL_NAMED_SEARCH_LOCALS (RSTL_VERSION >= RSTL_R3IJ)
// `find_if` takes its predicate by value.
#define RSTL_FIND_IF_PRED_BY_VALUE (RSTL_CORRUPTION)
// `swap` / `iter_swap` are inline only under `RSTL_INLINE_SWAP`, and `swap` has no precondition.
#define RSTL_SWAP_OPTIONAL_INLINE (RSTL_CORRUPTION)

// ============================================================================================
// rstl: vector
// ============================================================================================

// Members are declared in the class and defined outside it.
#define RSTL_VECTOR_OUT_OF_CLASS (RSTL_VERSION >= RSTL_P2ME)
// `assign` keeps its inline declaration.
#define RSTL_VECTOR_INLINE_DECLS (!RSTL_CORRUPTION)
// `begin` builds the iterator from `mItems` instead of `data()`.
#define RSTL_VECTOR_ITEM_BEGIN (RSTL_VERSION >= RSTL_P2ME)
// `end` and `insert_into` bind their pointers to `T* const` locals.
#define RSTL_VECTOR_CONST_LOCALS (RSTL_VERSION >= RSTL_GM8P_00)
// `insert_into` destroys the moved-from element through a `T* const item` local.
#define RSTL_VECTOR_INSERT_CONST_ITEM (RSTL_VERSION < RSTL_P2ME)
// `insert_into` indexes the leading elements with a `long`.
#define RSTL_VECTOR_LONG_INSERT_INDEX (RSTL_VERSION >= RSTL_P2ME)
// `vector(int, const T&)` is one constructor with a defaulted allocator argument.
#define RSTL_VECTOR_DEFAULT_ALLOC_FILL_CTOR (RSTL_VERSION >= RSTL_P2ME)
// ...which carries the (empty) allocation precondition after allocating.
#define RSTL_VECTOR_FILL_CTOR_PRECONDITION (RSTL_CORRUPTION)
// `vector(int)` sets the count and leaves the elements uninitialized.
#define RSTL_VECTOR_UNINITIALIZED_COUNT_CTOR RSTL_ECHOES
// `vector(int)` only reserves, assigning `mCount` in the body.
#define RSTL_VECTOR_RESERVE_CTOR (RSTL_VERSION >= RSTL_R3ME_00)
// `vector(int)` keeps a positive-count check (retail Corruption profiles only).
#define RSTL_VECTOR_COUNT_CTOR_POSITIVE_CHECK (RSTL_CORRUPTION && RSTL_VERSION != RSTL_G2MEAB)
// The stream constructor reserves `count + mCount` elements.
#define RSTL_VECTOR_STREAM_RESERVE_APPEND (RSTL_VERSION >= RSTL_R32J)
// `assign` and CStringExtras fill with `push_back_unsafe`.
#define RSTL_VECTOR_PUSH_BACK_UNSAFE (RSTL_VERSION >= RSTL_P2ME)
// `push_back` appends without growing or checking capacity.
#define RSTL_VECTOR_UNCHECKED_PUSH_BACK (RSTL_VERSION >= RSTL_R3IJ)
// `push_back` grows the storage, followed by `push_back_unsafe` and `pop_back`.
#define RSTL_VECTOR_GROWING_PUSH_BACK                                                              \
  (RSTL_VERSION >= RSTL_P2ME && !RSTL_VECTOR_UNCHECKED_PUSH_BACK)
// `push_back` asserts through `verify_capacity_failed`, with a `push_back_checked` wrapper.
#define RSTL_VECTOR_VERIFIED_PUSH_BACK (RSTL_CORRUPTION)
// `resize_uninitialized` compares against `mCount` in a braced block.
#define RSTL_VECTOR_RESIZE_UNINITIALIZED_COUNT_CHECK (RSTL_CORRUPTION)
// The `erase` move loop declares its destination iterator before the source.
#define RSTL_VECTOR_ERASE_DEST_FIRST (RSTL_VERSION >= RSTL_R32J)
// Trivially destructible elements skip cleanup before iterators are constructed (Wii Echoes).
#define RSTL_VECTOR_SKIP_TRIVIAL_DESTRUCTION (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J)

// ============================================================================================
// rstl: reserved_vector
// ============================================================================================

// `reserved_vector_traits` skips trivial destructor loops, and the count constructor is explicit.
#define RSTL_RESERVED_VECTOR_TRAITS (RSTL_VERSION >= RSTL_R3IJ)
// The count constructor exists.
#define RSTL_RESERVED_VECTOR_COUNT_CTOR (RSTL_RESERVED_VECTOR_TRAITS || RSTL_CORRUPTION)
// `reserved_vector_traits< int >` and `< u32 >` fill by value.
#define RSTL_RESERVED_VECTOR_INT_FILLS (RSTL_VERSION >= RSTL_R3ME_00)
// Destroys through `destroy_elements` and defines `operator=` out of class.
#define RSTL_RESERVED_VECTOR_OUT_OF_CLASS RSTL_ECHOES
// The out-of-line `operator=` is `inline` unless `RSTL_DONT_INLINE_RESERVED_VECTOR` is defined.
#define RSTL_RESERVED_VECTOR_DONT_INLINE_OPTION (RSTL_CORRUPTION)
// Storage is an aligned `uchar` array (the prototype's is unaligned).
#define RSTL_RESERVED_VECTOR_ALIGNED_DATA (!RSTL_CORRUPTION)
// The copy constructor copies through raw pointers.
#define RSTL_RESERVED_VECTOR_RAW_COPY (RSTL_VERSION >= RSTL_P2ME)
// The copy constructor placement-constructs each element through `construction_policy`.
#define RSTL_RESERVED_VECTOR_POLICY_COPY (RSTL_CORRUPTION)
// `resize` also shrinks.
#define RSTL_RESERVED_VECTOR_SHRINKING_RESIZE (RSTL_VERSION >= RSTL_P2ME)
// `reserved_vector_fill_type` exists and `resize` takes it.
#define RSTL_RESERVED_VECTOR_FILL_TYPE (RSTL_CORRUPTION)
// `erase` destroys and reconstructs shifted elements instead of assigning them.
#define RSTL_RESERVED_VECTOR_ERASE_CONSTRUCT (RSTL_VERSION >= RSTL_R3ME_00)
// `destroy_elements` takes a plain `T*` and destroys `&ptr[i]`.
#define RSTL_RESERVED_VECTOR_DESTROY_INDEXED (RSTL_CORRUPTION)
// The const `operator[]` carries the index precondition.
#define RSTL_RESERVED_VECTOR_CONST_INDEX_PRECONDITION (RSTL_PRECONDITION_MACRO)
// `destroy_elements` and `reserved_vector(int, fill_type)` carry (empty) precondition statements.
#define RSTL_RESERVED_VECTOR_PROTO_PRECONDITIONS (RSTL_CORRUPTION)

// ============================================================================================
// rstl: list and listpool
// ============================================================================================

// Members are declared in the class and defined outside it.
#define RSTL_LIST_OUT_OF_CLASS (RSTL_VERSION >= RSTL_P2ME)
// `remove_if` stays in the class body (over nodes).
#define RSTL_LIST_REMOVE_IF_IN_CLASS (RSTL_CORRUPTION)
// The copy constructor guards the partial copy with a `destroy_helper`.
#define RSTL_LIST_COPY_DESTROY_GUARD (RSTL_VERSION < RSTL_P2ME)
// `destroy` calls `clear()` instead of erasing the range.
#define RSTL_LIST_DESTROY_CALLS_CLEAR (RSTL_VERSION >= RSTL_GM8P_00)
// `~list` calls the element destructor directly instead of `rstl::destroy`.
#define RSTL_LIST_DIRECT_DTOR_CALL (RSTL_CORRUPTION)
// Iterators have real postfix `++` / `--`.
#define RSTL_LIST_POSTFIX_ITERATORS (RSTL_VERSION >= RSTL_P2ME)
// The sentinel `mEmpty_prev` is a plain, unaligned pointer.
#define RSTL_LIST_PLAIN_SENTINEL (RSTL_VERSION >= RSTL_P2ME)
// `node` is a plain `uchar` item array, and `do_insert_before` reads `n->mPrev` directly.
#define RSTL_LIST_NODE_PLAIN (RSTL_CORRUPTION)
// Node construction sets the links through their setters.
#define RSTL_LIST_CREATE_NODE_SETTERS (RSTL_VERSION == RSTL_G2MEAB)
// Insertion helpers take pointer elements by value.
#define RSTL_LIST_POINTER_VALUE_PARAM (RSTL_CORRUPTION)
// `exchange` is a plain member that links nodes through `mPrev` / `mNext`.
#define RSTL_LIST_EXCHANGE_DIRECT (RSTL_CORRUPTION)
// `sort`'s inner loop walks a plain node cursor (all but Trilogy).
#define RSTL_LIST_SORT_NODE_CURSOR (RSTL_VERSION < RSTL_R3ME_00)
// `listpool` holds an `Alloc mAllocator` member instead of deriving from `Alloc`.
#define RSTL_LISTPOOL_ALLOCATOR_MEMBER (RSTL_CORRUPTION)

// ============================================================================================
// rstl: bit_vector
// ============================================================================================

// Members are declared in the class and defined outside it.
#define RSTL_BIT_VECTOR_OUT_OF_CLASS RSTL_ECHOES
// `make_room` grows with `push_back_unsafe`.
#define RSTL_BIT_VECTOR_UNSAFE_GROW (RSTL_VERSION >= RSTL_R3ME_00)
// Compact `reference`/iterator records, out-of-line `at` / `operator[]` and explicit growth.
#define RSTL_BIT_VECTOR_COMPACT (RSTL_CORRUPTION)

// ============================================================================================
// rstl: red_black_tree, set and map
// ============================================================================================

// The tree's multi-key template parameter type (must agree with the games' stream constructors).
#if RSTL_PRIME1
#define RSTL_RBTREE_MULTI_TYPE bool
#else
#define RSTL_RBTREE_MULTI_TYPE int
#endif
// Lookups take keys through `tree_key_argument` (`int` and `uint` by value).
#define RSTL_RBTREE_KEY_ARGUMENT_TRAITS (RSTL_CORRUPTION)
#if RSTL_RBTREE_KEY_ARGUMENT_TRAITS
#define RSTL_RBTREE_KEY_ARGUMENT typename tree_key_argument< T >::type
#else
#define RSTL_RBTREE_KEY_ARGUMENT const T&
#endif
// Iterators take a third `bool` ctor argument, and `iterator::operator->` / `*` are non-const.
#define RSTL_RBTREE_PROTO_ITERATORS (RSTL_CORRUPTION)
#if RSTL_RBTREE_PROTO_ITERATORS
#define RSTL_RBTREE_ITERATOR_FLAG , false
#else
#define RSTL_RBTREE_ITERATOR_FLAG
#endif
// `iterator` has a non-const `get_node()` that hides the `const_iterator` one.
#define RSTL_RBTREE_ITERATOR_MUTABLE_GET_NODE (!RSTL_PRIME1)
// `node::mValue` carries `ALIGNAS`.
#define RSTL_RBTREE_NODE_ALIGNED (!RSTL_CORRUPTION)
// The tree has `destroy()` and a destroy-based destructor instead of going through `clear()`.
#define RSTL_RBTREE_DESTROY_HELPER (!RSTL_ECHOES)
// `find_node` returns through a ternary instead of a `noResult` flag.
#define RSTL_RBTREE_TERNARY_FIND (RSTL_VERSION >= RSTL_P2ME)
// ...directly, without a `result` local.
#define RSTL_RBTREE_DIRECT_FIND_RETURN (RSTL_CORRUPTION)
// `find_lower_bound` / `find_upper_bound` start with the (empty) root precondition.
#define RSTL_RBTREE_BOUND_PRECONDITION (RSTL_CORRUPTION)
// Out-of-line members in the prototype's order, using `get_left()` / `get_right()`.
#define RSTL_RBTREE_PROTO_OUT_OF_LINE (RSTL_CORRUPTION)
// `rbtree_rotate_left` / `rbtree_rotate_right` are `static` in rstl_map.cpp.
#define RSTL_RBTREE_STATIC_ROTATIONS RSTL_PRIME1
// Members are declared in the class and defined outside it.
#define RSTL_SET_OUT_OF_CLASS RSTL_ECHOES
// The copy constructor is `inline`.
#define RSTL_SET_INLINE_COPY_CTOR (RSTL_VERSION < RSTL_GM8P_00 || RSTL_PRIME1)
// `set` wraps a `red_black_tree` member (`inner`) instead of deriving from it.
#define RSTL_SET_INNER_TREE (RSTL_CORRUPTION)

// ============================================================================================
// rstl: optional_object and optional_storage
// ============================================================================================

// Storage is an aligned `uchar` array and an aligned `bool` (otherwise a `uint` array).
#define RSTL_OPTIONAL_OBJECT_ALIGNED_STORAGE RSTL_ECHOES
// Storage is a plain `uchar` array followed by `bool m_valid ATTRIBUTE_ALIGN(4)`.
#define RSTL_OPTIONAL_OBJECT_PLAIN_STORAGE (RSTL_CORRUPTION)
// `optional_storage::m_data` carries `ALIGNAS(T)`.
#define RSTL_OPTIONAL_STORAGE_ALIGNED (!RSTL_CORRUPTION)
// The destructor destroys the value directly instead of calling `clear()`.
#define RSTL_OPTIONAL_DTOR_DESTROYS (RSTL_VERSION >= RSTL_GM8P_00)
// ...with a direct destructor call instead of `destroy`.
#define RSTL_OPTIONAL_DTOR_DIRECT_CALL (RSTL_VERSION >= RSTL_R32J)
// `operator=(optional_object_null)` exists and calls `clear()`.
#define RSTL_OPTIONAL_ASSIGN_NULL (RSTL_VERSION >= RSTL_R32J)
// The copy constructor reads validity through the accessor call.
#define RSTL_OPTIONAL_COPY_READS_VALID_CALL (RSTL_CORRUPTION)
// The copy constructor and `assign` carry (empty) precondition statements.
#define RSTL_OPTIONAL_PRECONDITIONS (RSTL_CORRUPTION)

// ============================================================================================
// rstl: string and rstl_strings.cpp
// ============================================================================================

// Members are declared in the class and defined outside it.
#define RSTL_STRING_OUT_OF_CLASS (RSTL_VERSION >= RSTL_P2ME)
// `mNull` is a `const` static initialized in the class instead of explicit specializations.
#define RSTL_STRING_INCLASS_NULL (RSTL_VERSION >= RSTL_RM3E)
// `mNull` is a non-const static.
#define RSTL_STRING_MUTABLE_NULL (RSTL_VERSION >= RSTL_P2ME && !RSTL_STRING_INCLASS_NULL)
// `compare(const _CharTp*, int)` and `operator==(const string&, const char*)` are inline.
#define RSTL_STRING_INLINE_CHAR_COMPARE 1
// Typedefs and comparisons precede the out-of-line specializations; `substr` is out of line.
#define RSTL_STRING_PREDECLARED_SPECIALIZATIONS (RSTL_VERSION >= RSTL_P2ME)
// `operator+(const string&, const string&)` is inline.
#define RSTL_STRING_INLINE_CONCAT RSTL_ECHOES
// Characters are read with `ReadInt8()` instead of `Get< schar >()`.
#define RSTL_STRING_READ_INT8 (RSTL_VERSION >= RSTL_P2ME)
// The stream constructor reads each character into an `int` (otherwise a `char`).
#if RSTL_CORRUPTION
#define RSTL_STRING_STREAM_CH_TYPE int
#else
#define RSTL_STRING_STREAM_CH_TYPE char
#endif
// The copy-on-write buffer is released with `CMemory::Free`.
#define RSTL_STRING_FREE_WITH_CMEMORY (RSTL_VERSION >= RSTL_P2ME)
// rstl_strings.cpp specializes `basic_string< char >::assign(const char*, int)`.
#define RSTL_STRING_ASSIGN_SPECIALIZATION (RSTL_VERSION >= RSTL_GM8P_00)
// `char_traits::assign(T*, int, T)` takes its value by value.
#define RSTL_STRING_TRAITS_VALUE_PARAM (RSTL_CORRUPTION)
// The single-character constructor accepts an allocator argument.
#define RSTL_STRING_CHAR_CTOR_ALLOCATOR (RSTL_CORRUPTION)
// Non-const indexing detaches the buffer; kept separate because it changes existing calls.
#define RSTL_STRING_WRITABLE_INDEX (RSTL_CORRUPTION)
// The `literal_t` constructor measures with a plain `end` pointer loop.
#define RSTL_STRING_SIMPLE_LITERAL_CTOR (RSTL_CORRUPTION)
// The iterator-range constructor terminates with `Traits::eos()` instead of `0`.
#if RSTL_CORRUPTION
#define RSTL_STRING_RANGE_CTOR_EOS Traits::eos()
#else
#define RSTL_STRING_RANGE_CTOR_EOS 0
#endif
// The default constructor carries the (empty) pointer precondition.
#define RSTL_STRING_DEFAULT_CTOR_PRECONDITION (RSTL_CORRUPTION)
// `~basic_string` carries the (empty) pointer precondition.
#define RSTL_STRING_DTOR_PRECONDITION (RSTL_PRECONDITION_MACRO && !RSTL_CORRUPTION)
// `internal_search` is declared `static inline` in the class.
#define RSTL_STRING_INLINE_SEARCH_DECL (RSTL_STRING_OUT_OF_CLASS && !RSTL_CORRUPTION)
// `internal_search` walks `match` and `search` iterators in step (supports `rfind`).
#define RSTL_STRING_MATCH_LOOP_SEARCH (RSTL_CORRUPTION)
// Typedefs, specializations and `operator+` overloads come in the prototype's order.
#define RSTL_STRING_PROTO_TAIL (RSTL_CORRUPTION)
// rstl_strings.cpp's definitions come in the prototype's order.
#define RSTL_STRINGS_PROTO_LAYOUT (RSTL_CORRUPTION)

// ============================================================================================
// rstl: StringExtras and CStringExtras.cpp
// ============================================================================================

// CStringExtras reads with `at()` into a `signed char` buffer; its char parameter is non-const.
#define RSTL_STRING_EXTRAS_AT_ACCESS (RSTL_VERSION >= RSTL_P2ME)
// The unused string list factory is emitted (in PAL/JP Prime it would change the string pool).
#define RSTL_STRING_LIST_FACTORY (RSTL_VERSION >= RSTL_P2ME)
// `ConvertToUNICODE` returns early for an empty string.
#define RSTL_UNICODE_EMPTY_EARLY_RETURN (RSTL_VERSION >= RSTL_P2ME)
// `CStringExtras::CreateFromReal` exists.
#define RSTL_STRING_CREATE_FROM_REAL (RSTL_VERSION < RSTL_P2ME)
// The prototype's API: `ConvertToLowerCase(char)`, `ConvertToInteger`, `ConvertToReal`, `Format`.
#define RSTL_STRING_EXTRAS_PROTO_API (RSTL_CORRUPTION)
// The code lives in RstlExtras.cpp (CStringExtras.cpp is empty).
#define RSTL_STRING_EXTRAS_IN_RSTL_EXTRAS (RSTL_CORRUPTION)

// ============================================================================================
// Kyoto: across folders
// ============================================================================================

// By-value scalar returns are `const` (CBasics::SwapBytes, SObjectTag::GetType, CRandom16).
#define RSTL_KYOTO_CONST_SCALAR_RETURNS RSTL_PRIME1
// `const` on by-value scalar parameters and returns that only Prime declares.
#if RSTL_KYOTO_CONST_SCALAR_RETURNS
#define RSTL_PRIME1_CONST const
#else
#define RSTL_PRIME1_CONST
#endif
// `const` member functions only Echoes declares, and `inline` only Prime declares.
#if RSTL_PRIME1
#define RSTL_ECHOES_CONST
#define RSTL_PRIME1_INLINE inline
#else
#define RSTL_ECHOES_CONST const
#define RSTL_PRIME1_INLINE
#endif
// `const` by-value scalar parameters that Prime and Echoes declare and the prototype does not.
#if RSTL_CORRUPTION
#define RSTL_KYOTO_SCALAR_PARAM_CONST
#else
#define RSTL_KYOTO_SCALAR_PARAM_CONST const
#endif
// Small value types declare their rstl container copy policies.
#define RSTL_KYOTO_CONSTRUCTION_POLICIES (!RSTL_PRIME1)
// CAssetId is the prototype's class (Kyoto/CAssetId.hpp) instead of a `uint`.
#define RSTL_KYOTO_ASSET_ID_CLASS (RSTL_CORRUPTION)
// Resource factories receive the loaded buffer and size instead of a CInputStream.
#define RSTL_KYOTO_BUFFER_FACTORIES (RSTL_CORRUPTION)
// Debug build: asserts, debugger prints, source-line allocation tags and freed-block fills.
#define RSTL_KYOTO_DEBUG_BUILD (RSTL_VERSION == RSTL_G2MEAB)
// Spellings only the prototype's units use where Echoes' equivalent compiles differently.
#define RSTL_KYOTO_PROTO_SOURCE_FORMS (RSTL_CORRUPTION)

// ============================================================================================
// Kyoto: top-level files (resources, paks, tokens, object tags, DVD requests)
// ============================================================================================

// SObjectTag is declared trivially constructible for rstl containers.
#define RSTL_KYOTO_OBJECT_TAG_TRIVIAL (!RSTL_PRIME1)
// SObjectTag has a user-declared copy constructor.
#define RSTL_KYOTO_OBJECT_TAG_COPY_CTOR (RSTL_PRIME1 || RSTL_CORRUPTION)
// TUniqueId's data is a `uint`.
#define RSTL_KYOTO_UINT_UNIQUE_ID (RSTL_CORRUPTION)
// IObjectStore declares a pure virtual destructor ahead of its other virtuals.
#define RSTL_KYOTO_OBJECT_STORE_VIRTUAL_DTOR (RSTL_CORRUPTION)
// CObjectReference::AddReference is defined out of line.
#define RSTL_KYOTO_OBJECT_REFERENCE_ADD_OUT_OF_LINE (RSTL_CORRUPTION)
// The demo discs' CObjectReference packs its counts into one word, as Prime's does.
#define RSTL_KYOTO_OBJECT_REFERENCE_PACKED_COUNTS (RSTL_PRIME1 || RSTL_ECHOES_DEMO)
// The demo discs' CModelFlags::DepthBackwards only sets the greater-depth flag, as Prime's does.
#define RSTL_KYOTO_MODEL_FLAGS_DEPTH_BACKWARDS_SET_ONLY (RSTL_ECHOES_DEMO)
// IObj and CObjOwnerDerivedFromIObjUntyped declare their destructors inline in the class.
#define RSTL_KYOTO_IOBJ_INLINE_DTOR (RSTL_CORRUPTION)
// TObjOwnerParam::GetData returns the held value by const reference.
#define RSTL_KYOTO_PARAM_DATA_REFERENCE (RSTL_PRIME1 || RSTL_CORRUPTION)
// CSimplePool's resource map orders tags by id, then by type.
#define RSTL_KYOTO_POOL_TAG_TYPE_TIEBREAK (RSTL_CORRUPTION)
// CResLoader looks for network assets before the paks.
#define RSTL_KYOTO_RES_LOADER_NETWORK_ASSETS (RSTL_CORRUPTION)
// CResFactory::GetResourceIdToNameList is defined in its header.
#define RSTL_KYOTO_RES_FACTORY_INLINE_NAME_LIST (RSTL_CORRUPTION)
// Corruption's CPakFile: asset names, a 16 KB header read, CAssetTypesList type indices,
// out-of-line CResInfo copies and invalid-ID default records.
#define RSTL_KYOTO_PAK_FILE_CORRUPTION (RSTL_CORRUPTION)
// CDependencyGroupToken is defined in CDependencyGroup.cpp.
#define RSTL_KYOTO_DEPENDENCY_GROUP_TOKEN (!RSTL_PRIME1)
// CDependencyGroup::ReadFromStream reads the tag type with `Get< FourCC >()`.
#define RSTL_KYOTO_ECHOES_DEPENDENCY_GROUP (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// CDependencyGroup::ReadFromStream appends its tags with `push_back_unsafe`.
#define RSTL_KYOTO_DEPENDENCY_PUSH_UNSAFE (!RSTL_PRIME1)
// CRandom16::Float draws 28 bits from the seed directly and GetRandomNumber asserts a generator.
#define RSTL_KYOTO_RANDOM16_PROTOTYPE (RSTL_CORRUPTION)
// Wii ARAM tokens wait for the DMA instead of polling their status.
#define RSTL_KYOTO_ARAM_TOKEN_SYNC_WAIT (RSTL_VERSION >= RSTL_RM3E)
// CDvdRequest has the `IsReady` virtual (Wii).
#define RSTL_KYOTO_DVD_REQUEST_READY (RSTL_VERSION >= RSTL_RM3E)
// The DVD media type is 2 (Wii Prime and Echoes) instead of 1.
#if RSTL_VERSION >= RSTL_R3IJ && (RSTL_PRIME1 || defined(PRIME2))
#define RSTL_KYOTO_DVD_REQUEST_MEDIA_TYPE_2 1
#else
#define RSTL_KYOTO_DVD_REQUEST_MEDIA_TYPE_2 0
#endif

// ============================================================================================
// Kyoto/Alloc
// ============================================================================================

// Wii allocators manage separate MEM1 and MEM2 heaps.
#define RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS (RSTL_VERSION >= RSTL_RM3E)
// The medium pool's size-range search and puddle flag (Trilogy built without PRIME1 or PRIME2).
#if RSTL_VERSION >= RSTL_R3ME_00 && !RSTL_PRIME1 && !defined(PRIME2)
#define RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL 1
#else
#define RSTL_KYOTO_ALLOCATOR_EXTENDED_MEDIUM_POOL 0
#endif
// IAllocator has `OffsetFakeStatics` (single-heap builds, and Prime and Echoes on Wii).
#if !RSTL_KYOTO_ALLOCATOR_MULTIPLE_HEAPS || RSTL_PRIME1 || defined(PRIME2)
#define RSTL_KYOTO_ALLOCATOR_OFFSET_FAKE_STATICS 1
#else
#define RSTL_KYOTO_ALLOCATOR_OFFSET_FAKE_STATICS 0
#endif
// `GetMetrics(bool resetPeriodicPeak, bool resetPeak)` and the periodic peak in `SMetrics`.
#define RSTL_KYOTO_ALLOCATOR_METRICS_RESET (!RSTL_PRIME1)
// `SMetrics` declares a copy constructor.
#define RSTL_KYOTO_METRICS_COPY_CTOR (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// CGameAllocator::Alloc queries pool statistics and call-stack text whose results are unused.
#define RSTL_KYOTO_ALLOCATOR_DEBUG_QUERIES (RSTL_PRIME1 || RSTL_CORRUPTION)
// CGameAllocator::DumpAllocations calls the unit's `GetLargestFreeChunk`.
#define RSTL_KYOTO_ALLOCATOR_LARGEST_FREE_CHUNK RSTL_PRIME1
// Wii's small pool records the last allocation size and returns a block count from Free.
#define RSTL_KYOTO_SMALL_ALLOC_POOL_BLOCK_COUNTS (RSTL_VERSION >= RSTL_RM3E)
// Plain `new` / `new[]` are inline wrappers passing "??(??)" to the source-position operator new.
#define RSTL_KYOTO_INLINE_GLOBAL_NEW (!RSTL_PRIME1)
// CCallStack also declares the prototype's (int, const char*, const char*) constructor.
#define RSTL_KYOTO_CALLSTACK_SIGNED_CTOR (RSTL_CORRUPTION)
// CCircularBuffer.cpp defines `GetAllocatedAmount`.
#define RSTL_KYOTO_CIRCULAR_BUFFER_ALLOCATED_AMOUNT (RSTL_PRIME1 || RSTL_CORRUPTION)

// ============================================================================================
// Kyoto/Animation
// ============================================================================================

// GameCube Echoes emits the streamed compression duration method in the reader unit.
#define RSTL_CFB_ANIMATION_DURATION_IN_READER (!RSTL_PRIME1 && RSTL_ECHOES_GC)
// CLinearAnimationTimeScale::CFunctionDescription members are const.
#if RSTL_CORRUPTION
#define RSTL_KYOTO_TIMESCALE_CONST const
#else
#define RSTL_KYOTO_TIMESCALE_CONST
#endif
// CSegIdList's destructor is defined in CSegIdList.cpp.
#define RSTL_KYOTO_SEGIDLIST_OUT_OF_LINE_DTOR (RSTL_CORRUPTION)
// CAdvancementDeltas defines the translation-only and rotation-only blends.
#define RSTL_KYOTO_ADVANCEMENT_PARTIAL_BLENDS (RSTL_CORRUPTION)

// ============================================================================================
// Kyoto/Audio
// ============================================================================================

// Wii Echoes zeroes the audio effect with CBasics and defines the name methods out of line.
#define RSTL_AUDIO_EFFECT_WII (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J)
// Wii Echoes passes the input mix through while RSF decoding is muted.
#define RSTL_RSF_AUDIO_MUTE_PASSTHROUGH (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_R32J)

// ============================================================================================
// Kyoto/Basics
// ============================================================================================

// Wii Prime runs on the Revolution SDK: CBasics::Init calls NANDInit, CStopwatch::Wait uses fabsf.
#define RSTL_KYOTO_REVOLUTION_SDK (RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3IJ)
// CBasics::ZeroMemory/CopyMemory are defined in CBasicsDolphin.cpp (Prime Trilogy's are not).
#define RSTL_KYOTO_BLOCK_MEMORY_FUNCTIONS (!RSTL_PRIME1)
// Wii uses a cache-block fill for nonzero bytes as well.
#define RSTL_KYOTO_BLOCK_MEMORY_FILL (RSTL_VERSION >= RSTL_R3IJ)
// CCast overloads only Prime declares: ToUint32(uint), ToReal32(const short&), ToInt32(double).
#define RSTL_KYOTO_CAST_PRIME_OVERLOADS RSTL_PRIME1
// CCast::ToReal32(int) exists (not in the prototype).
#define RSTL_KYOTO_CAST_REAL32_INT (!RSTL_CORRUPTION)
// CCast::ToUint32(int) exists (prototype only).
#define RSTL_KYOTO_CAST_UINT32_INT (RSTL_CORRUPTION)
// CStopwatch::Wait skips waits at or below FLT_EPSILON.
#define RSTL_KYOTO_STOPWATCH_EPSILON_WAIT (!RSTL_PRIME1)
// Stringize uses `vsnprintf` instead of `vsprintf`.
#define RSTL_KYOTO_STRINGIZE_VSNPRINTF (!RSTL_PRIME1 || RSTL_VERSION >= RSTL_R3IJ)
// Stringize has a `va_list` overload.
#define RSTL_KYOTO_STRINGIZE_VA_LIST (RSTL_VERSION >= RSTL_G2MEAB)
// The variadic Stringize delegates to the `va_list` overload.
#define RSTL_KYOTO_STRINGIZE_DELEGATES (RSTL_CORRUPTION)
// The fatal error handler switches to a clean OSContext and waits for controller port 1.
#define RSTL_KYOTO_ERROR_HANDLER_NEW_CONTEXT (RSTL_VERSION >= RSTL_GM8E_01)
// Echoes' fatal error handler: halts audio and DVD, resets when idle, names REL addresses.
#define RSTL_KYOTO_REL_ERROR_HANDLER (!RSTL_PRIME1)
// BUILD_TIME_DUMMY holds a real build string instead of padding.
#define RSTL_KYOTO_BUILD_TIME_STRING (!RSTL_PRIME1)
// Echoes PAL's build string.
#define RSTL_KYOTO_BUILD_TIME_PAL (RSTL_VERSION == RSTL_G2MP)
// COsContext's Wii MEM1/MEM2 arenas and preallocated video buffers.
#define RSTL_KYOTO_WII_OS_CONTEXT (RSTL_VERSION >= RSTL_RM3E)
// COsContext stores OSGetLanguage() and reserves a graphics arena block.
#define RSTL_KYOTO_OS_CONTEXT_ARENA_BLOCK                                                          \
  (!RSTL_PRIME1 || RSTL_VERSION == RSTL_GM8P_00 || RSTL_KYOTO_WII_OS_CONTEXT)
// COsContext has a virtual destructor.
#define RSTL_KYOTO_OS_CONTEXT_VIRTUAL (RSTL_CORRUPTION || RSTL_KYOTO_WII_OS_CONTEXT)
// Non-Prime Wii builds keep the system aspect ratio in a separate word after the language.
#define RSTL_KYOTO_OS_CONTEXT_ASPECT_RATIO (!RSTL_PRIME1 && RSTL_KYOTO_WII_OS_CONTEXT)
// COsContext has an extra word before the MEM1 arena (Trilogy built without PRIME1 or PRIME2).
#if RSTL_VERSION >= RSTL_R3ME_00 && !RSTL_PRIME1 && !defined(PRIME2)
#define RSTL_KYOTO_OS_CONTEXT_EXTRA_ARENA_FIELD 1
#else
#define RSTL_KYOTO_OS_CONTEXT_EXTRA_ARENA_FIELD 0
#endif
// Prime Trilogy reserves a larger auxiliary GPU buffer.
#define RSTL_KYOTO_OS_CONTEXT_LARGE_GPU_BUFFER (RSTL_PRIME1 && RSTL_VERSION >= RSTL_R3ME_00)

// ============================================================================================
// Kyoto/Graphics
// ============================================================================================

// Corruption's CGraphics: lighting modes, draw-sync tokens, alpha contexts, a uchar light mask
// and a blue clear color.
#define RSTL_KYOTO_GRAPHICS_CORRUPTION (RSTL_CORRUPTION)
// CGraphics allocates its video buffers from the OS arena block (PAL Prime and all non-Prime).
#define RSTL_KYOTO_GRAPHICS_ARENA (!RSTL_PRIME1 || RSTL_VERSION == RSTL_GM8P_00)
// PAL builds switch between 50 Hz PAL and EURGB60 instead of progressive scan.
#define RSTL_KYOTO_PAL_VIDEO (RSTL_VERSION == RSTL_GM8P_00 || RSTL_VERSION == RSTL_G2MP)
// CGraphics::ConfigureVideoMode takes its progressive flag as a `uchar`.
#define RSTL_KYOTO_UCHAR_VIDEO_MODE_FLAG (RSTL_VERSION >= RSTL_GM8P_00 && !RSTL_CORRUPTION)
// CGraphics::InitGraphicsFifo sets the fifo base again before its limits.
#define RSTL_KYOTO_GRAPHICS_FIFO_REBASE (RSTL_VERSION == RSTL_G2MP || RSTL_CORRUPTION)
// CGraphics can render streamed primitives with a per-vertex delay.
#define RSTL_KYOTO_GRAPHICS_STREAM_VERTEX_DELAY (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// GXGetGPFifo copies the FIFO into an output object instead of returning a pointer.
#define RSTL_KYOTO_FIFO_COPY                                                                       \
  ((RSTL_VERSION >= RSTL_RM3E && RSTL_VERSION <= RSTL_RM3J) || RSTL_VERSION >= RSTL_R3IJ)
// Display-list triangle loops keep the packet's 16-bit vertex count.
#define RSTL_KYOTO_DISPLAY_LIST_SHORT_COUNT (RSTL_VERSION >= RSTL_RM3E)
// Corruption's CGX: extended GX state, out-of-line helpers, GXBool z mode, apply_fog in the
// header and its own function order.
#define RSTL_KYOTO_CGX_CORRUPTION (RSTL_CORRUPTION)
// CGX keeps its state behind the `.sdata` pointer `CGX::gpGXState`.
#define RSTL_CGX_STATE_POINTER RSTL_ECHOES_GC
// Corruption's CLight: processed color, no hard-light helpers, and its own function order.
#define RSTL_KYOTO_LIGHT_CORRUPTION (RSTL_CORRUPTION)
// Wii builds read the CColor yellow and grey constants directly instead of through getters.
#define RSTL_CCOLOR_INLINE_CONSTANTS (RSTL_VERSION >= RSTL_RM3E)
// CColor::WithAlphaOf multiplies the byte scale by the alpha.
#define RSTL_KYOTO_COLOR_ALPHA_SCALE_FIRST (RSTL_VERSION >= RSTL_R32J)
// CColor::ToRGB5A3 widens its channels to `uint`.
#define RSTL_KYOTO_COLOR_UINT_RGB5A3_CHANNELS (RSTL_CORRUPTION)
// CGraphicsPalette can keep its entries in GPU memory (otherwise UnLock stores the entry range).
#define RSTL_KYOTO_PALETTE_GPU_MEMORY (RSTL_CORRUPTION)
// GPUMemory::EnsureAllocation marks itself allocating once it has the memory.
#define RSTL_KYOTO_GPU_MEMORY_LATE_ALLOCATION_FLAG (RSTL_CORRUPTION)
// CTexture can cap a texture's size as it loads and records the mip bytes it skips.
#define RSTL_KYOTO_TEXTURE_SECONDARY_MEMORY (RSTL_CORRUPTION)
// The ordinary spare-texture loader takes a min/mag filter.
#define RSTL_KYOTO_SPARE_TEXTURE_FILTER                                                          \
  (RSTL_CORRUPTION || (!RSTL_PRIME1 && RSTL_VERSION == RSTL_R3MP_00))
// The CI spare-texture loader takes a filter.
#define RSTL_KYOTO_SPARE_CI_TEXTURE_FILTER (RSTL_CORRUPTION)
// CMoviePlayer::InitializeTextures appends its texture sets with `push_back_unsafe`.
#define RSTL_KYOTO_MOVIE_TEXTURE_PUSH_UNSAFE (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// CMoviePlayer::DecodeFromRead warns separately about oversized 60fps and other frames.
#define RSTL_KYOTO_MOVIE_FRAME_SIZE_WARNINGS (RSTL_CORRUPTION)

// ============================================================================================
// Kyoto/Input
// ============================================================================================

// The Revolution controller interface.
#define RSTL_CONTROLLER_WII (RSTL_VERSION >= RSTL_RM3E)
// Later controllers add acceptance and motion-activity slots.
#define RSTL_CONTROLLER_EXTENDED_VIRTUALS (RSTL_VERSION >= RSTL_RM3P)
// CRevolutionController supports up to four channels (`SetMaxControllers`); Wii Echoes only.
#if defined(PRIME2) && RSTL_VERSION >= RSTL_R32J
#define RSTL_CONTROLLER_CHANNEL_LIMIT 1
#else
#define RSTL_CONTROLLER_CHANNEL_LIMIT 0
#endif
// CFinalInput holds the controller snapshot and queries its buttons and axes.
#define RSTL_FINAL_INPUT_CONTROLLER_DATA (RSTL_VERSION >= RSTL_RM3E)

// ============================================================================================
// Kyoto/Math
// ============================================================================================

// Corruption's CVector3f: Normalize returns the magnitude and Cross is out of line.
#define RSTL_KYOTO_VECTOR3F_CORRUPTION (RSTL_CORRUPTION)
// CVector3f declares reserved_vector, optional and vector construction policies.
#define RSTL_KYOTO_VECTOR3F_CONTAINER_POLICIES (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// CVector3f has the component-wise `operator*`.
#define RSTL_KYOTO_VECTOR3F_ELEMENTWISE_MULTIPLY (RSTL_PRIME1 || RSTL_CORRUPTION)
// CVector3d has compound assignment, scalar multiplication and equality operators.
#define RSTL_KYOTO_VECTOR3D_COMPOUND (RSTL_CORRUPTION)
// CVector3i's const `operator[]` returns a reference.
#define RSTL_KYOTO_VECTOR3I_INDEX_REFERENCE (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// CUnitVector3f takes one run-time normalization flag enum instead of per-choice overloads.
#define RSTL_KYOTO_UNIT_VECTOR_FLAG_ENUM (RSTL_CORRUPTION)
// Corruption's CTransform4f: TransformPointArray and FromEulerAngles, and direct field reads.
#define RSTL_KYOTO_TRANSFORM4F_CORRUPTION (RSTL_CORRUPTION)
// CTransform4f defines Shear.
#define RSTL_KYOTO_TRANSFORM4F_SHEAR (!RSTL_PRIME1 && !RSTL_CORRUPTION)
// CQuaternion::IsValidQuaternion tests finiteness and the squared length in double.
#define RSTL_KYOTO_QUATERNION_VALID_DOUBLE (RSTL_CORRUPTION)
// CNUQuaternion declares trivial construction for rstl containers.
#define RSTL_KYOTO_NUQUATERNION_TRIVIAL_CONSTRUCTION (RSTL_CORRUPTION)
// CPlane declares trivial destruction and reserved_vector push construction.
#define RSTL_KYOTO_PLANE_PUSH_CONSTRUCTION (RSTL_CORRUPTION)
// CFrustumPlanes passes the far plane's constant straight to CPlane (no local).
#define RSTL_KYOTO_FRUSTUM_FAR_CONSTANT_INLINE (RSTL_CORRUPTION)
// CSphere defines its bounding boxes.
#define RSTL_KYOTO_SPHERE_BOUNDING_BOX (RSTL_CORRUPTION)
// CTri declares a copy constructor defined out of line.
#define RSTL_KYOTO_TRI_OUT_OF_LINE_COPY (RSTL_CORRUPTION)

// ============================================================================================
// Kyoto/Particles
// ============================================================================================

// Element generator behaviour confined to CElementGen.cpp that the Echoes demo discs lack.
#define RSTL_PARTICLE_ELEMENT_GEN_RETAIL (!RSTL_PRIME1 && !RSTL_ECHOES_DEMO)
// Fixed (FXBR/FXBO) and dynamic bounds modes.
#define RSTL_PARTICLE_BOUNDS_MODES (!RSTL_PRIME1 && !RSTL_ECHOES_DEMO)
// Spawn and sorted systems store the child flags enum, not just the OPTS bit.
#define RSTL_PARTICLE_SPAWN_OPTIONAL_FLAGS (!RSTL_ECHOES_DEMO)
// Velocity sources use the scaled emitter translation.
#define RSTL_PARTICLE_VELOCITY_SCALED_TRANSLATION (!RSTL_PRIME1 && !RSTL_ECHOES_DEMO)
// The RDOP description flag.
#define RSTL_PARTICLE_RDOP (!RSTL_PRIME1 && !RSTL_ECHOES_DEMO)
// The angle-sphere offset from the emitter origin is normalized (not Prime or US Echoes).
#define RSTL_PARTICLE_ANGLE_SPHERE_NORMALIZE                                                       \
  (!RSTL_PRIME1 && RSTL_VERSION != RSTL_G2ME && !RSTL_ECHOES_DEMO)
// CParticleSwoosh.cpp keeps its weak `vector< CParticleGen* >::clear` copy.
#define RSTL_PARTICLE_SWOOSH_GEN_CLEAR RSTL_ECHOES_GC
// CParticleData holds only tag, scale and parented mode, read inline.
#define RSTL_KYOTO_PARTICLE_DATA_PROTOTYPE (RSTL_CORRUPTION)
// CParticleGen has an extra word (retail Corruption, and Trilogy without PRIME1 or PRIME2).
#if RSTL_VERSION >= RSTL_RM3E && !RSTL_PRIME1 && !defined(PRIME2)
#define RSTL_KYOTO_PARTICLE_GEN_EXTRA_FIELD 1
#else
#define RSTL_KYOTO_PARTICLE_GEN_EXTRA_FIELD 0
#endif

// ============================================================================================
// Kyoto/Streams
// ============================================================================================

// CInputStream is Prime's abstract block-buffered stream, and CZipInputStream inflates per block.
// The demo discs still use Prime's block-reading input stream.
#define RSTL_KYOTO_BLOCK_INPUT_STREAM (RSTL_PRIME1 || RSTL_ECHOES_DEMO)
// The memory-buffer CInputStream steps its cursor through typed union members.
#define RSTL_KYOTO_INPUT_STREAM_TYPED_CURSOR (RSTL_CORRUPTION)
// Prime's COutputStream: the bit shift register (0x7C bytes) and its own pair helper.
#define RSTL_KYOTO_PRIME_OUTPUT_STREAM RSTL_PRIME1
// COutputStream keeps buffers of 64 bytes or fewer in an inline scratch array (not the demo).
#define RSTL_KYOTO_OUTPUT_SCRATCH_BUFFER (RSTL_VERSION != RSTL_GM8EAB_00)
// The bit writer's shift register offset is `volatile`.
#define RSTL_KYOTO_VOLATILE_SHIFT_OFFSET (!RSTL_CORRUPTION)
// Out-of-line 64-bit bit stream reads and writes, low word first.
#define RSTL_KYOTO_PROTO_STREAM_HELPERS (RSTL_CORRUPTION)
// CZipOutputStream has `Finish`, ends deflate in its destructor and checks deflate's result.
#define RSTL_KYOTO_ZIP_OUTPUT_FINISH (RSTL_CORRUPTION)

// ============================================================================================
// Kyoto/Text
// ============================================================================================

// Japanese GameCube Echoes and later builds use a line-spacing override.
#define RSTL_GUI_TEXT_LINE_SPACING (!RSTL_PRIME1 && RSTL_VERSION >= RSTL_G2MJ)
// CLineInstruction declares its destructor explicitly, giving the virtual order from R32J on.
#define RSTL_KYOTO_LINE_INSTRUCTION_INLINE_DTOR (RSTL_VERSION >= RSTL_R32J)

#include "Kyoto/Platform.h"

#endif // _RSTL_VERSIONS_H
