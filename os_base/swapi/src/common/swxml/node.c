#include "swapi.h"
#include "swxml.h"
#include "node.h"
#include "nodelist.h"
#include "map.h"
#include "element.h"
#include "membuf.h"
#include "document.h"
#include "parser.h"
#include "swmem.h"

/*================================================================
*   xml_node_freeSingleNode
*       frees a node content.
*       Internal to parser only.
*
*=================================================================*/
void
xml_node_freeSingleNode( XML_Node * nodeptr )
{
	XML_Element *element = NULL;

	if( nodeptr != NULL ) {
		if( nodeptr->nodeName != NULL ) {
			free( nodeptr->nodeName );
		}

		if( nodeptr->nodeValue != NULL ) {
			free( nodeptr->nodeValue );
		}

		if( nodeptr->namespaceURI != NULL ) {
			free( nodeptr->namespaceURI );
		}

		if( nodeptr->prefix != NULL ) {
			free( nodeptr->prefix );
		}

		if( nodeptr->localName != NULL ) {
			free( nodeptr->localName );
		}

		if( nodeptr->nodeType == eELEMENT_NODE ) {
			element = ( XML_Element * ) nodeptr;
			free( element->tagName );
		}

		free( nodeptr );
	}
}

/*================================================================
*   xml_node_free
*       Frees all nodes under nodeptr subtree.
*       External function.
*
*=================================================================*/
void
xml_node_free( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		xml_node_free( nodeptr->firstChild );
		xml_node_free( nodeptr->nextSibling );
		xml_node_free( nodeptr->firstAttr );
		xml_node_freeSingleNode( nodeptr );
	}
}

/*================================================================
*   xml_node_getNodeName
*       Returns the nodename(the qualified name)
*       External function.
*
*=================================================================*/
const char*
xml_node_getNodeName( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return ( nodeptr->nodeName );
	}

	return NULL;
}

/*================================================================
*   xml_node_getLocalName
*       Returns the node local name
*       External function.          		
*
*=================================================================*/
const char*
xml_node_getLocalName( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return ( nodeptr->localName );
	}

	return NULL;
}

