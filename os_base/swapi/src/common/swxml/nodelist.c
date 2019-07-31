#include "swapi.h"
#include "swxml.h"
#include "nodelist.h"
#include "membuf.h"
#include "parser.h"
#include "swxml.h"
#include "swmem.h"

/*================================================================
 *   xml_nodelist_getItem
 *       Returns the indexth item in the collection. If index is greater
 *       than or equal to the number of nodes in the list, this returns 
 *       null.
 *       External function.
 *
 *=================================================================*/
XML_Node *
xml_nodelist_getItem( XML_NodeList * nList,
		unsigned long index )
{
	XML_NodeList *next;
	unsigned int i;

	// if the list ptr is NULL
	if( nList == NULL ) {
		return NULL;
	}
	// if index is more than list length
	if( index > xml_nodelist_getSize( nList ) - 1 ) {
		return NULL;
	}

	next = nList;
	for( i = 0; i < index && next != NULL; ++i ) {
		next = next->next;
	}

	return next->nodeItem;
}

/*================================================================
 *   xml_nodelist_addItem
 *       Add a node to nodelist
 *       Internal to parser only.
 *
 *=================================================================*/
int
xml_nodelist_addItem( XML_NodeList ** nList,
		XML_Node * add )
{
	XML_NodeList *traverse,
								*p = NULL;
	XML_NodeList *newListItem;

	assert( add != NULL );

	if( add == NULL ) {
		return XML_FAILED;
	}

	if( *nList == NULL )        // nodelist is empty
	{
		*nList = ( XML_NodeList * ) malloc( sizeof( XML_NodeList ) );
		if( *nList == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}

		memset( nList, 0, sizeof(XML_NodeList) );
	}

	if( ( *nList )->nodeItem == NULL ) {
		( *nList )->nodeItem = add;
	} else {
		traverse = *nList;
		while( traverse != NULL ) {
			p = traverse;
			traverse = traverse->next;
		}

		newListItem =
			( XML_NodeList * ) malloc( sizeof( XML_NodeList ) );
		if( newListItem == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
		p->next = newListItem;
		newListItem->nodeItem = add;
		newListItem->next = NULL;
	}

	return XML_SUCCESS;
}

/*================================================================
 *   xml_nodelist_getSize
 *       Returns the number of nodes in the list.  The range of valid
 *       child node indices is 0 to length-1 inclusive.
 *       External function.       
 *
 *=================================================================*/
unsigned long
xml_nodelist_getSize( XML_NodeList * nList )
{
	XML_NodeList *list;
	unsigned long length = 0;

	list = nList;
	while( list != NULL ) {
		++length;
		list = list->next;
	}

	return length;
}

/*================================================================
 *   xml_nodelist_clear
 *       frees a nodeList
 *       External function
 *       
 *=================================================================*/
void
xml_nodelist_clear( XML_NodeList * nList )
{
	XML_NodeList *next;

	while( nList != NULL ) {
		next = nList->next;

		free( nList );
		nList = next;
	}
}
