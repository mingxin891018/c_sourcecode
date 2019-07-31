#include "swapi.h"
#include "swxml.h"
#include "node.h"
#include "membuf.h"
#include "document.h"
#include "element.h"
#include "parser.h"
#include "swmem.h"

/*================================================================
*   xml_element_getTagName
*       Gets the element node's tagName
*       External function.
*
*=================================================================*/
const char*
xml_element_getTagName( XML_Element * element )
{
	if( element != NULL ) {
		return element->tagName;
	} else {
		return NULL;
	}
}

/*================================================================
*   xml_element_setTagName
*       Sets the given element's tagName.
*   Parameters:
*       tagName: new tagName for the element.
*
*=================================================================*/
int
xml_element_setTagName( XML_Element * element,
                        char *tagName )
{
	int rc = XML_SUCCESS;

	assert( ( element != NULL ) && ( tagName != NULL ) );
	if( ( element == NULL ) || ( tagName == NULL ) ) {
		return XML_FAILED;
	}

	if( element->tagName != NULL ) {
		free( element->tagName );
	}

	element->tagName = strdup( tagName );
	if( element->tagName == NULL ) {
		rc = XML_INSUFFICIENT_MEMORY;
	}

	return rc;
}

/*================================================================
*   xml_element_getAttribute
*       Retrievea an attribute value by name.
*       External function.
*   Parameters:
*       name: the name of the attribute to retrieve.
*   Return Values:
*       attribute value as a string, or the empty string if that attribute
*       does not have a specified value.
*
*=================================================================*/
char*
xml_element_getAttribute( XML_Element * element,
                          char* name )
{
	XML_Node *attrNode;

	if( ( element == NULL ) || ( name == NULL ) ) {
		return NULL;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, name ) == 0 ) { // found it
			return attrNode->nodeValue;
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return NULL;
}