/*================================================================
*   xml_node_setNamespaceURI
*       sets the namespace URI of the node.
*       Internal function.
*	Return:
*       XML_SUCCESS or failure	
*
*=================================================================*/
int
xml_node_setNamespaceURI( XML_Node * nodeptr,
                          char *namespaceURI )
{
	if( nodeptr == NULL ) {
		return XML_INVALID_PARAMETER;
	}

	if( nodeptr->namespaceURI != NULL ) {
		free( nodeptr->namespaceURI );
		nodeptr->namespaceURI = NULL;
	}

	if( namespaceURI != NULL ) {
		nodeptr->namespaceURI = strdup( namespaceURI );
		if( nodeptr->namespaceURI == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_node_setPrefix
*       set the prefix of the node.
*       Internal to parser only.
*	Returns:	
*       XML_SUCCESS or failure.
*
*=================================================================*/
int
xml_node_setPrefix( XML_Node * nodeptr,
                    char *prefix )
{

	if( nodeptr == NULL ) {
		return XML_INVALID_PARAMETER;
	}

	if( nodeptr->prefix != NULL ) {
		free( nodeptr->prefix );
		nodeptr->prefix = NULL;
	}

	if( prefix != NULL ) {
		nodeptr->prefix = strdup( prefix );
		if( nodeptr->prefix == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}

	return XML_SUCCESS;

}

/*================================================================
*   xml_node_setLocalName
*	    set the localName of the node.
*       Internal to parser only.
*	Returns:	
*       XML_SUCCESS or failure.
*
*=================================================================*/
int
xml_node_setLocalName( XML_Node * nodeptr,
                       char *localName )
{
	assert( nodeptr != NULL );

	if( nodeptr->localName != NULL ) {
		free( nodeptr->localName );
		nodeptr->localName = NULL;
	}

	if( localName != NULL ) {
		nodeptr->localName = strdup( localName );
		if( nodeptr->localName == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_node_getNodeNamespaceURI
*       Returns the node namespaceURI
*       External function.
*   Returns:		
*       the namespaceURI of the node
*
*=================================================================*/
const char*
xml_node_getNamespaceURI( XML_Node * nodeptr )
{
	char* retNamespaceURI = NULL;

	if( nodeptr != NULL ) {
		retNamespaceURI = nodeptr->namespaceURI;
	}

	return retNamespaceURI;
}

/*================================================================
*   xml_node_getPrefix
*       Returns the node prefix
*       External function.
*   Returns:		
*       the prefix of the node.
*
*=================================================================*/
char*
xml_node_getPrefix( XML_Node * nodeptr )
{
	char* prefix = NULL;

	if( nodeptr != NULL ) {
		prefix = nodeptr->prefix;
	}

	return prefix;
}

/*================================================================
*   xml_node_getNodeValue
*       Returns the nodeValue of this node
*       External function.
*   Return:
*       the nodeValue of the node.
*
*=================================================================*/
char*
xml_node_getNodeValue( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return ( nodeptr->nodeValue );
	}

	return NULL;
}

/*================================================================
*   xml_node_setNodeValue
*       Sets the nodeValue
*       Internal function.
*   Returns:    
*       XML_SUCCESS or failure
*
*=================================================================*/
int
xml_node_setNodeValue( XML_Node * nodeptr,
                       char *newNodeValue )
{
	int rc = XML_SUCCESS;

	if( nodeptr == NULL ) {
		return XML_INVALID_PARAMETER;
	}

	if( nodeptr->nodeValue != NULL ) {
		free( nodeptr->nodeValue );
		nodeptr->nodeValue = NULL;
	}

	if( newNodeValue != NULL ) {
		nodeptr->nodeValue = strdup( newNodeValue );
		if( nodeptr->nodeValue == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}

	return rc;
}

/*================================================================
*   xml_node_getNodeType
*       Gets the NodeType of this node
*       External function.
*
*=================================================================*/
unsigned short
xml_node_getNodeType( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return ( nodeptr->nodeType );
	} else {
		return ( eINVALID_NODE );
	}
}

/*================================================================
*   xml_node_getParentNode
*       Get the parent node
*       External function.
*   Return:    
*       
*=================================================================*/
XML_Node *
xml_node_getParentNode( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return nodeptr->parentNode;
	} else {
		return NULL;
	}
}

/*================================================================
*   xml_node_getFirstChild
*       Returns the first child of nodeptr.
*       External function.
*
*=================================================================*/
XML_Node *
xml_node_getFirstChild( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return nodeptr->firstChild;
	} else {
		return NULL;
	}
}

/*================================================================
*   xml_node_getLastChild
*       Returns the last child of nodeptr.
*       External function.
*
*=================================================================*/
XML_Node *
xml_node_getLastChild( XML_Node * nodeptr )
{
	XML_Node *prev,
					 *next;

	if( nodeptr != NULL ) {
		prev = nodeptr;
		next = nodeptr->firstChild;

		while( next != NULL ) {
			prev = next;
			next = next->nextSibling;
		}
		return prev;
	}

	return NULL;
}

/*================================================================
*   xml_node_getPreviousSibling
*       returns the previous sibling node of nodeptr.
*       External function.
*
*=================================================================*/
XML_Node *
xml_node_getPreviousSibling( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return nodeptr->prevSibling;
	} else {
		return NULL;
	}
}

/*================================================================
*   xml_node_getNextSibling
*       Returns the next sibling node.
*       External function.
*
*=================================================================*/
XML_Node *
xml_node_getNextSibling( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		return nodeptr->nextSibling;
	} else {
		return NULL;
	}

}

/*================================================================
*   xml_node_isAncestor
*       check if ancestorNode is ancestor of toFind 
*       Internal to parser only.
*   Returns:
*       TRUE or FALSE
*
*=================================================================*/
BOOL
xml_node_isAncestor( XML_Node * ancestorNode,
                     XML_Node * toFind )
{
	BOOL found = FALSE;

	if( ( ancestorNode != NULL ) && ( toFind != NULL ) ) {
		if( toFind->parentNode == ancestorNode ) {
			return TRUE;
		} else {
			found =
				xml_node_isAncestor( ancestorNode->firstChild, toFind );
			if( found == FALSE ) {
				found =
					xml_node_isAncestor( ancestorNode->nextSibling,
							toFind );
			}
		}
	}

	return found;
}

/*================================================================
*   xml_node_isParent
*       Check whether toFind is a children of nodeptr.
*       Internal to parser only.
*   Return:
*       TRUE or FALSE       
*
*=================================================================*/
BOOL
xml_node_isParent( XML_Node * nodeptr,
                   XML_Node * toFind )
{
	BOOL found = FALSE;

	assert( nodeptr != NULL && toFind != NULL );
	if( toFind->parentNode == nodeptr ) {
		found = TRUE;
	}

	return found;
}

/*================================================================
*   xml_node_allowChildren
*       Check to see whether nodeptr allows children of type newChild.    
*       Internal to parser only.
*   Returns:      
*       TRUE, if nodeptr can have newChild as children
*       FALSE,if nodeptr cannot have newChild as children
*
*=================================================================*/
BOOL
xml_node_allowChildren( XML_Node * nodeptr,
                        XML_Node * newChild )
{
	assert( nodeptr != NULL && newChild != NULL );
	switch ( nodeptr->nodeType ) {
		case eATTRIBUTE_NODE:
		case eTEXT_NODE:
		case eCDATA_SECTION_NODE:
			return FALSE;
			break;

		case eELEMENT_NODE:
			if( ( newChild->nodeType == eATTRIBUTE_NODE ) ||
					( newChild->nodeType == eDOCUMENT_NODE ) ) {
				return FALSE;
			}
			break;

		case eDOCUMENT_NODE:
			if( newChild->nodeType != eELEMENT_NODE ) {
				return FALSE;
			}

		default:
			break;
	}

	return TRUE;
}

/*================================================================
*   xml_node_compare
*       Compare two nodes to see whether they are the same node.
*       Parent, sibling and children node are ignored.
*       Internal to parser only.
*   Returns:
*       TRUE, the two nodes are the same.
*       FALSE, the two nodes are not the same.
*
*=================================================================*/
BOOL
xml_node_compare( XML_Node * srcNode,
                  XML_Node * destNode )
{
	assert( srcNode != NULL && destNode != NULL );
	if( ( srcNode == destNode ) ||
			( strcmp( srcNode->nodeName, destNode->nodeName ) == 0 &&
				strcmp( srcNode->nodeValue, destNode->nodeValue ) == 0 &&
				( srcNode->nodeType == destNode->nodeType ) &&
				strcmp( srcNode->namespaceURI, destNode->namespaceURI ) == 0 &&
				strcmp( srcNode->prefix, destNode->prefix ) == 0 &&
				strcmp( srcNode->localName, destNode->localName ) == 0 ) ) {
		return TRUE;
	} else {
		return FALSE;
	}
}

/*================================================================
*   xml_node_insertBefore
*       Inserts the node newChild before the existing child node refChild.
*       If refChild is null, insert newChild at the end of the list of
*       children. If the newChild is already in the tree, it is first
*       removed.   
*       External function.
*   Parameters:
*       newChild: the node to insert.
*   Returns:
*
*=================================================================*/
int
xml_node_insertBefore( XML_Node * nodeptr,
                       XML_Node * newChild,
                       XML_Node * refChild )
{
	int ret = XML_SUCCESS;

	if( ( nodeptr == NULL ) || ( newChild == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}
	// whether nodeptr allow children of the type of newChild 
	if( xml_node_allowChildren( nodeptr, newChild ) == FALSE ) {
		return XML_HIERARCHY_REQUEST_ERR;
	}
	// or if newChild is one of nodeptr's ancestors
	if( xml_node_isAncestor( newChild, nodeptr ) == TRUE ) {
		return XML_HIERARCHY_REQUEST_ERR;
	}
	// if refChild is not a child of nodeptr
	if( xml_node_isParent( nodeptr, refChild ) == FALSE ) {
		return XML_NOT_FOUND_ERR;
	}

	if( refChild != NULL ) {
		if( xml_node_isParent( nodeptr, newChild ) == TRUE ) {
			xml_node_removeChild( nodeptr, newChild, NULL );
			newChild->nextSibling = NULL;
			newChild->prevSibling = NULL;
		}

		newChild->nextSibling = refChild;
		if( refChild->prevSibling != NULL ) {
			( refChild->prevSibling )->nextSibling = newChild;
			newChild->prevSibling = refChild->prevSibling;
		}

		refChild->prevSibling = newChild;

		if( newChild->prevSibling == NULL ) {
			nodeptr->firstChild = newChild;
		}

		newChild->parentNode = nodeptr;
	} else {
		ret = xml_node_appendChild( nodeptr, newChild );
	}

	return ret;
}

/*================================================================
*   xml_node_replaceChild
*       Replaces the child node oldChild with newChild in the list of children,
*       and returns the oldChild node.
*       External function.
*   Parameters:
*       newChild:   the new node to put in the child list.
*       oldChild:   the node being replaced in the list.
*       returnNode: the node replaced.
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if anyone of nodeptr, newChild or oldChild is NULL.
*       XML_HIERARCHY_REQUEST_ERR: if the newChild is ancestor of nodeptr or nodeptr
*                                   is of a type that does not allow children of the
*                                   type of the newChild node.
*       XML_WRONG_DOCUMENT_ERR:    if newChild was created from a different document than
*                                   the one that created this node.
*       XML_NOT_FOUND_ERR:         if oldChild is not a child of nodeptr.
*
*=================================================================*/
int
xml_node_replaceChild( XML_Node * nodeptr,
                       XML_Node * newChild,
                       XML_Node * oldChild,
                       XML_Node** returnNode )
{
	int ret = XML_SUCCESS;

	if( ( nodeptr == NULL ) || ( newChild == NULL )
			|| ( oldChild == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}
	// if nodetype of nodeptr does not allow children of the type of newChild 
	// needs to add later

	// or if newChild is one of nodeptr's ancestors
	if( xml_node_isAncestor( newChild, nodeptr ) == TRUE ) {
		return XML_HIERARCHY_REQUEST_ERR;
	}

	if( xml_node_allowChildren( nodeptr, newChild ) == FALSE ) {
		return XML_HIERARCHY_REQUEST_ERR;
	}
	// if refChild is not a child of nodeptr
	if( xml_node_isParent( nodeptr, oldChild ) != TRUE ) {
		return XML_NOT_FOUND_ERR;
	}

	ret = xml_node_insertBefore( nodeptr, newChild, oldChild );
	if( ret != XML_SUCCESS ) {
		return ret;
	}

	ret = xml_node_removeChild( nodeptr, oldChild, returnNode );
	return ret;
}

/*================================================================
*   xml_node_removeChild
*       Removes the child node indicated by oldChild from the list of
*       children, and returns it.
*       External function.
*   Parameters:
*       oldChild: the node being removed.
*       returnNode: the node removed.
*   Return Value:
*       XML_SUCCESS
*       XML_NOT_FOUND_ERR: if oldChild is not a child of this node.
*       XML_INVALID_PARAMETER: if either oldChild or nodeptr is NULL
*
*=================================================================*/
int
xml_node_removeChild( XML_Node * nodeptr,
                      XML_Node * oldChild,
                      XML_Node** returnNode )
{

	if( ( nodeptr == NULL ) || ( oldChild == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	if( xml_node_isParent( nodeptr, oldChild ) == FALSE ) {
		return XML_NOT_FOUND_ERR;
	}

	if( oldChild->prevSibling != NULL ) {
		( oldChild->prevSibling )->nextSibling = oldChild->nextSibling;
	}

	if( nodeptr->firstChild == oldChild ) {
		nodeptr->firstChild = oldChild->nextSibling;
	}

	if( oldChild->nextSibling != NULL ) {
		( oldChild->nextSibling )->prevSibling = oldChild->prevSibling;
	}

	oldChild->nextSibling = NULL;
	oldChild->prevSibling = NULL;
	oldChild->parentNode = NULL;

	if( returnNode != NULL ) {
		*returnNode = oldChild;
	}
	return XML_SUCCESS;
}

/*=============================================================================
*   xml_node_appendChild
*       Adds the node newChild to the end of the list of children of this node.
*       If the newChild is already in the tree, it is first removed.
*       External function.   
*   Parameter:
*       newChild: the node to add.
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if either nodeptr or newChild is NULL
*       XML_WRONG_DOCUMENT_ERR:    if newChild was created from a different document than
*                                   the one that created nodeptr.
*       XML_HIERARCHY_REQUEST_ERR: if newChild is ancestor of nodeptr or if nodeptr is of
*                                   a type that does not allow children of the type of the
*                                   newChild node.
*
*=================================================================*/
int
xml_node_appendChild( XML_Node * nodeptr,
                      XML_Node * newChild )
{

	XML_Node *prev = NULL,
					 *next = NULL;

	if( ( nodeptr == NULL ) || ( newChild == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}
	// if newChild is an ancestor of nodeptr
	if( xml_node_isAncestor( newChild, nodeptr ) == TRUE ) {
		return XML_HIERARCHY_REQUEST_ERR;
	}
	// if nodeptr does not allow to have newChild as children
	if( xml_node_allowChildren( nodeptr, newChild ) == FALSE ) {
		return XML_HIERARCHY_REQUEST_ERR;
	}

	if( xml_node_isParent( nodeptr, newChild ) == TRUE ) {
		xml_node_removeChild( nodeptr, newChild, NULL );
	}
	// set the parent node pointer
	newChild->parentNode = nodeptr;

	//if the first child
	if( nodeptr->firstChild == NULL ) {
		nodeptr->firstChild = newChild;
	} else {
		prev = nodeptr->firstChild;
		next = prev->nextSibling;
		while( next != NULL ) {
			prev = next;
			next = prev->nextSibling;
		}
		prev->nextSibling = newChild;
		newChild->prevSibling = prev;
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_node_cloneTextNode
*       Returns a clone of nodeptr
*       Internal to parser only.
*
*=================================================================*/
XML_Node *
xml_node_cloneTextNode( XML_Node * nodeptr )
{
	XML_Node *newNode = NULL;

	assert( nodeptr != NULL );

	newNode = ( XML_Node * ) malloc( sizeof( XML_Node ) );
	if( newNode == NULL ) {
		return NULL;
	} else {
		memset( newNode, 0, sizeof(XML_Node) );

		xml_node_setNodeName( newNode, nodeptr->nodeName );
		xml_node_setNodeValue( newNode, nodeptr->nodeValue );
		newNode->nodeType = eTEXT_NODE;
	}

	return newNode;
}

/*================================================================
*   xml_node_cloneCDATASect
*       Return a clone of CDATASection node.
*       Internal to parser only.
*
*=================================================================*/
XML_CDATASection *
xml_node_cloneCDATASect( XML_CDATASection * nodeptr )
{
	XML_CDATASection *newCDATA = NULL;
	XML_Node *newNode;
	XML_Node *srcNode;

	assert( nodeptr != NULL );
	newCDATA =
		( XML_CDATASection * ) malloc( sizeof( XML_CDATASection ) );
	if( newCDATA != NULL ) {
		newNode = ( XML_Node * ) newCDATA;
		if( newNode )
			memset( newNode, 0, sizeof(XML_Node) );

		srcNode = ( XML_Node * ) nodeptr;
		xml_node_setNodeName( newNode, srcNode->nodeName );
		xml_node_setNodeValue( newNode, srcNode->nodeValue );
		newNode->nodeType = eCDATA_SECTION_NODE;
	}

	return newCDATA;
}

/*================================================================
*   xml_node_cloneElement
*       returns a clone of element node
*       Internal to parser only.
*
*=================================================================*/
XML_Element *
xml_node_cloneElement( XML_Element * nodeptr )
{
	XML_Element *newElement;
	XML_Node *elementNode;
	XML_Node *srcNode;
	int rc;

	assert( nodeptr != NULL );

	newElement = ( XML_Element * ) malloc( sizeof( XML_Element ) );
	if( newElement == NULL ) {
		return NULL;
	}

	memset( newElement, 0, sizeof( XML_Element ) );
	rc = xml_element_setTagName( newElement, nodeptr->tagName );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
	}

	elementNode = ( XML_Node * ) newElement;
	srcNode = ( XML_Node * ) nodeptr;
	rc = xml_node_setNodeName( elementNode, srcNode->nodeName );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
	}

	rc = xml_node_setNodeValue( elementNode, srcNode->nodeValue );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
	}

	rc = xml_node_setNamespaceURI( elementNode, srcNode->namespaceURI );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
	}

	rc = xml_node_setPrefix( elementNode, srcNode->prefix );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
	}

	rc = xml_node_setLocalName( elementNode, srcNode->localName );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
	}

	elementNode->nodeType = eELEMENT_NODE;

	return newElement;
}

/*================================================================
*   xml_node_cloneDoc
*       Returns a clone of document node
*       Internal to parser only.
*
*=================================================================*/
XML_Document *
xml_node_cloneDoc( XML_Document * nodeptr )
{
	XML_Document *newDoc;
	XML_Node *docNode;
	int rc;

	assert( nodeptr != NULL );
	newDoc = ( XML_Document * ) malloc( sizeof( XML_Document ) );
	if( newDoc == NULL ) {
		return NULL;
	}

	memset( newDoc, 0, sizeof( XML_Document ) );
		docNode = ( XML_Node * ) newDoc;

	rc = xml_node_setNodeName( docNode, DOCUMENTNODENAME );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newDoc );
		return NULL;
	}

	newDoc->nodeType = eDOCUMENT_NODE;

	return newDoc;
}

/*================================================================
*   xml_node_cloneAttr
*       Returns a clone of attribute node
*       Internal to parser only
*
*=================================================================*/
XML_Attr *
xml_node_cloneAttr( XML_Attr * nodeptr )
{
	XML_Attr *newAttr;
	XML_Node *attrNode;
	XML_Node *srcNode;
	int rc;

	assert( nodeptr != NULL );
	newAttr = ( XML_Attr * ) malloc( sizeof( XML_Attr ) );
	if( newAttr == NULL ) {
		return NULL;
	}
	
	memset( newAttr, 0, sizeof( XML_Attr ) );
	attrNode = ( XML_Node * ) newAttr;
	srcNode = ( XML_Node * ) nodeptr;

	rc = xml_node_setNodeName( attrNode, srcNode->nodeName );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newAttr );
		return NULL;
	}

	rc = xml_node_setNodeValue( attrNode, srcNode->nodeValue );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newAttr );
		return NULL;
	}
	//check to see whether we need to split prefix and localname for attribute
	rc = xml_node_setNamespaceURI( attrNode, srcNode->namespaceURI );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newAttr );
		return NULL;
	}

	rc = xml_node_setPrefix( attrNode, srcNode->prefix );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newAttr );
		return NULL;
	}

	rc = xml_node_setLocalName( attrNode, srcNode->localName );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newAttr );
		return NULL;
	}

	attrNode->nodeType = eATTRIBUTE_NODE;

	return newAttr;
}

