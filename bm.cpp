//$Revision: 6 $
//Implements classes in bm.h

#include "stdafx.h"	// This must be before the #ifdef USE..., else the #ifdef will be ignored
                    // since the precompiler will search for the PCH first.

#ifdef USE_MEMORY_MANAGER


//We will use memory management for classes in these files
#include "tasks.h"
#include "physop.h"

#ifdef _DEBUG
	#define new DEBUG_NEW
#endif

//********  MEMORY_MANAGER *************
//##ModelId=3B0C08780391
MEMORY_MANAGER::MEMORY_MANAGER(int block_anchor_count, int realloc_factor)
{
#if SAFETY>=1
    assert(realloc_factor > 1) ;
    assert(block_anchor_count > 0) ;
#endif
	
    _block_anchors
		= (BLOCK_ANCHOR *) malloc(block_anchor_count 
		* sizeof(BLOCK_ANCHOR)) ;
    assert(_block_anchors != NULL) ;
    _allocated_block_anchor_count = block_anchor_count ;
    _block_anchor_count = 0 ;
    _realloc_factor = realloc_factor ;
	
#ifdef USE_MEMORY_MANAGER
	APPLY_RULE::_anchor = NULL ; 
	ATTR::_anchor = NULL ; 
	BINDERY::_anchor = NULL ; 
	COST::_anchor = NULL ; 
	EQJOIN::_anchor = NULL ; 
	EXPR::_anchor = NULL ; 
	GROUP::_anchor = NULL ; 
	LEAF_OP::_anchor = NULL ; 
	LOG_COLL_PROP::_anchor = NULL ; 
	LOOPS_JOIN::_anchor = NULL ; 
	M_EXPR::_anchor = NULL ; 
	O_EXPR::_anchor = NULL ; 
	O_INPUTS::_anchor = NULL ; 
	SCHEMA::_anchor = NULL ; 
#endif
}

//##ModelId=3B0C0878039B
MEMORY_MANAGER::~MEMORY_MANAGER()
{
    for (int i = 0 ; i < _block_anchor_count ; ++i)
    {
		BLOCK_ANCHOR * anchor_p ;
		
		anchor_p = &(_block_anchors[i]) ;
		anchor_p -> BLOCK_ANCHOR::~BLOCK_ANCHOR() ;
    }
	
    if (_block_anchors != NULL)
    {
		free (_block_anchors) ;
		_block_anchors = NULL ;
    }
	
    _allocated_block_anchor_count = _block_anchor_count = 0 ;
}

//##ModelId=3B0C08780387
void MEMORY_MANAGER::allocate_block(BLOCK_ANCHOR& anchor)
{
    void *      tmp ;
	
    //malloc the memory needed for the block
    tmp = (void *) malloc (anchor._block_size) ;
    if (!tmp) assert(false) ;
	
	anchor.ActualAlloc ++ ;
	
    // Set pointer to next block for this anchor
    * (void **) 
        (  ((unsigned long) tmp)
		+ (unsigned long) (anchor._next_block_offset)
        )
		= anchor._first_block ;
    anchor._first_block = tmp ;
	
    anchor._last_slot_index = 0 ;
    anchor._last_slot 
		= (void *) ((unsigned long) tmp 
		+ (unsigned long) anchor._first_slot_offset) ;
}

//##ModelId=3B0C087803B0
void * MEMORY_MANAGER::allocate(BLOCK_ANCHOR** anchor_v, int elem_size,
								int slots_per_block)
{
    assert(anchor_v != NULL) ;
    assert(elem_size > 0) ;
    assert(slots_per_block > 0) ;
	
    void * ret_value ;
#if SAFETY>=1
    ret_value = 0 ;
#endif
	
    BLOCK_ANCHOR * anchor_p = *anchor_v ;
	
    // If called with a null anchor, then allocate a new one
    if (!anchor_p)
    {
		anchor_p = allocate_anchor(elem_size, slots_per_block) ;
		*anchor_v = anchor_p ;
    }
	
	anchor_p -> NewCalls ++;
	
    if (anchor_p -> _free_slot != NULL)
    {
        void            *free_slot = anchor_p -> _free_slot ;
		
        ret_value = free_slot ;
        anchor_p -> _free_slot = * (void **) (free_slot) ;
    }
    else
    {
        if (anchor_p -> _last_slot_index == anchor_p -> _slots_per_block)
        {
            this -> allocate_block(*anchor_p) ;
        }
		
        ret_value = anchor_p -> _last_slot ;
        ++ (anchor_p -> _last_slot_index) ;
        (anchor_p -> _last_slot) 
			= (void *) ((unsigned long) (anchor_p -> _last_slot)
			+ (unsigned long) (anchor_p -> _slot_size)) ;
    }
	
#if SAFETY>=1
    assert(ret_value != NULL) ;
#endif
	
    return ret_value ;
}

