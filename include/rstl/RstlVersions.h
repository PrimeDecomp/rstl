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
#define RSTL_G2ME 10
#define RSTL_G2MP 11
#define RSTL_G2MJ 12

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

// Feature macros. Each is 1 when the selected RSTL_VERSION has the behavior and 0 otherwise;
// the rstl sources test these instead of comparing RSTL_VERSION directly.
//
// RSTL_ECHOES_ERA groups GameCube Echoes with Corruption, including the built E3 prototype.
// Retail Corruption selections are provisional; see Prime's MCP shared-rstl.md for build coverage.
#define RSTL_ECHOES_ERA (RSTL_VERSION >= RSTL_G2ME && RSTL_VERSION < RSTL_R3IJ)

// RSTL_PRIME1 is 1 when the code is built for Metroid Prime. The Trilogy versions (R3ME, R3MP) are
// shared by Prime and Echoes, so they are told apart by the game's `PRIME1` define. Hunks that
// only differ between the two games' otherwise identical code are gated on this.
#if defined(PRIME1) || RSTL_VERSION < RSTL_G2ME || RSTL_VERSION == RSTL_R3IJ
#define RSTL_PRIME1 1
#else
#define RSTL_PRIME1 0
#endif

// --- Allocation ---
// Containers and strings derive privately from `Alloc`; otherwise they hold an `Alloc
// mAllocator` member.
#define RSTL_ALLOCATOR_BASE_CLASS (RSTL_VERSION >= RSTL_R3IJ)
// Containers allocate and free through the `mAllocator` instance instead of static `Alloc::` calls.
#define RSTL_ALLOCATOR_INSTANCE RSTL_ECHOES_ERA
// `rmemory_allocator::allocate(int)` is declared only and always called out of line (defined in
// RstlExtras.cpp), `deallocate` deletes without a null check, and `allocate2` exists.
#define RSTL_ALLOCATOR_EXTERN_ALLOCATE (RSTL_VERSION >= RSTL_G2ME)
// Echoes Trilogy USA exports the allocation entry point as global_allocate.
#define RSTL_ALLOCATOR_GLOBAL_ALLOCATE (!RSTL_PRIME1 && RSTL_VERSION == RSTL_R3ME_00)
// Allocation calls the out-of-line `rmemory_allocator::allocate(int)` (MWCC only).
#define RSTL_OUTLINED_ALLOCATE (RSTL_VERSION >= RSTL_GM8P_00)
// rstl_allocator.cpp defines `rmemory_allocator::allocate(int)` (GM8E_02 defines it although
// nothing calls it yet).
#define RSTL_EMIT_OUTLINED_ALLOCATE (RSTL_VERSION >= RSTL_GM8E_02)

// --- Construction and preconditions ---
// `construct` and `destroy(T*)` dispatch through `construction_policy` / `destruction_policy`
// instead of acting directly (the policy classes exist everywhere; MWCC inlines the direct form
// better, which the Wii and Prime 1 builds depend on).
#define RSTL_CONSTRUCTION_POLICY RSTL_ECHOES_ERA
// Precondition checks are the empty `RSTL_PRECONDITION` statements; otherwise `RS_ASSERT` or
// `((void)0)` stubs.
#define RSTL_PRECONDITION_MACRO (RSTL_VERSION >= RSTL_G2ME)
// `vector::operator[]` and the non-const `reserved_vector::operator[]` carry the index
// precondition (on Trilogy the extra statement stops CInputFilter's loops from unrolling).
#define RSTL_INDEX_PRECONDITION (RSTL_ECHOES_ERA && RSTL_VERSION != RSTL_G2MEAB)
// Member functions are declared in the class and defined outside it (or marked `inline` there),
// not defined in the class body. This is chosen per container.
#define RSTL_STRING_OUT_OF_CLASS (RSTL_VERSION >= RSTL_G2ME)
#define RSTL_LIST_OUT_OF_CLASS (RSTL_VERSION >= RSTL_G2ME)
#define RSTL_VECTOR_OUT_OF_CLASS (RSTL_VERSION >= RSTL_G2ME)
// bit_vector, reserved_vector and set regress on Trilogy in this form (frame size,
// `reserved_vector_traits` against `destroy_elements`, and a stream constructor that conflicts
// with CInputStream.hpp), so they stay Echoes-only.
#define RSTL_BIT_VECTOR_OUT_OF_CLASS RSTL_ECHOES_ERA
#define RSTL_SET_OUT_OF_CLASS RSTL_ECHOES_ERA
// `construct`, `destroy` and `destroy_impl` are external `inline` templates instead of `static
// inline` ones (the uninitialized_* helpers are `static inline` everywhere).
#define RSTL_EXTERN_CONSTRUCT_FUNCTIONS RSTL_ECHOES_ERA
// Declare pointer/uint trivial-destructor traits here. The prototype supplies equivalent
// traits earlier through RSTL_SCALAR_TYPE_TRAITS; disabling this does not make pointers non-trivial.
#define RSTL_TRIVIAL_POINTERS                                                                      \
  ((RSTL_ECHOES_ERA && RSTL_VERSION != RSTL_G2MEAB) || RSTL_VERSION >= RSTL_R3ME_00)