/*================================================================
*   xml_node_cloneAttrDirect
*       Return a clone of attribute node, with specified field set
*       to TRUE.
*
*=================================================================*/
XML_Attr *
xml_node_cloneAttrDirect( XML_Attr * nodeptr )
{

	XML_Attr *newAttr;

	assert( nodeptr != NULL );

	newAttr = xml_node_cloneAttr( nodeptr );
	if( newAttr != NULL ) {
		newAttr->specified = TRUE;
	}

	return newAttr;
}

void
xml_node_setSiblingNodesParent( XML_Node * nodeptr )
{
	XML_Node *parentNode = nodeptr->parentNode;
	XML_Node *nextptr = nodeptr->nextSibling;

	while( nextptr != NULL ) {
		nextptr->parentNode = parentNode;
		nextptr = nextptr->nextSibling;
	}
}

/*================================================================
*   xml_node_cloneNodeTreeRecursive
*       recursive functions that clones node tree of nodeptr.
*       Internal to parser only.
*       
*=================================================================*/
XML_Node *
xml_node_cloneNodeTreeRecursive( XML_Node * nodeptr,
                                 BOOL deep )
{
	XML_Node *newNode = NULL;
	XML_Element *newElement;
	XML_Attr *newAttr = NULL;
	XML_CDATASection *newCDATA = NULL;
	XML_Document *newDoc;
	XML_Node *nextSib;

	if( nodeptr != NULL ) {
		switch ( nodeptr->nodeType ) {
			case eELEMENT_NODE:
				newElement =
					xml_node_cloneElement( ( XML_Element * ) nodeptr );
				newElement->firstAttr =
					xml_node_cloneNodeTreeRecursive( nodeptr->firstAttr,
							deep );
				if( deep ) {
					newElement->firstChild =
						xml_node_cloneNodeTreeRecursive( nodeptr->firstChild, deep );
					if( newElement->firstChild != NULL ) {
						( newElement->firstChild )->parentNode =
							( XML_Node * ) newElement;
						xml_node_setSiblingNodesParent( newElement->firstChild );
					}
					nextSib =
						xml_node_cloneNodeTreeRecursive( nodeptr->nextSibling, deep );
					newElement->nextSibling = nextSib;
					if( nextSib != NULL ) {
						nextSib->prevSibling = ( XML_Node * ) newElement;
					}
				}

				newNode = ( XML_Node * ) newElement;
				break;

			case eATTRIBUTE_NODE:
				newAttr = xml_node_cloneAttr( ( XML_Attr * ) nodeptr );
				nextSib =
					xml_node_cloneNodeTreeRecursive( nodeptr->nextSibling,
							deep );
				newAttr->nextSibling = nextSib;

				if( nextSib != NULL ) {
					nextSib->prevSibling = ( XML_Node * ) newAttr;
				}
				newNode = ( XML_Node * ) newAttr;
				break;

			case eTEXT_NODE:
				newNode = xml_node_cloneTextNode( nodeptr );
				break;

			case eCDATA_SECTION_NODE:
				newCDATA =
					xml_node_cloneCDATASect( ( XML_CDATASection * )
							nodeptr );
				newNode = ( XML_Node * ) newCDATA;
				break;

			case eDOCUMENT_NODE:
				newDoc = xml_node_cloneDoc( ( XML_Document * ) nodeptr );
				newNode = ( XML_Node * ) newDoc;
				if( deep ) {
					newNode->firstChild =
						xml_node_cloneNodeTreeRecursive( nodeptr->
								firstChild,
								deep );
					if( newNode->firstChild != NULL ) {
						newNode->firstChild->parentNode = newNode;
					}
				}

				break;

			case eINVALID_NODE:
			case eENTITY_REFERENCE_NODE:
			case eENTITY_NODE:
			case ePROCESSING_INSTRUCTION_NODE:
			case eCOMMENT_NODE:
			case eDOCUMENT_TYPE_NODE:
			case eDOCUMENT_FRAGMENT_NODE:
			case eNOTATION_NODE:
				break;
		}
	}

	return newNode;
}