/*================================================================
*   xml_element_setAttribute
*       Adds a new attribute.  If an attribute with that name is already
*       present in the element, its value is changed to be that of the value
*       parameter. If not, a new attribute is inserted into the element.
*
*       External function.
*   Parameters:
*       name: the name of the attribute to create or alter.
*       value: value to set in string form
*   Return Values:
*       XML_SUCCESS or failure code.    
*
*=================================================================*/
int
xml_element_setAttribute( XML_Element * element,
                          char *name,
                          char *value )
{
	XML_Node *attrNode;
	XML_Attr *newAttrNode;
	short errCode = XML_SUCCESS;

	if( ( element == NULL ) || ( name == NULL ) || ( value == NULL ) ) {
		errCode = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	if( xml_parser_isValidXmlName( name ) == FALSE ) {
		errCode = XML_INVALID_CHARACTER_ERR;
		goto ErrorHandler;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, name ) == 0 ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	if( attrNode == NULL ) {    // add a new attribute
		errCode = xml_document_createAttributeEx( name, &newAttrNode );
		if( errCode != XML_SUCCESS ) {
			goto ErrorHandler;
		}

		attrNode = ( XML_Node * ) newAttrNode;

		attrNode->nodeValue = strdup( value );
		if( attrNode->nodeValue == NULL ) {
			xml_node_free( newAttrNode );
			errCode = XML_INSUFFICIENT_MEMORY;
			goto ErrorHandler;
		}

		errCode =
			xml_element_setAttributeNode( element, newAttrNode, NULL );
		if( errCode != XML_SUCCESS ) {
			xml_node_free( newAttrNode );
			goto ErrorHandler;
		}

	} else {
		if( attrNode->nodeValue != NULL ) { // attribute name has a value already
			free( attrNode->nodeValue );
		}

		attrNode->nodeValue = strdup( value );
		if( attrNode->nodeValue == NULL ) {
			errCode = XML_INSUFFICIENT_MEMORY;
		}
	}

ErrorHandler:
	return errCode;
}

/*================================================================
*   xml_element_removeAttribute
*       Removes an attribute value by name. The attribute node is
*       not removed.
*       External function.
*   Parameters:
*       name: the name of the attribute to remove.
*   Return Values:
*       XML_SUCCESS or error code.
*
*=================================================================*/
int
xml_element_removeAttribute( XML_Element * element,
                             char *name )
{
	XML_Node *attrNode;

	if( ( element == NULL ) || ( name == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, name ) == 0 ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	if( attrNode != NULL ) {    // has the attribute
		if( attrNode->nodeValue != NULL ) {
			free( attrNode->nodeValue );
			attrNode->nodeValue = NULL;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_element_getAttributeNode
*       Retrieve an attribute node by name.
*       External function.        
*   Parameters:
*       name: the name(nodeName) of the attribute to retrieve.
*   Return Value:
*       The attr node with the specified name (nodeName) or NULL if
*       there is no such attribute.  
*
*=================================================================*/
XML_Attr *
xml_element_getAttributeNode( XML_Element * element,
                              char *name )
{

	XML_Node *attrNode;

	if( ( element == NULL ) || ( name == NULL ) ) {
		return NULL;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, name ) == 0 ) { // found it
			break;
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return ( XML_Attr * ) attrNode;

}

/*================================================================
*   xml_element_setAttributeNode
*       Adds a new attribute node.  If an attribute with that name(nodeName)
*       is already present in the element, it is replaced by the new one.
*       External function.
*   Parameters:
*       The attr node to add to the attribute list.
*   Return Value:
*       if newAttr replaces an existing attribute, the replaced
*       attr node is returned, otherwise NULL is returned.           
*
*=================================================================*/
int
xml_element_setAttributeNode( XML_Element * element,
                              XML_Attr * newAttr,
                              XML_Attr ** rtAttr )
{

	XML_Node *attrNode;
	XML_Node *node;
	XML_Node *nextAttr = NULL;
	XML_Node *prevAttr = NULL;
	XML_Node *preSib,
					 *nextSib;

	if( ( element == NULL ) || ( newAttr == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	if( newAttr->owner != NULL ) {
		return XML_INUSE_ATTRIBUTE_ERR;
	}

	newAttr->owner = element;
	node = ( XML_Node * ) newAttr;

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, node->nodeName ) == 0 ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	if( attrNode != NULL )      // already present, will replace by newAttr
	{
		preSib = attrNode->prevSibling;
		nextSib = attrNode->nextSibling;

		if( preSib != NULL ) {
			preSib->nextSibling = node;
		}

		if( nextSib != NULL ) {
			nextSib->prevSibling = node;
		}

		if( element->firstAttr == attrNode ) {
			element->firstAttr = node;
		}

		if( rtAttr != NULL ) {
			*rtAttr = ( XML_Attr * ) attrNode;
		}
	} else                      // add this attribute 
	{
		if( element->firstAttr != NULL ) {
			prevAttr = element->firstAttr;
			nextAttr = prevAttr->nextSibling;
			while( nextAttr != NULL ) {
				prevAttr = nextAttr;
				nextAttr = prevAttr->nextSibling;
			}
			prevAttr->nextSibling = node;
			node->prevSibling = prevAttr;
		} else                  // this is the first attribute node
		{
			element->firstAttr = node;
			node->prevSibling = NULL;
			node->nextSibling = NULL;
		}

		if( rtAttr != NULL ) {
			*rtAttr = NULL;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_element_findAttributeNode
*       Find a attribute node whose contents are the same as the oldAttr.
*       Internal only to parser.
*   Parameter:
*       oldAttr: the attribute node to match
*   Return:
*       if found it, the attribute node is returned,
*       otherwise, return NULL.
*
*=================================================================*/
XML_Node *
xml_element_findAttributeNode( XML_Element * element,
                               XML_Attr * oldAttr )
{
	XML_Node *attrNode;
	XML_Node *oldAttrNode = ( XML_Node * ) oldAttr;

	assert( ( element != NULL ) && ( oldAttr != NULL ) );

	attrNode = element->firstAttr;
	while( attrNode != NULL ) { // parentNode, prevSib, nextSib and ownerDocument doesn't matter
		if( xml_node_compare( attrNode, oldAttrNode ) == TRUE ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return attrNode;
}

/*================================================================
*   xml_element_removeAttributeNode	
*       Removes the specified attribute node.
*       External function.
*
*   Parameters:
*       oldAttr: the attr node to remove from the attribute list.
*       
*   Return Value:
*       XML_SUCCESS or failure
*
*=================================================================*/
int
xml_element_removeAttributeNode( XML_Element * element,
                                 XML_Attr * oldAttr,
                                 XML_Attr ** rtAttr )
{
	XML_Node *attrNode;
	XML_Node *preSib,
					 *nextSib;

	if( ( element == NULL ) || ( oldAttr == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	attrNode = xml_element_findAttributeNode( element, oldAttr );
	if( attrNode != NULL ) {    // has the attribute
		preSib = attrNode->prevSibling;
		nextSib = attrNode->nextSibling;

		if( preSib != NULL ) {
			preSib->nextSibling = nextSib;
		}

		if( nextSib != NULL ) {
			nextSib->prevSibling = preSib;
		}

		if( element->firstAttr == attrNode ) {
			element->firstAttr = nextSib;
		}

		/* ( XML_Attr * ) */ attrNode->parentNode = NULL;
		/* ( XML_Attr * ) */ attrNode->prevSibling = NULL;
		/* ( XML_Attr * ) */ attrNode->nextSibling = NULL;
		*rtAttr = ( XML_Attr * ) attrNode;
		return XML_SUCCESS;
	} else {
		return XML_NOT_FOUND_ERR;
	}
}

/*================================================================
*   xml_element_getElementsByTagName
*       Returns a nodeList of all descendant Elements with a given
*       tag name, in the order in which they are encountered in a preorder
*       traversal of this element tree.
*       External function.
*
*   Parameters:
*       tagName: the name of the tag to match on. The special value "*"
*       matches all tags.
*
*   Return Value:
*       a nodeList of matching element nodes.
*
*=================================================================*/
XML_NodeList *
xml_element_getElementsByTagName( XML_Element * element,
                                  char *tagName )
{
	XML_NodeList *returnNodeList = NULL;

	if( ( element != NULL ) && ( tagName != NULL ) ) {
		xml_node_getElementsByTagName( ( XML_Node * ) element, tagName,
				&returnNodeList );
	}
	return returnNodeList;
}

/*================================================================
*   xml_element_getAttributeNS
*       Retrieves an attribute value by local name and namespace URI.
*       External function.
*
*   Parameters:
*       namespaceURI: the namespace URI of the attribute to retrieve.
*       localName: the local name of the attribute to retrieve.
*
*   Return Value:
*       the attr value as a string, or NULL if that attribute does
*       not have the specified value.
*
*=================================================================*/
char*
xml_element_getAttributeNS( XML_Element * element,
                            char* namespaceURI,
                            char* localName )
{
	XML_Node *attrNode;

	if( ( element == NULL ) || ( namespaceURI == NULL )
			|| ( localName == NULL ) ) {
		return NULL;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->localName, localName ) == 0 && strcmp( attrNode->namespaceURI, namespaceURI ) == 0 ) {    // found it
			return attrNode->nodeValue;
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return NULL;
}

/*================================================================
*   xml_element_setAttributeNS
*       Adds a new attribute. If an attribute with the same local name
*       and namespace URI is already present on the element, its prefix
*       is changed to be the prefix part of the qualifiedName, and its
*       value is changed to be the value parameter.  This value is a
*       simple string.
*       External function.
*
*   Parameter:
*       namespaceURI: the namespace of the attribute to create or alter.
*       qualifiedName: the qualified name of the attribute to create or alter.
*       value: the value to set in string form.
*
*   Return Value:
*       XML_SUCCESS or failure 
*
*=================================================================*/
int
xml_element_setAttributeNS( XML_Element * element,
                            char* namespaceURI,
                            char* qualifiedName,
                            char* value )
{
	XML_Node *attrNode = NULL;
	XML_Node newAttrNode;
	XML_Attr *newAttr;
	int rc;

	if( ( element == NULL ) || ( namespaceURI == NULL ) ||
			( qualifiedName == NULL ) || ( value == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	if( xml_parser_isValidXmlName( qualifiedName ) == FALSE ) {
		return XML_INVALID_CHARACTER_ERR;
	}

	memset( &newAttrNode, 0, sizeof(XML_Node) );

	newAttrNode.nodeName = strdup( qualifiedName );
	if( newAttrNode.nodeName == NULL ) {
		return XML_INSUFFICIENT_MEMORY;
	}

	rc = xml_parser_setNodePrefixAndLocalName( &newAttrNode );
	if( rc != XML_SUCCESS ) {
		xml_parser_freeNodeContent( &newAttrNode );
		return rc;
	}
	// see DOM 2 spec page 59
	if( ( newAttrNode.prefix != NULL && namespaceURI == NULL ) ||
			( strcmp( newAttrNode.prefix, "xml" ) == 0 &&
				strcmp( namespaceURI,
					"http://www.w3.org/XML/1998/namespace" ) != 0 )
			|| ( strcmp( qualifiedName, "xmlns" ) == 0
				&& strcmp( namespaceURI,
					"http://www.w3.org/2000/xmlns/" ) != 0 ) ) {
		xml_parser_freeNodeContent( &newAttrNode );
		return XML_NAMESPACE_ERR;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->localName, newAttrNode.localName ) == 0 &&
				strcmp( attrNode->namespaceURI, namespaceURI ) == 0 ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	if( attrNode != NULL ) {
		if( attrNode->prefix != NULL ) {
			free( attrNode->prefix );   // remove the old prefix
		}
		// replace it with the new prefix
		attrNode->prefix = strdup( newAttrNode.prefix );
		if( attrNode->prefix == NULL ) {
			xml_parser_freeNodeContent( &newAttrNode );
			return XML_INSUFFICIENT_MEMORY;
		}

		if( attrNode->nodeValue != NULL ) {
			free( attrNode->nodeValue );
		}

		attrNode->nodeValue = strdup( value );
		if( attrNode->nodeValue == NULL ) {
			free( attrNode->prefix );
			xml_parser_freeNodeContent( &newAttrNode );
			return XML_INSUFFICIENT_MEMORY;
		}

	} else {
		// add a new attribute
		rc = xml_document_createAttributeNSEx( namespaceURI, qualifiedName,
				&newAttr );
		if( rc != XML_SUCCESS ) {
			return rc;
		}

		newAttr->nodeValue = strdup( value );
		if( newAttr->nodeValue == NULL ) {
			xml_node_free( newAttr );
			return XML_INSUFFICIENT_MEMORY;
		}

		if( xml_element_setAttributeNodeNS( element, newAttr, NULL ) !=
				XML_SUCCESS ) {
			xml_node_free( newAttr );
			return XML_FAILED;
		}

	}

	xml_parser_freeNodeContent( &newAttrNode );
	return XML_SUCCESS;
}

/*================================================================
*   xml_element_removeAttributeNS
*       Removes an attribute by local name and namespace URI. The replacing
*       attribute has the same namespace URI and local name, as well as
*       the original prefix.
*       External function.
*
*   Parameters:
*       namespaceURI: the namespace URI of the attribute to remove.
*       localName: the local name of the atribute to remove.
*
*   Return Value:
*       XML_SUCCESS or failure.
*
*=================================================================*/
int
xml_element_removeAttributeNS( XML_Element * element,
                               char* namespaceURI,
                               char* localName )
{
	XML_Node *attrNode;

	if( ( element == NULL ) || ( namespaceURI == NULL )
			|| ( localName == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->localName, localName ) == 0 &&
				strcmp( attrNode->namespaceURI, namespaceURI ) == 0 ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	if( attrNode != NULL ) {    // has the attribute
		if( attrNode->nodeValue != NULL ) {
			free( attrNode->nodeValue );
			attrNode->nodeValue = NULL;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_element_getAttributeNodeNS
*       Retrieves an attr node by local name and namespace URI. 
*       External function.
*
*   Parameter:
*       namespaceURI: the namespace of the attribute to retrieve.
*       localName: the local name of the attribute to retrieve.
*
*   Return Value:
*       The attr node with the specified attribute local name and 
*       namespace URI or null if there is no such attribute.
*
*=================================================================*/
XML_Attr *
xml_element_getAttributeNodeNS( XML_Element * element,
                                char* namespaceURI,
                                char* localName )
{

	XML_Node *attrNode;

	if( ( element == NULL ) || ( namespaceURI == NULL )
			|| ( localName == NULL ) ) {
		return NULL;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->localName, localName ) == 0 && strcmp( attrNode->namespaceURI, namespaceURI ) == 0 ) {    // found it
			break;
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return ( XML_Attr * ) attrNode;
}

/*================================================================
*   xml_element_setAttributeNodeNS
*       Adds a new attribute. If an attribute with that local name and
*       that namespace URI is already present in the element, it is replaced
*       by the new one.
*       External function.
*
*   Parameter:
*       newAttr: the attr node to add to the attribute list.
*
*   Return Value:
*       If the newAttr attribute replaces an existing attribute with the
*       same local name and namespace, the replaced attr node is returned,
*       otherwise null is returned.
*
*=================================================================*/
int
xml_element_setAttributeNodeNS( XML_Element * element,
                                XML_Attr * newAttr,
                                XML_Attr ** rtAttr )
{
	XML_Node *attrNode;
	XML_Node *node;
	XML_Node *prevAttr = NULL,
					 *nextAttr = NULL;
	XML_Node *preSib,
					 *nextSib;

	if( ( element == NULL ) || ( newAttr == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	if( ( newAttr->owner != NULL )
			&& ( newAttr->owner != element ) ) {
		return XML_INUSE_ATTRIBUTE_ERR;
	}

	newAttr->owner = element;
	node = ( XML_Node * ) newAttr;

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->localName, node->localName ) == 0 &&
				strcmp( attrNode->namespaceURI, node->namespaceURI ) == 0 ) {
			break;              //found it
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	if( attrNode != NULL )      // already present, will replace by newAttr
	{
		preSib = attrNode->prevSibling;
		nextSib = attrNode->nextSibling;

		if( preSib != NULL ) {
			preSib->nextSibling = node;
		}

		if( nextSib != NULL ) {
			nextSib->prevSibling = node;
		}

		if( element->firstAttr == attrNode ) {
			element->firstAttr = node;
		}

		*rtAttr = ( XML_Attr * ) attrNode;

	} else                      // add this attribute 
	{
		if( element->firstAttr != NULL )  // element has attribute already
		{
			prevAttr = element->firstAttr;
			nextAttr = prevAttr->nextSibling;
			while( nextAttr != NULL ) {
				prevAttr = nextAttr;
				nextAttr = prevAttr->nextSibling;
			}
			prevAttr->nextSibling = node;
		} else                  // this is the first attribute node
		{
			element->firstAttr = node;
			node->prevSibling = NULL;
			node->nextSibling = NULL;
		}

		if( rtAttr != NULL ) {
			*rtAttr = NULL;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_element_getElementsByTagNameNS
*       Returns a nodeList of all the descendant Elements with a given
*       local name and namespace in the order in which they are encountered
*       in a preorder traversal of the element tree.
*       External function.
*
*   Parameters:
*       namespaceURI: the namespace URI of the elements to match on. The
*               special value "*" matches all namespaces.
*       localName: the local name of the elements to match on. The special
*               value "*" matches all local names.
*
*   Return Value:
*       A new nodeList object containing all the matched Elements.
*
*=================================================================*/
XML_NodeList *
xml_element_getElementsByTagNameNS( XML_Element * element,
                                    char* namespaceURI,
                                    char* localName )
{
	XML_Node *node = ( XML_Node * ) element;
	XML_NodeList *nodeList = NULL;

	if( ( element != NULL ) && ( namespaceURI != NULL )
			&& ( localName != NULL ) ) {
		xml_node_getElementsByTagNameNS( node, namespaceURI, localName,
				&nodeList );
	}

	return nodeList;
}

/*================================================================
*   xml_element_hasAttribute
*       Returns true when an attribute with a given name is specified on
*       this element, false otherwise.
*       External function.
*
*   Parameters:
*       name: the name of the attribute to look for.
*
*   Return Value:
*       ture if an attribute with the given name is specified on this
*       element, false otherwise.
*
*=================================================================*/
BOOL
xml_element_hasAttribute( XML_Element * element,
                          char* name )
{
	XML_Node *attrNode;

	if( ( element == NULL ) || ( name == NULL ) ) {
		return FALSE;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, name ) == 0 ) {
			return TRUE;
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return FALSE;
}

/*================================================================
*   xml_element_hasAttributeNS
*       Returns true when attribute with a given local name and namespace
*       URI is specified on this element, false otherwise.
*       External function.
*
*   Parameters:
*       namespaceURI: the namespace URI of the attribute to look for.
*       localName: the local name of the attribute to look for.
*
*   Return Value:
*       true if an attribute with the given local name and namespace URI
*       is specified, false otherwise.
*
*=================================================================*/
BOOL
xml_element_hasAttributeNS( XML_Element * element,
                            char* namespaceURI,
                            char* localName )
{
	XML_Node *attrNode;

	if( ( element == NULL ) || ( namespaceURI == NULL )
			|| ( localName == NULL ) ) {
		return FALSE;
	}

	attrNode = element->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->localName, localName ) == 0 &&
				strcmp( attrNode->namespaceURI, namespaceURI ) == 0 ) {
			return TRUE;
		} else {
			attrNode = attrNode->nextSibling;
		}
	}

	return FALSE;
}
