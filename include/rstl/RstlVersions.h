#ifndef _RSTL_VERSIONS_H
#define _RSTL_VERSIONS_H

// Prime 1 GC
#define RSTL_GM8EAB_00 0
#define RSTL_GM8E_00 1
#define RSTL_GM8E_01 2
#define RSTL_GM8E_48 3
#define RSTL_GM8E_02 4
#define RSTL_GM8P_00 5
#define RSTL_GM8J_00 6

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
// RSTL_ECHOES_ERA is an internal helper: the GameCube Prime 2 versions (and the unbuilt Prime 3
// standalone versions between them and the Wii versions).
#define RSTL_ECHOES_ERA (RSTL_VERSION >= RSTL_G2ME && RSTL_VERSION < RSTL_R3IJ)

// --- Allocation ---
// Containers and strings derive privately from `Alloc`; otherwise they hold an `Alloc
// mAllocator` member.
#define RSTL_ALLOCATOR_BASE_CLASS (RSTL_VERSION >= RSTL_R3IJ)
// Containers allocate and free through the `mAllocator` instance instead of static `Alloc::` calls.
#define RSTL_ALLOCATOR_INSTANCE RSTL_ECHOES_ERA
// `rmemory_allocator::allocate(int)` is declared only and always called out of line (defined in
// RstlExtras.cpp), `deallocate` deletes without a null check, and `allocate2` exists.
#define RSTL_ALLOCATOR_EXTERN_ALLOCATE (RSTL_VERSION >= RSTL_G2ME)
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
#define RSTL_INDEX_PRECONDITION RSTL_ECHOES_ERA
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
// `prereserved_vector` is declared inside `namespace rstl`; the Prime 1 GameCube builds declare it
// in the global namespace.
#define RSTL_PRERESERVED_VECTOR_NAMESPACE (RSTL_VERSION >= RSTL_G2ME)
// `construct`, `destroy` and `destroy_impl` are external `inline` templates instead of `static
// inline` ones (the uninitialized_* helpers are `static inline` everywhere).
#define RSTL_EXTERN_CONSTRUCT_FUNCTIONS RSTL_ECHOES_ERA
// Pointers and `uint` are trivially destructible.
#define RSTL_TRIVIAL_POINTERS (RSTL_ECHOES_ERA || RSTL_VERSION >= RSTL_R3ME_00)
// The arithmetic types are declared trivially destructible and copied by assignment, and so are
// pointers. Elsewhere only the types above skip destructor loops; everything else is copied with
// a placement new.
#define RSTL_TRIVIAL_SCALARS RSTL_ECHOES_ERA
// `construct< int >` stores directly.
#define RSTL_DIRECT_INT_CONSTRUCT (RSTL_ECHOES_ERA || RSTL_VERSION >= RSTL_R3ME_00)
// The direct `destroy(T*)` takes a `T* const` (Echoes' takes a plain pointer).
#define RSTL_DESTROY_CONST_PARAM (RSTL_VERSION >= RSTL_GM8P_00 && !RSTL_ECHOES_ERA)
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
// `uninitialized_fill_n` fills through `construct` instead of a placement new.
#define RSTL_FILL_USES_CONSTRUCT RSTL_ECHOES_ERA
// `uninitialized_fill_n` advances its `dest` parameter itself instead of a copy, and
// `bit_vector::insert` uses counted loops.
#define RSTL_COUNTED_FILL_LOOPS (RSTL_VERSION >= RSTL_R3ME_00)
// `construct` has direct overloads for `uint`, `bool`, `float` and pointers, and
// `reserved_vector_traits` has fill/push fast paths for scalar elements.
#define RSTL_SCALAR_TRAITS (RSTL_VERSION >= RSTL_R3ME_00)

// `auto_ptr` has a converting copy constructor from `auto_ptr< U >` (the GameCube compiler rejects
// its template friend declaration).
#define RSTL_AUTO_PTR_CONVERSION (RSTL_VERSION >= RSTL_R3IJ)
// `auto_ptr::release` returns null when it does not own the item.
#define RSTL_AUTO_PTR_CHECKED_RELEASE (RSTL_VERSION >= RSTL_R3IJ)
// `single_ptr` has a const-ref copy constructor that transfers ownership.
#define RSTL_SINGLE_PTR_TRANSFER (RSTL_VERSION >= RSTL_G2ME)
// `unk_singleptr` is `single_ptr< void >` (the Wii compiler rejects `void&` from `operator*`).
#define RSTL_UNK_SINGLE_PTR_VOID RSTL_ECHOES_ERA
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
#define RSTL_INLINE_ADVANCE_ITERATOR (RSTL_VERSION >= RSTL_G2ME)
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
#define RSTL_VECTOR_INSERT_CONST_ITEM (RSTL_VERSION >= RSTL_GM8P_00 && RSTL_VERSION < RSTL_G2ME)
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
#define RSTL_VECTOR_LONG_INSERT_INDEX (RSTL_VERSION >= RSTL_G2ME)
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
// Adds `find(const char*)`, `compare(const char*)`, `string_l`, the wide `position_iterator` and
// the case-insensitive `assign`.
#define RSTL_STRING_EXTRA_API (RSTL_VERSION >= RSTL_G2ME)
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
// CStringExtras reads characters with `at()` and a `signed char` digit buffer.
#define RSTL_STRING_EXTRAS_AT_ACCESS (RSTL_VERSION >= RSTL_G2ME)
// CStringExtras has the `ConvertToLowerCase` and `ConvertToUpperCase(char)` overloads.
#define RSTL_STRING_CASE_CONVERSION_API (RSTL_VERSION >= RSTL_G2ME)
// CStringExtras.cpp defines `FStringListFactory`.
#define RSTL_STRING_LIST_FACTORY (RSTL_VERSION >= RSTL_G2ME)
// `ConvertToUNICODE` returns early for an empty string.
#define RSTL_UNICODE_EMPTY_EARLY_RETURN (RSTL_VERSION >= RSTL_G2ME)
// `CStringExtras::CreateFromReal` exists.
#define RSTL_STRING_CREATE_FROM_REAL (RSTL_VERSION < RSTL_G2ME)

#endif // _RSTL_VERSIONS_H