/*================================================================
*   xml_node_cloneNodeTree
*       clones a node tree.
*       Internal to parser only.
*
*=================================================================*/
XML_Node *
xml_node_cloneNodeTree( XML_Node * nodeptr,
                        BOOL deep )
{
	XML_Node *newNode = NULL;
	XML_Element *newElement;
	XML_Node *childNode;

	assert( nodeptr != NULL );

	switch ( nodeptr->nodeType ) {
		case eELEMENT_NODE:
			newElement =
				xml_node_cloneElement( ( XML_Element * ) nodeptr );
			newElement->firstAttr =
				xml_node_cloneNodeTreeRecursive( nodeptr->firstAttr,
						deep );
			if( deep ) {
				newElement->firstChild =
					xml_node_cloneNodeTreeRecursive( nodeptr->firstChild,
							deep );
				childNode = newElement->firstChild;
				while( childNode != NULL ) {
					childNode->parentNode = ( XML_Node * ) newElement;
					childNode = childNode->nextSibling;
				}
				newElement->nextSibling = NULL;
			}

			newNode = ( XML_Node * ) newElement;
			break;

		case eATTRIBUTE_NODE:
		case eTEXT_NODE:
		case eCDATA_SECTION_NODE:
		case eDOCUMENT_NODE:
			newNode = xml_node_cloneNodeTreeRecursive( nodeptr, deep );
			break;

		case eINVALID_NODE:
		case eENTITY_REFERENCE_NODE:
		case eENTITY_NODE:
		case ePROCESSING_INSTRUCTION_NODE:
		case eCOMMENT_NODE:
		case eDOCUMENT_TYPE_NODE:
		case eDOCUMENT_FRAGMENT_NODE:
		case eNOTATION_NODE:
			break;
	}

	// by spec, the duplicate node has no parent
	newNode->parentNode = NULL;

	return newNode;
}

