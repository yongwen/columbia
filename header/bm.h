/* BM.H - Catalog.  Bill's memory manager
Code given to us by Bill McKenna to manage memory.
Turned on by defining USE_MEMORY_MANAGER
$Revision: 4 $
Columbia Optimizer Framework

  A Joint Research Project of Portland State University 
  and the Oregon Graduate Institute
  Directed by Leonard Shapiro and David Maier
  Supported by NSF Grants IRI-9610013 and IRI-9619977
*/

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "stdafx.h"

// Initial size of array of BLOCK_ANCHORs. Use one array per MEM_MGR class.
#define INIT_BLOCK_ANCHOR_COUNT 	20

// initial value of the factor of realloc block anchor each time
#define	DEFAULT_REALLOC_FACTOR			2

// Initial size of slots per anchor block
#define DEFAULT_SLOTS_PER_BLOCK		10000

// Anchor for a set of memory blocks for elements of a particular size.
// Only a MEMORY_MANAGER object can create these.
//##ModelId=3B0C087801BA
class BLOCK_ANCHOR
{
	
	friend class MEMORY_MANAGER ;
	
protected:
	
	// The anchor manages a list of blocks of memory.  
	
	// The first block of memory is:
	//##ModelId=3B0C087801CE
	void * 	_first_block ;
	
	// Each block of memory contains a next_block_ptr, then
	// _slots_per_block slots, each _slot_size long
	//##ModelId=3B0C087801D8
	int     _first_slot_offset ;// == sizeof(_next_block_ptr)
	//##ModelId=3B0C087801EC
	int     _slots_per_block ;	//set by constructor
	//##ModelId=3B0C087801F6
	int     _slot_size ;		//set by constructor
	//##ModelId=3B0C08780200
	int     _next_block_ptr_size ; 
	//##ModelId=3B0C08780214
	int     _next_block_offset;  //offset within the block of next_block_ptr
	
	// These values are cached for rapid access.
	//##ModelId=3B0C0878021E
	void *  _free_slot;	// First free slot in any block
	//##ModelId=3B0C08780232
	void *  _last_slot;	// Last free slot in any block
	//##ModelId=3B0C0878023C
	int     _last_slot_index; // index in a block, of the last free slot
	//##ModelId=3B0C08780250
	int     _data_offset ;	  // not used?  always null?
	//##ModelId=3B0C0878025A
	int     _next_free_slot_offset ;  // unused?  always 0??
	//##ModelId=3B0C08780264
	int     _block_size ;   // space for ptr + slots_per_block slots
	
	// memory statistics
	//##ModelId=3B0C08780278
	int		NewCalls;		// new operation calls to manager
	//##ModelId=3B0C08780282
	int		DeleteCalls;	// delete operation calls to manager
	//##ModelId=3B0C08780296
	int		ActualAlloc;	// actual allocations
	
	// Constructor to set up one anchor, with NULL list
	//##ModelId=3B0C087802A1
	BLOCK_ANCHOR(int slot_size, int slots_per_block) ;
	//##ModelId=3B0C087802AC
	void	init(int slot_size, int slots_per_block) ;
	
	
	//##ModelId=3B0C087802B7
	~BLOCK_ANCHOR() ;
	
} ; 



//##ModelId=3B0C0878030F
class MEMORY_MANAGER
{
private:
	// Pointer to array of BLOCK_ANCHORs.
	//##ModelId=3B0C08780324
	BLOCK_ANCHOR *	_block_anchors ;
	
	// Number of currently existant block anchors.
	//##ModelId=3B0C08780337
	int		_block_anchor_count ;
	
	// Number of block anchors which have been initialized.
	//##ModelId=3B0C08780341
	int		_allocated_block_anchor_count ;
	
	// If all existant anchors have been initialized, create this many more
	//##ModelId=3B0C08780355
	int		_realloc_factor ;
	
	
	// Allocate an anchor for element size `elem_size' and 
	// data slots per block `slots_per_block'.
	//##ModelId=3B0C08780369
	BLOCK_ANCHOR *	allocate_anchor( int elem_size, int slots_per_block) ;
	
	// Allocate a block of elements.
	//##ModelId=3B0C08780387
	void		allocate_block(BLOCK_ANCHOR&) ;
	
public:
	//##ModelId=3B0C08780391
	MEMORY_MANAGER(int block_anchor_count=INIT_BLOCK_ANCHOR_COUNT,
		int realloc_factor=DEFAULT_REALLOC_FACTOR) ;
	
	//##ModelId=3B0C0878039B
	~MEMORY_MANAGER() ;
	
	
	// get internal values
	//##ModelId=3B0C0878039C
	int	block_anchor_count() { return _block_anchor_count ;}
	
	//##ModelId=3B0C087803A5
	int	allocated_block_anchor_count()
	{ return _allocated_block_anchor_count ;}
	
	//##ModelId=3B0C087803A6
	int	realloc_factor() { return _realloc_factor ;}
	
	// print out memory statistics
	//##ModelId=3B0C087803AF
	CString Dump();
	
	// Allocate an element. 
	// The first time this is called for a class, *anchor_v should be NULL. 
	// The memory manager instance will allocate one and pass it back.
	//##ModelId=3B0C087803B0
	void *	allocate(BLOCK_ANCHOR** anchor_v, int elem_size,
		int slots_per_block=DEFAULT_SLOTS_PER_BLOCK) ;
	
	// Deallocate an element.
	//##ModelId=3B0C087803C3
	void	deallocate(BLOCK_ANCHOR* anchor_p, void * data_p) ;	
} ;

#endif