//##ModelId=3B0C087803C3
void MEMORY_MANAGER::deallocate(BLOCK_ANCHOR * anchor_p, void * data_p)
{
	anchor_p-> DeleteCalls ++;
	
    * (void **) data_p = anchor_p -> _free_slot ;
	
    anchor_p -> _free_slot = data_p ;
}

//##ModelId=3B0C08780369
BLOCK_ANCHOR * MEMORY_MANAGER::allocate_anchor(int elem_size, 
											   int slots_per_block)
											   
{
    if (_block_anchor_count == _allocated_block_anchor_count)
    {
		_allocated_block_anchor_count 
			= _realloc_factor * _allocated_block_anchor_count ;
		void * temp_ptr 
			= (void *) realloc ((char *)_block_anchors,
			(unsigned int) (_allocated_block_anchor_count 
			* (sizeof (BLOCK_ANCHOR)))) ;
		assert(temp_ptr != NULL) ;
		_block_anchors = (BLOCK_ANCHOR *) temp_ptr ;
    }
	
    BLOCK_ANCHOR * anchor = &_block_anchors[_block_anchor_count] ;
    ++_block_anchor_count ;
    anchor -> init(elem_size, slots_per_block) ;
    return anchor ;
}

// print out memory statistics
//##ModelId=3B0C087803AF
CString MEMORY_MANAGER::Dump()
{
	CString os,temp;
	
	os.Format("Allocated block anchor : %d\r\n", _block_anchor_count) ;
	
    for (int i = 0 ; i < _block_anchor_count ; i++)
    {
		BLOCK_ANCHOR * anchor_p =  &(_block_anchors[i]) ;
		temp.Format("Anchor %d --- slot_size: %d, block_size: %d, NewCalls: %d, DeleteCalls: %d, ActualAlloc: %d\r\n",
			i, anchor_p->_slot_size, anchor_p->_block_size, anchor_p->NewCalls, anchor_p->DeleteCalls, anchor_p->ActualAlloc);
		os += temp;
    }
	
	return os;
}

//********  BLOCK_ANCHOR *************

//##ModelId=3B0C087802A1
BLOCK_ANCHOR::BLOCK_ANCHOR(int slot_size, int slots_per_block)
{
    this -> init(slot_size, slots_per_block) ;
}


//##ModelId=3B0C087802B7
BLOCK_ANCHOR::~BLOCK_ANCHOR()
{
    void * block_p = _first_block ;
	
    while (block_p)
    {
		void * temp_block_p ;
		temp_block_p 
			= * (void **) (((unsigned long) block_p)
			+ (unsigned long) (_next_block_offset)) ;
		
		free (block_p) ;
		
		block_p = temp_block_p ;
    }
}

//##ModelId=3B0C087802AC
void BLOCK_ANCHOR::init(int slot_size, int slots_per_block)
{
    _first_block = NULL ;
    _first_slot_offset = (int) sizeof(void *)  ;
    _slots_per_block = slots_per_block ;
    _slot_size = slot_size ;
    _next_block_ptr_size = (int) sizeof(void *) ;
    _block_size = (int) sizeof (void *) + slots_per_block * slot_size ;
    _free_slot = NULL ;
    _last_slot = NULL ;
    _last_slot_index = slots_per_block ;
    _data_offset = 0 ; 
    _next_free_slot_offset = 0 ;
    _next_block_offset = 0 ;
	ActualAlloc = 0;
	DeleteCalls = 0;
	NewCalls = 0;
}
#endif //USE_MEMORY_MANAGER