/*================================================================
*   xml_node_cloneNode
*       Clones a node, if deep==TRUE, clones subtree under nodeptr.
*       External function.
*   Returns:
*       the cloned node or NULL if error occurs.
*
*=================================================================*/
XML_Node *
xml_node_cloneNode( XML_Node * nodeptr,
                    BOOL deep )
{

	XML_Node *newNode;
	XML_Attr *newAttrNode;

	if( nodeptr == NULL ) {
		return NULL;
	}

	switch ( nodeptr->nodeType ) {
		case eATTRIBUTE_NODE:
			newAttrNode =
				xml_node_cloneAttrDirect( ( XML_Attr * ) nodeptr );
			return ( XML_Node * ) newAttrNode;
			break;

		default:
			newNode = xml_node_cloneNodeTree( nodeptr, deep );
			return newNode;
			break;
	}

}

/*================================================================
*   xml_node_getChildNodes
*       Returns a XML_NodeList of all the child nodes of nodeptr.
*       External function.
*   
*=================================================================*/
XML_NodeList *
xml_node_getChildNodes( XML_Node * nodeptr )
{
	XML_Node *tempNode;
	XML_NodeList *newNodeList;
	int rc;

	if( nodeptr == NULL ) {
		return NULL;
	}

	newNodeList = ( XML_NodeList * ) malloc( sizeof( XML_NodeList ) );
	if( newNodeList == NULL ) {
		return NULL;
	}

	memset( newNodeList, 0, sizeof(XML_NodeList) );

	tempNode = nodeptr->firstChild;
	while( tempNode != NULL ) {
		rc = xml_nodelist_addItem( &newNodeList, tempNode );
		if( rc != XML_SUCCESS ) {
			xml_nodelist_clear( newNodeList );
			return NULL;
		}

		tempNode = tempNode->nextSibling;
	}
	return newNodeList;
}