// The arithmetic types are declared trivially destructible and copied by assignment, and so are
// pointers. Elsewhere only the types above skip destructor loops; everything else is copied with
// a placement new.
#define RSTL_TRIVIAL_SCALARS RSTL_ECHOES_ERA
// `construct< int >` stores directly.
#define RSTL_DIRECT_INT_CONSTRUCT                                                                  \
  ((RSTL_ECHOES_ERA && RSTL_VERSION != RSTL_G2MEAB) || RSTL_VERSION >= RSTL_R3ME_00)
// The direct `destroy(T*)` takes a `T* const` (Echoes' takes a plain pointer).
#define RSTL_DESTROY_CONST_PARAM (!RSTL_ECHOES_ERA)
// The direct `destroy(T*)` carries the (empty) null precondition, which Prime 1 from GM8P_00 on and
// the Wii versions lack.
#define RSTL_DESTROY_PRECONDITION (RSTL_VERSION < RSTL_GM8P_00)
// The range algorithms check `begin <= end` (and `n >= 0`) first; `destroy(begin, end)` then calls
// `destroy_impl`, and `uninitialized_copy` has a pointer overload without the checks.
#define RSTL_RANGE_PRECONDITIONS RSTL_ECHOES_ERA
// `destroy(T*, T*)` is a separate overload over pointer ranges.
#define RSTL_POINTER_RANGE_OVERLOADS (RSTL_ECHOES_ERA || RSTL_VERSION >= RSTL_R3IJ)
// `destroy(begin, end)` over iterators tests `begin != end` before looping (pointer ranges do not).
#define RSTL_DESTROY_RANGE_GUARD (RSTL_VERSION >= RSTL_R3IJ)
// `uninitialized_copy` steps `++cur, ++tmp` (otherwise `tmp += 1, ++cur`), and
// `uninitialized_copy_n` uses a pre-increment loop when it does not use the counted form below.
#define RSTL_PREINCREMENT_COPY_LOOPS (RSTL_ECHOES_ERA || RSTL_VERSION >= RSTL_R3IJ)
// `uninitialized_copy_n` counts down a `remaining` variable.
#define RSTL_COUNTED_COPY_N_LOOP (RSTL_ECHOES_ERA || RSTL_VERSION >= RSTL_R3ME_00)
// ...and declares that counter before the iterator copies (instead of in the `for` header).
#define RSTL_COPY_N_COUNTER_FIRST (RSTL_VERSION >= RSTL_R3ME_00)
// `uninitialized_fill_n` has a pointer overload that assigns the elements.
#define RSTL_POINTER_FILL_OVERLOAD (RSTL_VERSION >= RSTL_R3IJ)
// Byte fills take their scalar value directly.
#define RSTL_BYTE_FILL_VALUE (RSTL_VERSION == RSTL_G2MEAB)
// `uninitialized_fill_n` fills through `construct` instead of a placement new.
#define RSTL_FILL_USES_CONSTRUCT RSTL_ECHOES_ERA
// `uninitialized_fill_n` advances its `dest` parameter itself instead of a copy, and
// `bit_vector::insert` uses counted loops.
#define RSTL_COUNTED_FILL_LOOPS (RSTL_VERSION >= RSTL_R3ME_00)
// `construct` has direct overloads for `uint`, `bool`, `float` and pointers, and
// `reserved_vector_traits` has fill/push fast paths for scalar elements.
#define RSTL_SCALAR_TRAITS (RSTL_VERSION >= RSTL_R3ME_00)

// `auto_ptr` pointer constructor defaults to null.
#define RSTL_AUTO_PTR_CONSTRUCT_NULL (RSTL_VERSION == RSTL_G2MEAB)
// `auto_ptr` has a converting copy constructor from `auto_ptr< U >` (the GameCube compiler rejects
// its template friend declaration).
#define RSTL_AUTO_PTR_CONVERSION (RSTL_VERSION >= RSTL_R3IJ)
// `auto_ptr::release` returns null when it does not own the item.
#define RSTL_AUTO_PTR_CHECKED_RELEASE (RSTL_VERSION >= RSTL_G2MEAB)

