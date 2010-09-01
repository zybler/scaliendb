#ifndef STORAGEKEYINDEX_H
#define STORAGEKEYINDEX_H

/*
===============================================================================

 StorageKeyIndex

===============================================================================
*/

class StorageKeyIndex
{
public:
	StorageKeyIndex();
	~StorageKeyIndex();
	
	void					SetKey(ReadBuffer& key, bool copy);

	ReadBuffer				key;
	Buffer*					keyBuffer;
	uint32_t				index;

	InTreeNode<StorageKeyIndex>	treeNode;

	static bool				LessThan(StorageKeyIndex &a, StorageKeyIndex &b);
};

inline bool LessThan(uint32_t a, uint32_t b)
{
	return (a < b);
}

#endif