/*================================================================
*   xml_node_getAttributes
*       returns a namedNodeMap of attributes of nodeptr
*       External function.
*   Returns:
*
*=================================================================*/
XML_NamedNodeMap *
xml_node_getAttributes( XML_Node * nodeptr )
{
	XML_NamedNodeMap *returnNamedNodeMap = NULL;
	XML_Node *tempNode;
	int rc;

	if( nodeptr == NULL ) {
		return NULL;
	}

	if( nodeptr->nodeType == eELEMENT_NODE ) {
		returnNamedNodeMap =
			( XML_NamedNodeMap * ) malloc( sizeof( XML_NamedNodeMap ) );
		if( returnNamedNodeMap == NULL ) {
			return NULL;
		}
		
		memset( returnNamedNodeMap, 0, sizeof( XML_NamedNodeMap ) );

		tempNode = nodeptr->firstAttr;
		while( tempNode != NULL ) {
			rc = xml_nodemap_addItem( &returnNamedNodeMap,
					tempNode );
			if( rc != XML_SUCCESS ) {
				xml_nodemap_free( returnNamedNodeMap );
				return NULL;
			}

			tempNode = tempNode->nextSibling;
		}
		return returnNamedNodeMap;
	} else {                    // if not an ELEMENT_NODE
		return NULL;
	}
}