// `single_ptr` has a const-ref copy constructor that transfers ownership.
#define RSTL_SINGLE_PTR_TRANSFER (RSTL_VERSION >= RSTL_G2ME)
// `~single_ptr` deletes through `get()` instead of `mPtr`.
#define RSTL_SINGLE_PTR_DELETE_VIA_GET (RSTL_VERSION >= RSTL_R3IJ)
// `rc_ptr` is the standalone implementation counting through `rc_ptr_private::sNull`.
#define RSTL_RC_PTR_PRIVATE_COUNT RSTL_ECHOES_ERA
// `rc_ptr` keeps `const T* mPtr` plus an `int* mCount` that starts at `CRefData::sNull`.
#define RSTL_RC_PTR_INLINE_COUNT (RSTL_VERSION >= RSTL_R3IJ)

// --- Iterators and algorithms ---
// `distance`, `advance` and their tag-dispatch helpers are `inline`.
#define RSTL_INLINE_ITERATOR_HELPERS (RSTL_VERSION >= RSTL_G2ME)
// `advance_iterator` is `inline`.
#define RSTL_INLINE_ADVANCE_ITERATOR (RSTL_VERSION >= RSTL_G2ME && RSTL_VERSION != RSTL_G2MEAB)
// `min_val` / `max_val` exist and take their arguments by value.
#define RSTL_MIN_MAX_VAL (RSTL_VERSION >= RSTL_G2ME)
// `pointer_iterator` `operator+` / `operator-` copy the iterator and use `+=` / `-=`.
#define RSTL_POINTER_ITERATOR_COPY_ARITHMETIC (RSTL_VERSION >= RSTL_R3ME_00)
// `less` declares its own typedefs instead of deriving from `binary_function`, and `less<
// unsigned int >` exists.
#define RSTL_LESS_WITHOUT_BASE (RSTL_VERSION >= RSTL_R3ME_00)
// `__insertion_sort` takes a non-const `first` and the partition loop is `for (;;)`.
#define RSTL_SORT_ECHOES_LOOPS (RSTL_VERSION >= RSTL_G2ME)
// `__insertion_sort` computes `t1 = next - 1` (the Wii compiler hoists the `subi` above the value
// load, so it steps `t1 = next; --t1;` instead).
#define RSTL_INSERTION_SORT_NEXT_MINUS_ONE RSTL_ECHOES_ERA
// `lower_bound` advances `start` itself with a loop-local iterator.
#define RSTL_LOWER_BOUND_ADVANCES_START (RSTL_VERSION >= RSTL_G2ME)
// `binary_find` and `equal_range` keep named `lower` / `found` locals.
#define RSTL_NAMED_SEARCH_LOCALS (RSTL_VERSION >= RSTL_R3IJ)

// --- vector, list and reserved_vector ---
// `vector::begin` builds the iterator from `mItems` instead of `data()`.
#define RSTL_VECTOR_ITEM_BEGIN (RSTL_VERSION >= RSTL_G2ME)
// `vector::end` and `insert_into` bind their pointers to `T* const` locals.
#define RSTL_VECTOR_CONST_LOCALS (RSTL_VERSION >= RSTL_GM8P_00)
// `insert_into` destroys the moved-from element through a `T* const item` local.
#define RSTL_VECTOR_INSERT_CONST_ITEM (RSTL_VERSION < RSTL_G2ME)
// `vector(int)` sets the count and leaves the elements uninitialized.
#define RSTL_VECTOR_UNINITIALIZED_COUNT_CTOR RSTL_ECHOES_ERA
// `vector(int, const T&)` is one constructor with a defaulted allocator argument.
#define RSTL_VECTOR_DEFAULT_ALLOC_FILL_CTOR (RSTL_VERSION >= RSTL_G2ME)
// `vector(int)` only reserves, with `mCount` assigned in the body.
#define RSTL_VECTOR_RESERVE_CTOR (RSTL_VERSION >= RSTL_R3ME_00)
// `vector::assign` and CStringExtras fill with `push_back_unsafe`.
#define RSTL_VECTOR_PUSH_BACK_UNSAFE (RSTL_VERSION >= RSTL_G2ME)
// `vector::push_back` appends without growing or checking capacity.
#define RSTL_VECTOR_UNCHECKED_PUSH_BACK (RSTL_VERSION >= RSTL_R3IJ)
// `vector::push_back` grows the storage, and is followed by `push_back_unsafe` and `pop_back`.
#define RSTL_VECTOR_GROWING_PUSH_BACK                                                              \
  (RSTL_VERSION >= RSTL_G2ME && !RSTL_VECTOR_UNCHECKED_PUSH_BACK)
