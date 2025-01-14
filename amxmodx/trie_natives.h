// vim: set ts=4 sw=4 tw=99 noet:
//
// AMX Mod X, based on AMX Mod by Aleksander Naszko ("OLO").
// Copyright (C) The AMX Mod X Development Team.
//
// This software is licensed under the GNU General Public License, version 3 or higher.
// Additional exceptions apply. For full license details, see LICENSE.txt or visit:
//     https://alliedmods.net/amxmodx-license

#ifndef _TRIE_NATIVES_H_
#define _TRIE_NATIVES_H_

#include "amxmodx.h"
#include <sm_stringhashmap.h>
#include <sm_memtable.h>
#include "natives_handles.h"
#include <amtl/am-uniqueptr.h>

enum EntryType
{
	EntryType_Cell,
	EntryType_CellArray,
	EntryType_String,
};

class Entry
{
	struct ArrayInfo
	{
		size_t length;
		size_t maxbytes;

		void *base() {
			return this + 1;
		}
	};

public:
	Entry()
		: control_(0)
	{
	}
	Entry(Entry &&other) noexcept
	{
		control_ = other.control_;
		data_ = other.data_;
		other.control_ = 0;
	}
	~Entry()
	{
		free(raw());
	}

	void setCell(cell value) {
		setType(EntryType_Cell);
		data_ = value;
	}
	void setArray(cell *cells, size_t length) {
		ArrayInfo *array = ensureArray(length * sizeof(cell));
		array->length = length;
		memcpy(array->base(), cells, length * sizeof(cell));
		setTypeAndPointer(EntryType_CellArray, array);
	}
	void setString(const char *str) {
		const size_t length = strlen(str);
		ArrayInfo *array = ensureArray(length + 1);
		array->length = length;
		strcpy(static_cast<char*>(array->base()), str);
		setTypeAndPointer(EntryType_String, array);
	}

	size_t arrayLength() const {
		assert(isArray());
		return raw()->length;
	}
	cell *array() const {
		assert(isArray());
		return static_cast<cell *>(raw()->base());
	}
	char *chars() const {
		assert(isString());
		return static_cast<char *>(raw()->base());
	}
	cell cell_() const {
		assert(isCell());
		return data_;
	}

	bool isCell() const {
		return type() == EntryType_Cell;
	}
	bool isArray() const {
		return type() == EntryType_CellArray;
	}
	bool isString() const {
		return type() == EntryType_String;
	}

private:
	Entry(const Entry &other) = delete;

	ArrayInfo *ensureArray(size_t bytes) {
		ArrayInfo *array = raw();
		if (array && array->maxbytes >= bytes)
			return array;
		array = (ArrayInfo *)realloc(array, bytes + sizeof(ArrayInfo));
		if (!array)
		{
			fprintf(stderr, "Out of memory!\n");
			abort();
		}
		array->maxbytes = bytes;
		return array;
	}

	// Pointer and type are overlaid, so we have some accessors.
	ArrayInfo *raw() const {
		return reinterpret_cast<ArrayInfo *>(control_ & ~uintptr_t(0x3));
	}
	void setType(EntryType aType) {
		control_ = uintptr_t(raw()) | uintptr_t(aType);
		assert(type() == aType);
	}
	void setTypeAndPointer(EntryType aType, ArrayInfo *ptr) {
		// malloc() should guarantee 8-byte alignment at worst
		assert((uintptr_t(ptr) & 0x3) == 0);
		control_ = uintptr_t(ptr) | uintptr_t(aType);
		assert(type() == aType);
	}
	EntryType type() const {
		return (EntryType)(control_ & 0x3);
	}

private:
	// Contains the bits for the type, and an array pointer, if one is set.
	uintptr_t control_;

	// Contains data for cell-only entries.
	cell data_;
};


template <typename T>
class StringHashMapCustom
{
	typedef StringHashMap<T> Internal;

public:
	StringHashMapCustom() : mod_count_(0)
	{}

public:
	typedef typename Internal::Result Result;
	typedef typename Internal::Insert Insert;
	typedef typename Internal::iterator iterator;

public:
	size_t elements() const
	{
		return internal_.elements();
	}

	iterator iter()
	{
		return internal_.iter();
	}

	bool contains(const char *aKey)
	{
		return internal_.contains(aKey);
	}

	bool replace(const char *aKey, const T &value)
	{
		return internal_.contains(aKey, value);
	}

	bool insert(const char *aKey, const T &value)
	{
		if (internal_.insert(aKey, value))
		{
			mod_count_++;
			return true;
		}
		return false;
	}

	bool remove(const char *aKey)
	{
		if (internal_.remove(aKey))
		{
			mod_count_++;
			return true;
		}
		return false;
	}

	void clear()
	{
		mod_count_++;
		internal_.clear();
	}

	void remove(Result &r)
	{
		mod_count_++;
		internal_.remove(r);
	}

	Result find(const char *aKey)
	{
		return internal_.find(aKey);
	}

	Insert findForAdd(const char *aKey)
	{
		return internal_.findForAdd(aKey);
	}

	bool add(Insert &i, const char *aKey)
	{
		if (internal_.add(i, aKey))
		{
			mod_count_++;
			return true;
		}
		return false;
	}

	bool add(Insert &i)
	{
		if (internal_.add(i))
		{
			mod_count_++;
			return true;
		}
		return false;
	}

public:
	size_t mod_count() const
	{
		return mod_count_;
	}
	
private:
	Internal internal_;
	size_t mod_count_;
};

struct CellTrie
{
	StringHashMapCustom<Entry> map;
};

struct CellTrieIter
{
	CellTrieIter(CellTrie *_trie) : trie(_trie), iter(_trie->map.iter()), mod_count(_trie->map.mod_count())
	{}

	CellTrie *trie;
	StringHashMapCustom<Entry>::iterator iter;
	size_t mod_count;
};

struct TrieSnapshot
{
	TrieSnapshot()
		: length(0), strings(128)
	{
	}

	size_t mem_usage()
	{
		return length * sizeof(int) + strings.GetMemTable()->GetMemUsage();
	}

	size_t length;
	ke::AutoPtr<int[]> keys;
	BaseStringTable strings;
};

extern NativeHandle<CellTrie> TrieHandles;
extern NativeHandle<CellTrieIter> TrieIterHandles;
extern NativeHandle<TrieSnapshot> TrieSnapshotHandles;
extern AMX_NATIVE_INFO trie_Natives[];

#endif