/*================================================================
*   xml_node_hasChildNodes
*       External function.
*
*=================================================================*/
BOOL
xml_node_hasChildNodes( XML_Node * nodeptr )
{
	if( nodeptr == NULL ) {
		return FALSE;
	}

	return ( nodeptr->firstChild != NULL );
}

/*================================================================
*   xml_node_hasAttributes
*       External function.
*
*=================================================================*/
BOOL
xml_node_hasAttributes( XML_Node * nodeptr )
{
	if( nodeptr != NULL ) {
		if( ( nodeptr->nodeType == eELEMENT_NODE )
				&& ( nodeptr->firstAttr != NULL ) ) {
			return TRUE;
		}
	}
	return FALSE;

}

/*================================================================
*   xml_node_getElementsByTagNameRecursive
*       Recursively traverse the whole tree, search for element
*       with the given tagname.
*       Internal to parser.
*
*=================================================================*/
void
xml_node_getElementsByTagNameRecursive( XML_Node * n,
                                        char *tagname,
                                        XML_NodeList ** list )
{
	const char *name;

	if( n != NULL ) {
		if( xml_node_getNodeType( n ) == eELEMENT_NODE ) {
			name = xml_node_getNodeName( n );
			if( strcmp( tagname, name ) == 0
					|| strcmp( tagname, "*" ) == 0 ) {
				xml_nodelist_addItem( list, n );
			}
		}

		xml_node_getElementsByTagNameRecursive( xml_node_getFirstChild
				( n ), tagname, list );
		xml_node_getElementsByTagNameRecursive( xml_node_getNextSibling
				( n ), tagname, list );
	}

}

/*================================================================
*   xml_node_getElementsByTagName
*       Returns a nodeList of all descendant Elements with a given 
*       tagName, in the order in which they are encountered in a
*       traversal of this element tree.
*       External function.		
*
*=================================================================*/
void
xml_node_getElementsByTagName( XML_Node * n,
                               char *tagname,
                               XML_NodeList ** list )
{
	const char *name;

	assert( n != NULL && tagname != NULL );

	if( xml_node_getNodeType( n ) == eELEMENT_NODE ) {
		name = xml_node_getNodeName( n );
		if( strcmp( tagname, name ) == 0 || strcmp( tagname, "*" ) == 0 ) {
			xml_nodelist_addItem( list, n );
		}
	}

	xml_node_getElementsByTagNameRecursive( xml_node_getFirstChild( n ),
			tagname, list );

}