// `insert_into` indexes the leading elements with a `long`.
#define RSTL_VECTOR_LONG_INSERT_INDEX (RSTL_VERSION >= RSTL_G2ME && RSTL_VERSION != RSTL_G2MEAB)
// The `list` copy constructor guards the partial copy with a `destroy_helper`.
#define RSTL_LIST_COPY_DESTROY_GUARD (RSTL_VERSION < RSTL_G2ME)
// `list::destroy` calls `clear()` instead of erasing the range.
#define RSTL_LIST_DESTROY_CALLS_CLEAR (RSTL_VERSION >= RSTL_GM8P_00)
// `list` iterators have real postfix `++` / `--` (copy, step, return the copy).
#define RSTL_LIST_POSTFIX_ITERATORS (RSTL_VERSION >= RSTL_G2ME)
// The `list` sentinel `mEmpty_prev` is a plain, unaligned pointer.
#define RSTL_LIST_PLAIN_SENTINEL (RSTL_VERSION >= RSTL_G2ME)
// `reserved_vector` uses `reserved_vector_traits` to skip destructor loops for trivial types and
// has `explicit reserved_vector(int)`.
#define RSTL_RESERVED_VECTOR_TRAITS (RSTL_VERSION >= RSTL_R3IJ)
// The `reserved_vector` copy constructor copies through raw pointers.
#define RSTL_RESERVED_VECTOR_RAW_COPY (RSTL_VERSION >= RSTL_G2ME)
// `reserved_vector::resize` also shrinks.
#define RSTL_RESERVED_VECTOR_SHRINKING_RESIZE (RSTL_VERSION >= RSTL_G2ME)
// `reserved_vector` destroys through `destroy_elements` and defines `operator=` out of class.
#define RSTL_RESERVED_VECTOR_OUT_OF_CLASS RSTL_ECHOES_ERA
// `bit_vector::make_room` grows with `push_back_unsafe`.
#define RSTL_BIT_VECTOR_UNSAFE_GROW (RSTL_VERSION >= RSTL_R3ME_00)

// --- Other containers ---
// The tree's non-type template parameter is bool in Prime and int in Echoes/Corruption.
// This must agree with the stream-constructor definitions in the consuming game's headers.
#if RSTL_PRIME1
#define RSTL_RBTREE_MULTI_TYPE bool
#else
#define RSTL_RBTREE_MULTI_TYPE int
#endif
// `optional_object` stores its value in an aligned `uchar` array followed by an aligned `bool`
// (otherwise a `uint` array and a plain `bool`).
#define RSTL_OPTIONAL_OBJECT_ALIGNED_STORAGE RSTL_ECHOES_ERA
// `~optional_object` destroys the value directly instead of calling `clear()`.
#define RSTL_OPTIONAL_DTOR_DESTROYS (RSTL_VERSION >= RSTL_GM8P_00)
// The `set` copy constructor is `inline`.
#define RSTL_SET_INLINE_COPY_CTOR (RSTL_VERSION < RSTL_GM8P_00)
// `red_black_tree::find_node` returns through a ternary instead of a `noResult` flag.
#define RSTL_RBTREE_TERNARY_FIND (RSTL_VERSION >= RSTL_G2ME)
// `red_black_tree` has `destroy()` and a destroy-based destructor; otherwise
// copy/assign/destructor go through `clear()`.
#define RSTL_RBTREE_DESTROY_HELPER (!RSTL_ECHOES_ERA)

// --- Strings ---
// `basic_string::mNull` is a non-const static.
#define RSTL_STRING_MUTABLE_NULL (RSTL_VERSION >= RSTL_G2ME)
// The string typedefs and comparison operators are declared before the out-of-line specializations,
// and `substr` is defined out of line.
#define RSTL_STRING_PREDECLARED_SPECIALIZATIONS (RSTL_VERSION >= RSTL_G2ME)
// `operator+(const string&, const string&)` is inline instead of declared out of line.
#define RSTL_STRING_INLINE_CONCAT RSTL_ECHOES_ERA
// `basic_string` reads characters with `ReadInt8()` instead of `Get< schar >()`.
#define RSTL_STRING_READ_INT8 (RSTL_VERSION >= RSTL_G2ME)
// Releasing the copy-on-write buffer calls `CMemory::Free` instead of
// `rmemory_allocator::deallocate`.
#define RSTL_STRING_FREE_WITH_CMEMORY (RSTL_VERSION >= RSTL_G2ME)
// rstl_strings.cpp specializes `basic_string< char >::assign(const char*, int)`.
#define RSTL_STRING_ASSIGN_SPECIALIZATION (RSTL_VERSION >= RSTL_GM8P_00)
// CStringExtras reads characters with `at()` and a `signed char` digit buffer, and
// declares the uppercase character parameter without top-level const (MWCC codegen).
#define RSTL_STRING_EXTRAS_AT_ACCESS (RSTL_VERSION >= RSTL_G2ME)
// Emitting the unused factory also changes the string pool and existing conversion code
// in PAL/Japanese Prime, so this is an emission boundary rather than just API availability.
#define RSTL_STRING_LIST_FACTORY (RSTL_VERSION >= RSTL_G2ME)
// `ConvertToUNICODE` returns early for an empty string.
#define RSTL_UNICODE_EMPTY_EARLY_RETURN (RSTL_VERSION >= RSTL_G2ME)
// `CStringExtras::CreateFromReal` exists.
#define RSTL_STRING_CREATE_FROM_REAL (RSTL_VERSION < RSTL_G2ME)

// --- Corruption prototype differences ---
// Sharing Echoes' allocation expression (with a size local) outlines the
// prototype's CEditorToGeometryMap fill constructor, so keep this source form.
#define RSTL_ALLOCATOR_LEGACY_SPELLING (RSTL_VERSION == RSTL_G2MEAB)
// `aligned_allocator::allocate(T*&, int)` calls `CMemory::Alloc` directly and there is no
// `aligned_allocator::allocate(int)`.
#define RSTL_ALIGNED_ALLOCATE_INLINE (RSTL_PRIME1 || RSTL_VERSION == RSTL_G2MEAB)
// `locked_cache_allocator` takes a plain `int` mode (no `EMode`).
#define RSTL_LOCKED_CACHE_RAW_MODE (RSTL_VERSION == RSTL_G2MEAB)
// `pair< int, float >` is explicitly kept non-trivial for the construction policies.
#define RSTL_PAIR_INT_FLOAT_POLICY (RSTL_VERSION == RSTL_G2MEAB)
// `const_linear_iterator` builds its `+`/`-` results directly (and `operator-` returns `long`).
#define RSTL_LINEAR_ITERATOR_DIRECT_ARITHMETIC (RSTL_VERSION == RSTL_G2MEAB)
// `ownership_transfer::take_ownership` returns null when it does not own the object.
#define RSTL_OWNERSHIP_TRANSFER_CHECKED_TAKE (RSTL_VERSION == RSTL_G2MEAB)
// construct.hpp declares the pointer specializations of `is_trivially_destructible` and
// `use_assignment_for_construction` right after the primary templates, has `is_scalar_type`, and
// declares `uint` and `u64` as trivially constructible.
#define RSTL_SCALAR_TYPE_TRAITS (RSTL_VERSION == RSTL_G2MEAB)
// `range_destruction_checks` and the destruction loop that consults it exist.
#define RSTL_RANGE_DESTRUCTION_CHECKS (!RSTL_PRIME1 && RSTL_VERSION != RSTL_G2MEAB)
// `construct` also has an overload taking a mutable `T&`, and the placement-new policy has no
// null precondition.
#define RSTL_CONSTRUCT_MUTABLE_OVERLOAD (RSTL_VERSION == RSTL_G2MEAB)
// A non-inline declaration before the inline const construct definition gives MWCC's
// prototype instantiations external weak linkage, needed by native callers.
#define RSTL_CONSTRUCT_EXTERN_DECL (RSTL_VERSION == RSTL_G2MEAB)
// `destroy(T*)` and `destroy(begin, end)` forward to `destroy_impl` (which holds the trivial
// destructor test and the range precondition) instead of `destruction_policy`.
#define RSTL_DESTROY_VIA_IMPL (RSTL_VERSION == RSTL_G2MEAB)
// `uninitialized_copy_n` starts with the `n >= 0` precondition.
#define RSTL_COPY_N_PRECONDITION (RSTL_VERSION == RSTL_G2MEAB)
// `find_if` takes its predicate by value.
#define RSTL_FIND_IF_PRED_BY_VALUE (RSTL_VERSION == RSTL_G2MEAB)
// `swap` and `iter_swap` are inline only when the including file defines `RSTL_INLINE_SWAP`, and
// `swap` has no precondition.
#define RSTL_SWAP_OPTIONAL_INLINE (RSTL_VERSION == RSTL_G2MEAB)
// The comparison sorts keep the comparison result in a bool local. Sharing this
// with the prototype changes CGuiFrame's emitted sort helpers.
#define RSTL_SORT_LESS_LOCAL (RSTL_VERSION != RSTL_G2MEAB)
// `lower_bound` with a comparator comes right after the comparator `sort`, before the `__sort3`
// without one (otherwise it follows the plain `lower_bound`).
#define RSTL_LOWER_BOUND_CMP_BEFORE_SORT3 (RSTL_VERSION == RSTL_G2MEAB)
// The plain `lower_bound` declares `halfDist` and `it` inside its loop.
#define RSTL_LOWER_BOUND_LOOP_LOCALS (RSTL_VERSION == RSTL_G2MEAB)
// Strings and their source files.
// Prototype `char_traits::assign(T*, int, T)` takes its value by value.
#define RSTL_STRING_TRAITS_VALUE_PARAM (RSTL_VERSION == RSTL_G2MEAB)
// The single-character string constructor accepts an allocator argument.
#define RSTL_STRING_CHAR_CTOR_ALLOCATOR (RSTL_VERSION == RSTL_G2MEAB)
// Non-const indexing detaches the COW buffer. This changes existing calls, unlike the
// additional string methods, so retain the prototype's writable overload separately.
#define RSTL_STRING_WRITABLE_INDEX (RSTL_VERSION == RSTL_G2MEAB)
// The `literal_t` constructor measures with a plain `end` pointer loop.
#define RSTL_STRING_SIMPLE_LITERAL_CTOR (RSTL_VERSION == RSTL_G2MEAB)
// The iterator-range constructor terminates through `Traits::eos()` instead of `0`.
#if RSTL_VERSION == RSTL_G2MEAB
#define RSTL_STRING_RANGE_CTOR_EOS Traits::eos()
#else
#define RSTL_STRING_RANGE_CTOR_EOS 0
#endif
// `~basic_string` carries the (empty) pointer precondition.
#define RSTL_STRING_DTOR_PRECONDITION (RSTL_PRECONDITION_MACRO && RSTL_VERSION != RSTL_G2MEAB)
// `internal_search` is declared `static inline` in the class.
#define RSTL_STRING_INLINE_SEARCH_DECL (RSTL_STRING_OUT_OF_CLASS && RSTL_VERSION != RSTL_G2MEAB)
// `internal_search` walks a `match` and a `search` iterator in step (a third implementation).
// Only this implementation supports the reverse iterators used by `rfind`.
#define RSTL_STRING_MATCH_LOOP_SEARCH (RSTL_VERSION == RSTL_G2MEAB)
// `compare(const _CharTp*, int)` measures with a separate `rhsEnd` pointer (otherwise it advances
// `rhs` itself and keeps `rhsStart`).
#define RSTL_STRING_COMPARE_END_POINTER (RSTL_STRING_OUT_OF_CLASS && RSTL_VERSION != RSTL_G2MEAB)
// The string typedefs, specializations and `operator+` overloads come in the prototype's own
// order.
#define RSTL_STRING_PROTO_TAIL (RSTL_VERSION == RSTL_G2MEAB)
// rstl_strings.cpp defines the case-insensitive string specializations first and the `*_l`
// helpers last, and `range_iterator` before `position_iterator`.
#define RSTL_STRINGS_PROTO_LAYOUT (RSTL_VERSION == RSTL_G2MEAB)
// The stream constructor reads each character into an `int` (otherwise a `char`).
#if RSTL_VERSION == RSTL_G2MEAB
#define RSTL_STRING_STREAM_CH_TYPE int
#else
#define RSTL_STRING_STREAM_CH_TYPE char
#endif
// In CStringExtras the source file for the prototype is RstlExtras.cpp (CStringExtras.cpp is empty).
#define RSTL_STRING_EXTRAS_IN_RSTL_EXTRAS (RSTL_VERSION == RSTL_G2MEAB)
// StringExtras has `ConvertToLowerCase(char)`, `ConvertToInteger`, `ConvertToReal`, `Format` and
// names the `CreateFromReal` parameter `precision`.
#define RSTL_STRING_EXTRAS_PROTO_API (RSTL_VERSION == RSTL_G2MEAB)
// Vector, list and related containers.
// `reserved_vector_fill_type` exists, and `resize` takes it (shrinking with pointer-range
// `destroy`), followed by `resize_uninitialized`.
#define RSTL_RESERVED_VECTOR_FILL_TYPE (RSTL_VERSION == RSTL_G2MEAB)