/*================================================================
*   xml_node_getElementsByTagNameNSRecursive
*	    Internal function to parser.	
*		
*
*=================================================================*/
void
xml_node_getElementsByTagNameNSRecursive( XML_Node * n,
                                          char *namespaceURI,
                                          char *localName,
                                          XML_NodeList ** list )
{
	const char* nsURI;
	const char* name;

	if( n != NULL ) {
		if( xml_node_getNodeType( n ) == eELEMENT_NODE ) {
			name = xml_node_getLocalName( n );
			nsURI = xml_node_getNamespaceURI( n );

			if( ( name != NULL ) && ( nsURI != NULL ) &&
					( strcmp( namespaceURI, nsURI ) == 0
						|| strcmp( namespaceURI, "*" ) == 0 )
					&& ( strcmp( name, localName ) == 0
						|| strcmp( localName, "*" ) == 0 ) ) {
				xml_nodelist_addItem( list, n );
			}
		}

		xml_node_getElementsByTagNameNSRecursive( xml_node_getFirstChild
				( n ), namespaceURI,
				localName, list );
		xml_node_getElementsByTagNameNSRecursive( xml_node_getNextSibling
				( n ), namespaceURI,
				localName, list );
	}

}

/*================================================================
*   xml_node_getElementsByTagNameNS
*       Returns a nodeList of all the descendant Elements with a given
*       local name and namespace URI in the order in which they are
*       encountered in a preorder traversal of this Elememt tree.		
*		External function.
*
*=================================================================*/
void
xml_node_getElementsByTagNameNS( XML_Node * n,
                                 char *namespaceURI,
                                 char *localName,
                                 XML_NodeList ** list )
{
	const char* nsURI;
	const char* name;

	assert( n != NULL && namespaceURI != NULL && localName != NULL );

	if( xml_node_getNodeType( n ) == eELEMENT_NODE ) {
		name = xml_node_getLocalName( n );
		nsURI = xml_node_getNamespaceURI( n );

		if( ( name != NULL ) && ( nsURI != NULL ) &&
				( strcmp( namespaceURI, nsURI ) == 0
					|| strcmp( namespaceURI, "*" ) == 0 )
				&& ( strcmp( name, localName ) == 0
					|| strcmp( localName, "*" ) == 0 ) ) {
			xml_nodelist_addItem( list, n );
		}
	}

	xml_node_getElementsByTagNameNSRecursive( xml_node_getFirstChild( n ),
			namespaceURI, localName, list );

}

/*================================================================
*   xml_node_setNodeName
*       Internal to parser only.
*
*=================================================================*/
int
xml_node_setNodeName( XML_Node * node,
                      char* qualifiedName )
{
	int rc = XML_SUCCESS;

	assert( node != NULL );

	if( node->nodeName != NULL ) {
		free( node->nodeName );
		node->nodeName = NULL;
	}

	if( qualifiedName != NULL ) {
		// set the name part
		node->nodeName = strdup( qualifiedName );
		if( node->nodeName == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}

		rc = xml_parser_setNodePrefixAndLocalName( node );
		if( rc != XML_SUCCESS ) {
			free( node->nodeName );
		}
	}

	return rc;
}

/*================================================================
*   xml_node_setNodeProperties
*       Internal to parser only.
*
*=================================================================*/
int
xml_node_setNodeProperties( XML_Node * destNode,
                            XML_Node * src )
{

	int rc;

	assert( destNode != NULL || src != NULL );

	rc = xml_node_setNodeValue( destNode, src->nodeValue );
	if( rc != XML_SUCCESS ) {
		goto ErrorHandler;
	}

	rc = xml_node_setLocalName( destNode, src->localName );
	if( rc != XML_SUCCESS ) {
		goto ErrorHandler;
	}

	rc = xml_node_setPrefix( destNode, src->prefix );
	if( rc != XML_SUCCESS ) {
		goto ErrorHandler;
	}
	// set nodetype
	destNode->nodeType = src->nodeType;

	return XML_SUCCESS;

ErrorHandler:
	if( destNode->nodeName != NULL ) {
		free( destNode->nodeName );
		destNode->nodeName = NULL;
	}
	if( destNode->nodeValue != NULL ) {
		free( destNode->nodeValue );
		destNode->nodeValue = NULL;
	}
	if( destNode->localName != NULL ) {
		free( destNode->localName );
		destNode->localName = NULL;
	}

	return XML_INSUFFICIENT_MEMORY;
}

/*================================================================
 *   xml_node_dump
 *       Print DOM tree under node. Puts lots of white spaces
 *       External function.
 *
 *=================================================================*/
char*
xml_node_dump( XML_Node * node )
{
	xml_membuf memBuf;
	xml_membuf *buf = &memBuf;

	if( node == NULL ) {
		return NULL;
	}

	xml_membuf_init( buf );
	xml_document_dump( node, buf );

	return buf->buf;
}

/*================================================================
 *   xml_node_dump
 *       converts DOM tree under node to text string
 *       External function.
 *
*=================================================================*/
char*
xml_node_toString( XML_Node * node )
{
	xml_membuf memBuf;
	xml_membuf *buf = &memBuf;

	if( node == NULL ) {
		return NULL;
	}

	xml_membuf_init( buf );
	xml_document_toString( node, buf );
	return buf->buf;
}