// `vector` passes bool constructor fill values directly; class values remain references.
#define RSTL_VECTOR_FILL_TYPE (RSTL_VERSION == RSTL_G2MEAB)
// Byte-vector resize takes a signed-char fill value, rather than its address.
#define RSTL_VECTOR_RESIZE_TYPE (RSTL_VERSION == RSTL_G2MEAB)
// `reserved_vector` storage is an unaligned `uchar` array.
#define RSTL_RESERVED_VECTOR_ALIGNED_DATA (RSTL_VERSION != RSTL_G2MEAB)
// The copy constructor placement-constructs each element through `construction_policy`.
#define RSTL_RESERVED_VECTOR_POLICY_COPY (RSTL_VERSION == RSTL_G2MEAB)
// The const `operator[]` carries the index precondition.
#define RSTL_RESERVED_VECTOR_CONST_INDEX_PRECONDITION                                                (RSTL_PRECONDITION_MACRO && RSTL_VERSION != RSTL_G2MEAB)
// `destroy_elements` takes a plain `T*` and destroys `&ptr[i]` (the others use `T* const` and
// `destroy(ptr + i)`).
#define RSTL_RESERVED_VECTOR_DESTROY_INDEXED (RSTL_PRIME1 || RSTL_VERSION == RSTL_G2MEAB)
// The out-of-line `operator=` is `inline` unless the translation unit defines
// `RSTL_DONT_INLINE_RESERVED_VECTOR`.
#define RSTL_RESERVED_VECTOR_DONT_INLINE_OPTION (RSTL_VERSION == RSTL_G2MEAB)
// `listpool` is the full reconstructed container instead of the declaration-only stub.
#define RSTL_LISTPOOL_FULL (RSTL_VERSION == RSTL_G2MEAB)
// `list::node` is a plain `uchar` item array without alignment or constructor, and
// `do_insert_before` reads `n->mPrev` directly.
#define RSTL_LIST_NODE_PLAIN (RSTL_VERSION == RSTL_G2MEAB)
// `create_node` and the out-of-class `erase(start, end)` carry the (empty) precondition statements.
#define RSTL_LIST_PRECONDITIONS (RSTL_VERSION != RSTL_G2MEAB)
// `list::remove_if` is defined in the class body (over nodes) even though the rest is out of class.
#define RSTL_LIST_REMOVE_IF_IN_CLASS (RSTL_VERSION == RSTL_G2MEAB)
// `list::exchange` is a plain (not `inline`) member that links nodes through `mPrev` / `mNext`.
#define RSTL_LIST_EXCHANGE_DIRECT (RSTL_VERSION == RSTL_G2MEAB)
// `~list` calls the element destructor directly instead of `rstl::destroy`.
#define RSTL_LIST_DIRECT_DTOR_CALL (RSTL_VERSION == RSTL_G2MEAB)
// Pointer elements are passed by value to list insertion helpers.
#define RSTL_LIST_POINTER_VALUE_PARAM (RSTL_VERSION == RSTL_G2MEAB)
// `bit_vector` is the reduced class with `reference` (word iterator plus `uint` mask), an
// out-of-line `at` / `operator[]`, and no iterators or growth.
#define RSTL_BIT_VECTOR_COMPACT (RSTL_VERSION == RSTL_G2MEAB)
// `vector(int)` reserves when count > 0. Testing count > mCount changes the
// prototype's CPFPointSearchState constructor; keep its existing form.
#define RSTL_VECTOR_COUNT_CTOR_POSITIVE_CHECK (RSTL_VERSION == RSTL_G2MEAB)
// `vector::resize_uninitialized` compares against `mCount` (in a braced block).
#define RSTL_VECTOR_RESIZE_UNINITIALIZED_COUNT_CHECK (RSTL_VERSION == RSTL_G2MEAB)
// `vector::assign`, `insert` and `erase` are declared/defined `inline`.
#define RSTL_VECTOR_INLINE_DECLS (RSTL_VERSION != RSTL_G2MEAB)
// `vector::push_back` asserts through `verify_capacity_failed` instead of growing (and
// `push_back_unsafe` checks too, with a `push_back_checked` wrapper).
#define RSTL_VECTOR_VERIFIED_PUSH_BACK (RSTL_VERSION == RSTL_G2MEAB)
// Trees, pointers and optional values.
// `red_black_tree` declares `tree_key_argument` (`const T&`, but `uint` by value) and takes its
// lookup keys through it.
#define RSTL_RBTREE_KEY_ARGUMENT_TRAITS (RSTL_VERSION == RSTL_G2MEAB)
#if RSTL_RBTREE_KEY_ARGUMENT_TRAITS
#define RSTL_RBTREE_KEY_ARGUMENT typename tree_key_argument< T >::type
#else
#define RSTL_RBTREE_KEY_ARGUMENT const T&
#endif
// The `red_black_tree` iterators take a mandatory third `bool` constructor argument (always passed
// as `false`), `const_iterator::value_type` is `P*`, and `iterator::operator->` / `operator*` are
// non-const.
#define RSTL_RBTREE_PROTO_ITERATORS (RSTL_VERSION == RSTL_G2MEAB)
#if RSTL_RBTREE_PROTO_ITERATORS
#define RSTL_RBTREE_ITERATOR_FLAG , false
#else
#define RSTL_RBTREE_ITERATOR_FLAG
#endif
// `red_black_tree::node::mValue` is a plain `uchar` array without `ALIGNAS`.
#define RSTL_RBTREE_NODE_ALIGNED (RSTL_VERSION != RSTL_G2MEAB)
// `find_node` returns the ternary directly instead of through a `result` local.
#define RSTL_RBTREE_DIRECT_FIND_RETURN (RSTL_VERSION == RSTL_G2MEAB)
// `create_node` carries the (empty) allocation precondition.
#define RSTL_RBTREE_CREATE_PRECONDITION (RSTL_VERSION != RSTL_G2MEAB)
// The out-of-line members are defined in the prototype's own order (`copy_from`,
// `free_node_and_sub_nodes`, `insert_into`), reach the children through `get_left()` /
// `get_right()` and name the multi-flag template parameter `U`.
#define RSTL_RBTREE_PROTO_OUT_OF_LINE (RSTL_VERSION == RSTL_G2MEAB)
// `set` wraps a `red_black_tree` member (`inner`) and forwards to it, instead of deriving from it.
#define RSTL_SET_INNER_TREE (RSTL_VERSION == RSTL_G2MEAB)
// `rc_ptr` allocates its count with `rs_new_line_in("rc_ptr.h", 87)` (constructor) and
// `rs_new_line_in("rc_ptr.h", 310)` (`Assign`), the lines of the prototype's original header.
#if RSTL_VERSION == RSTL_G2MEAB
#define RSTL_RC_PTR_NEW_COUNT_CTOR rs_new_line_in("rc_ptr.h", 87)
#define RSTL_RC_PTR_NEW_COUNT_ASSIGN rs_new_line_in("rc_ptr.h", 310)
#else
#define RSTL_RC_PTR_NEW_COUNT_CTOR rs_new
#define RSTL_RC_PTR_NEW_COUNT_ASSIGN rs_new
#endif
// `~rc_ptr` carries the (empty) count precondition.
#define RSTL_RC_PTR_DTOR_PRECONDITION (RSTL_VERSION != RSTL_G2MEAB)
// `rc_ptr` accessors go through `GetPtr()` and `ReleaseData` deletes `mRefCount` directly.
#define RSTL_RC_PTR_GET_PTR_ACCESS (RSTL_PRIME1 || RSTL_VERSION == RSTL_G2MEAB)
// `ncrc_ptr` has an explicit copy constructor (with preconditions).
#define RSTL_NCRC_PTR_COPY_CTOR (RSTL_RC_PTR_PRIVATE_COUNT && RSTL_VERSION != RSTL_G2MEAB)
// `single_ptr` has the `mutable` pointer and the const-ref ownership-transferring copy constructor
// (Prime 1 and the prototype).
#define RSTL_SINGLE_PTR_MUTABLE ((RSTL_PRIME1 || RSTL_VERSION == RSTL_G2MEAB) && RSTL_SINGLE_PTR_TRANSFER)
// `single_ptr::operator->` returns a plain `T*` and the const `operator*` returns `const T&`.
#define RSTL_SINGLE_PTR_PLAIN_ACCESS (RSTL_VERSION == RSTL_G2MEAB)
// Keep the accessor in the prototype: direct member access inlines away native
// CSpawnSystemKeyframeInfo copy/construction helpers (GC/2.7).
#define RSTL_OPTIONAL_COPY_READS_VALID_CALL (RSTL_VERSION == RSTL_G2MEAB)
// `optional_object::operator=` inlines `assign` into its body.
#define RSTL_OPTIONAL_ASSIGN_INLINE (RSTL_VERSION == RSTL_G2MEAB)
// `optional_object` stores a plain `uchar` array followed by `bool m_valid ATTRIBUTE_ALIGN(4)`.
#define RSTL_OPTIONAL_OBJECT_PLAIN_STORAGE (RSTL_VERSION == RSTL_G2MEAB)
// `optional_storage::m_data` carries `ALIGNAS(T)`.
#define RSTL_OPTIONAL_STORAGE_ALIGNED (RSTL_VERSION != RSTL_G2MEAB)

#if defined(TARGET_PC)
#define RSTL_DELETE_PTR(TYPE, PTR) pointer_deleter< TYPE >::destroy(PTR)
#else
#define RSTL_DELETE_PTR(TYPE, PTR) delete PTR
#endif

#endif // _RSTL_VERSIONS_H
