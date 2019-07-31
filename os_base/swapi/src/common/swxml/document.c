#include "swapi.h"
#include "swxml.h"
#include "node.h"
#include "membuf.h"
#include "parser.h"
#include "document.h"
#include "swmem.h"

static void
copy_with_escape( xml_membuf * buf,
                  char *p )
{
	int i;
	int plen;

	if( p == NULL )
		return;

	plen = strlen( p );

	for( i = 0; i < plen; i++ ) {
		switch ( p[i] ) {
			case '<':
				xml_membuf_append_str( buf, "&lt;" );
				break;

			case '>':
				xml_membuf_append_str( buf, "&gt;" );
				break;

			case '&':
				xml_membuf_append_str( buf, "&amp;" );
				break;

			case '\'':
				xml_membuf_append_str( buf, "&apos;" );
				break;

				case '\"':
					xml_membuf_append_str( buf, "&quot;" );
				break;

			default:
				xml_membuf_append( buf, &p[i] );
		}
	}
}

/*================================================================
*   xml_document_createElementEx
*       Creates an element of the type specified. 
*       External function.
*   Parameters:
*       tagName:    The name of the element, it is case-sensitive.
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if either doc or tagName is NULL
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
*=================================================================*/
int
xml_document_createElementEx( const char* tagName,
                              XML_Element ** rtElement )
{
	int errCode = XML_SUCCESS;
	XML_Element *newElement = NULL;

	if( tagName == NULL ) {
		errCode = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	newElement = ( XML_Element * ) malloc( sizeof( XML_Element ) );
	if( newElement == NULL ) {
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	memset( newElement, 0, sizeof( XML_Element ) );
	newElement->tagName = strdup( tagName );
	if( newElement->tagName == NULL ) {
		xml_node_free( newElement );
		newElement = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}
	// set the node fields 
	newElement->nodeType = eELEMENT_NODE;
	newElement->nodeName = strdup( tagName );
	if( newElement->nodeName == NULL ) {
		xml_node_free( newElement );
		newElement = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

ErrorHandler:
	*rtElement = newElement;
	return errCode;
}

/*================================================================
*   xml_document_createElement
*       Creates an element of the type specified. 
*       External function.
*   Parameters:
*       tagName:    The name of the element, it is case-sensitive.
*   Return Value: 
*       A new element object with the nodeName set to tagName, and
*       localName, prefix and namespaceURI set to null.
*
*=================================================================*/
XML_Element *
xml_document_createElement( const char* tagName )
{
	XML_Element *newElement = NULL;

	xml_document_createElementEx( tagName, &newElement );
	return newElement;
}

/*================================================================
*   xml_document_createDocumentEx
*       Creates an document object
*       Internal function.
*   Parameters:
*       rtDoc:  the document created or NULL on failure
*   Return Value:
*       XML_SUCCESS
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
*=================================================================*/
int
xml_document_createDocumentEx( XML_Document ** rtDoc )
{
	XML_Document *doc;
	int errCode = XML_SUCCESS;

	doc = NULL;
	doc = ( XML_Document * ) malloc( sizeof( XML_Document ) );
	if( doc == NULL ) {
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	memset( doc, 0, sizeof( XML_Document ) );

	doc->nodeName = strdup( DOCUMENTNODENAME );
	if( doc->nodeName == NULL ) {
		xml_node_free( doc );
		doc = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	doc->nodeType = eDOCUMENT_NODE;

ErrorHandler:
	*rtDoc = doc;
	return errCode;
}

/*================================================================
*   xml_document_createDocument
*       Creates an document object
*       Internal function.
*   Parameters:
*       none
*   Return Value:
*       A new document object with the nodeName set to "#document".
*
*=================================================================*/
XML_Document *
xml_document_createDocument(  )
{
	XML_Document *doc = NULL;

	xml_document_createDocumentEx( &doc );

	return doc;
}

/*================================================================
*   xml_document_createTextNodeEx
*       Creates an text node. 
*       External function.
*   Parameters:
*       data: text data for the text node. It is stored in nodeValue field.
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if either doc or data is NULL
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
*=================================================================*/
int
xml_document_createTextNodeEx( const char *data,
                               XML_Node** textNode )
{
	XML_Node *returnNode;
	int rc = XML_SUCCESS;

	returnNode = NULL;
	if( data == NULL ) {
		rc = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	returnNode = ( XML_Node * ) malloc( sizeof( XML_Node ) );
	if( returnNode == NULL ) {
		rc = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}
	memset( returnNode, 0, sizeof(XML_Node) );

	returnNode->nodeName = strdup( TEXTNODENAME );
	if( returnNode->nodeName == NULL ) {
		xml_node_free( returnNode );
		returnNode = NULL;
		rc = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}
	// add in node value
	if( data != NULL ) {
		returnNode->nodeValue = strdup( data );
		if( returnNode->nodeValue == NULL ) {
			xml_node_free( returnNode );
			returnNode = NULL;
			rc = XML_INSUFFICIENT_MEMORY;
			goto ErrorHandler;
		}
	}

	returnNode->nodeType = eTEXT_NODE;

ErrorHandler:
	*textNode = returnNode;
	return rc;
}

/*================================================================
*   xml_document_createTextNode
*       Creates an text node. 
*       External function.
*   Parameters:
*       data: text data for the text node. It is stored in nodeValue field.
*   Return Value:
*       The new text node.
*
*=================================================================*/
XML_Node *
xml_document_createTextNode( const char *data )
{
	XML_Node *returnNode = NULL;

	xml_document_createTextNodeEx( data, &returnNode );

	return returnNode;
}

/*================================================================
*   xml_document_createAttributeEx
*       Creates an attribute of the given name.             
*       External function.
*   Parameters:
*       name: The name of the Attribute node.
*   Return Value:
*       XML_SUCCESS
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
================================================================*/
int
xml_document_createAttributeEx( char *name,
                                XML_Attr ** rtAttr )
{
	XML_Attr *attrNode = NULL;
	int errCode = XML_SUCCESS;

	attrNode = ( XML_Attr * ) malloc( sizeof( XML_Attr ) );
	if( attrNode == NULL ) {
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	if( name == NULL ) {
		xml_node_free( attrNode );
		attrNode = NULL;
		errCode = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	memset( attrNode, 0, sizeof( XML_Attr ) );

	attrNode->nodeType = eATTRIBUTE_NODE;

	// set the node fields
	attrNode->nodeName = strdup( name );
	if( attrNode->nodeName == NULL ) {
		xml_node_free( attrNode );
		attrNode = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

ErrorHandler:
	*rtAttr = attrNode;
	return errCode;
}

/*================================================================
*   xml_document_createAttribute
*       Creates an attribute of the given name.             
*       External function.
*   Parameters:
*       name: The name of the Attribute node.
*   Return Value:   
*       A new attr object with the nodeName attribute set to the
*       given name, and the localName, prefix and namespaceURI set to NULL.
*       The value of the attribute is the empty string.
*
================================================================*/
XML_Attr *
xml_document_createAttribute( char *name )
{
	XML_Attr *attrNode = NULL;

	xml_document_createAttributeEx( name, &attrNode );
	return attrNode;
}

/*================================================================
*   xml_document_createAttributeNSEx
*       Creates an attrbute of the given name and namespace URI
*       External function.
*   Parameters:
*       namespaceURI: the namespace fo the attribute to create
*       qualifiedName: qualifiedName of the attribute to instantiate
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if either doc,namespaceURI or qualifiedName is NULL
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
*=================================================================*/
int
xml_document_createAttributeNSEx( char* namespaceURI,
                                  char* qualifiedName,
                                  XML_Attr ** rtAttr )
{
	XML_Attr *attrNode = NULL;
	int errCode = XML_SUCCESS;

	if( ( namespaceURI == NULL ) || ( qualifiedName == NULL ) ) {
		errCode = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	errCode =
		xml_document_createAttributeEx( qualifiedName, &attrNode );
	if( errCode != XML_SUCCESS ) {
		goto ErrorHandler;
	}
	// set the namespaceURI field 
	attrNode->namespaceURI = strdup( namespaceURI );
	if( attrNode->namespaceURI == NULL ) {
		xml_node_free( attrNode );
		attrNode = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}
	// set the localName and prefix 
	errCode =
		xml_node_setNodeName( ( XML_Node * ) attrNode, qualifiedName );
	if( errCode != XML_SUCCESS ) {
		xml_node_free( attrNode );
		attrNode = NULL;
		goto ErrorHandler;
	}

ErrorHandler:
	*rtAttr = attrNode;
	return errCode;
}

/*================================================================
*   xml_document_createAttributeNS
*       Creates an attrbute of the given name and namespace URI
*       External function.
*   Parameters:
*       namespaceURI: the namespace fo the attribute to create
*       qualifiedName: qualifiedName of the attribute to instantiate
*   Return Value:   
*       Creates an attribute node with the given namespaceURI and
*       qualifiedName. The prefix and localname are extracted from 
*       the qualifiedName. The node value is empty.
*	
*=================================================================*/
XML_Attr *
xml_document_createAttributeNS( char* namespaceURI,
                                char* qualifiedName )
{
	XML_Attr *attrNode = NULL;

	xml_document_createAttributeNSEx( namespaceURI, qualifiedName,
			&attrNode );
	return attrNode;
}

/*================================================================
*   xml_document_createCDATASectionEx
*       Creates an CDATASection node whose value is the specified string
*       External function.
*   Parameters:
*       data: the data for the CDATASection contents.
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if either doc or data is NULL
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
*=================================================================*/
int
xml_document_createCDATASectionEx( char* data,
                                   XML_CDATASection ** rtCD )
{
	int errCode = XML_SUCCESS;
	XML_CDATASection *cDSectionNode = NULL;

	if( data == NULL ) {
		errCode = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	cDSectionNode =
		( XML_CDATASection * ) malloc( sizeof( XML_CDATASection ) );
	if( cDSectionNode == NULL ) {
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	memset( cDSectionNode, 0, sizeof( XML_CDATASection ) );

	cDSectionNode->nodeType = eCDATA_SECTION_NODE;
	cDSectionNode->nodeName = strdup( CDATANODENAME );
	if( cDSectionNode->nodeName == NULL ) {
		xml_node_free( cDSectionNode );
		cDSectionNode = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	cDSectionNode->nodeValue = strdup( data );
	if( cDSectionNode->nodeValue == NULL ) {
		xml_node_free( cDSectionNode );
		cDSectionNode = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

ErrorHandler:
	*rtCD = cDSectionNode;
	return errCode;
}

/*================================================================
*   xml_document_createCDATASection
*       Creates an CDATASection node whose value is the specified string
*       External function.
*   Parameters:
*       data: the data for the CDATASection contents.
*   Return Value:   
*       The new CDATASection object.
*	
*=================================================================*/
XML_CDATASection *
xml_document_createCDATASection( char* data )
{
	XML_CDATASection *cDSectionNode = NULL;

	xml_document_createCDATASectionEx( data, &cDSectionNode );
	return cDSectionNode;
}

/*================================================================
*   xml_document_createElementNSEx
*       Creates an element of the given qualified name and namespace URI.
*       External function.
*   Parameters:
*       namespaceURI: the namespace URI of the element to create.
*       qualifiedName: the qualified name of the element to instantiate.
*   Return Value:   
*   Return Value:
*       XML_SUCCESS
*       XML_INVALID_PARAMETER:     if either doc,namespaceURI or qualifiedName is NULL
*       XML_INSUFFICIENT_MEMORY:   if not enough memory to finish this operations.
*
*=================================================================*/
int
xml_document_createElementNSEx( char* namespaceURI,
                                char* qualifiedName,
                                XML_Element ** rtElement )
{
	XML_Element *newElement = NULL;
	int errCode = XML_SUCCESS;

	if( ( namespaceURI == NULL ) || ( qualifiedName == NULL ) ) {
		errCode = XML_INVALID_PARAMETER;
		goto ErrorHandler;
	}

	errCode = xml_document_createElementEx( qualifiedName, &newElement );
	if( errCode != XML_SUCCESS ) {
		goto ErrorHandler;
	}
	// set the namespaceURI field 
	newElement->namespaceURI = strdup( namespaceURI );
	if( newElement->namespaceURI == NULL ) {
		xml_node_free( newElement );
		newElement = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}
	// set the localName and prefix 
	errCode =
		xml_node_setNodeName( ( XML_Node * ) newElement, qualifiedName );
	if( errCode != XML_SUCCESS ) {
		xml_node_free( newElement );
		newElement = NULL;
		errCode = XML_INSUFFICIENT_MEMORY;
		goto ErrorHandler;
	}

	newElement->nodeValue = NULL;

ErrorHandler:
	*rtElement = newElement;
	return errCode;
}

/*================================================================
*   xml_document_createElementNS
*       Creates an element of the given qualified name and namespace URI.
*       External function.
*   Parameters:
*       namespaceURI: the namespace URI of the element to create.
*       qualifiedName: the qualified name of the element to instantiate.
*   Return Value:   
*       The new element object with tagName qualifiedName, prefix and
*       localName extraced from qualfiedName, nodeName of qualfiedName,
*	    namespaceURI of namespaceURI.
*
*=================================================================*/
XML_Element *
xml_document_createElementNS( char* namespaceURI,
                              char* qualifiedName )
{
	XML_Element *newElement = NULL;

	xml_document_createElementNSEx( namespaceURI, qualifiedName, &newElement );
	return newElement;
}

/*================================================================
*   xml_document_getElementsByTagName
*       Returns a nodeList of all the Elements with a given tag name
*       in the order in which they are encountered in a preorder traversal
*       of the document tree.
*       External function.
*   Parameters:
*       tagName: the name of the tag to match on. The special value "*"
*                matches all tags.
*   Return Value:
*       A new nodeList object containing all the matched Elements.    
*
*=================================================================*/
XML_NodeList *
xml_document_getElementsByTagName( XML_Document * doc,
                                   char *tagName )
{
	XML_NodeList *returnNodeList = NULL;

	if( ( doc == NULL ) || ( tagName == NULL ) ) {
		return NULL;
	}

	xml_node_getElementsByTagName( ( XML_Node * ) doc, tagName, &returnNodeList );
	return returnNodeList;
}

/*================================================================
*   xml_document_getElementsByTagNameNS
*       Returns a nodeList of all the Elements with a given local name and
*       namespace URI in the order in which they are encountered in a 
*       preorder traversal of the document tree.
*       External function.
*   Parameters:
*       namespaceURI: the namespace of the elements to match on. The special
*               value "*" matches all namespaces.
*       localName: the local name of the elements to match on. The special
*               value "*" matches all local names.
*   Return Value:
*       A new nodeList object containing all the matched Elements.    
*
*=================================================================*/
XML_NodeList *
xml_document_getElementsByTagNameNS( XML_Document * doc,
                                     char* namespaceURI,
                                     char* localName )
{
	XML_NodeList *returnNodeList = NULL;

	if( ( doc == NULL ) || ( namespaceURI == NULL )
			|| ( localName == NULL ) ) {
		return NULL;
	}

	xml_node_getElementsByTagNameNS( ( XML_Node * ) doc, namespaceURI, localName, &returnNodeList );
	return returnNodeList;
}

/*================================================================
*   xml_document_getElementById
*       Returns the element whose ID is given by tagName. If no such
*       element exists, returns null. 
*       External function.
*   Parameter:
*       tagName: the tag name for an element.
*   Return Values:
*       The matching element.
*
*=================================================================*/
XML_Element *
xml_document_getElementById( XML_Document * doc,
                             char* tagName )
{
	XML_Element *rtElement = NULL;
	XML_Node *nodeptr = ( XML_Node * ) doc;
	const char *name;

	if( ( nodeptr == NULL ) || ( tagName == NULL ) ) {
		return rtElement;
	}

	if( xml_node_getNodeType( nodeptr ) == eELEMENT_NODE ) {
		name = xml_node_getNodeName( nodeptr );
		if( name == NULL ) {
			return rtElement;
		}

		if( strcmp( tagName, name ) == 0 ) {
			rtElement = ( XML_Element * ) nodeptr;
			return rtElement;
		} else {
			rtElement = xml_document_getElementById( ( XML_Document * )
					xml_node_getFirstChild
					( nodeptr ),
					tagName );
			if( rtElement == NULL ) {
				rtElement = xml_document_getElementById( ( XML_Document * )
						xml_node_getNextSibling
						( nodeptr ),
						tagName );
			}
		}
	} else {
		rtElement = xml_document_getElementById( ( XML_Document * )
				xml_node_getFirstChild
				( nodeptr ), tagName );
		if( rtElement == NULL ) {
			rtElement = xml_document_getElementById( ( XML_Document * )
					xml_node_getNextSibling
					( nodeptr ),
					tagName );
		}
	}

	return rtElement;
}

/*================================================================
*	xml_document_dumpRecursive
*       It is a recursive function to print all the node in a tree.
*       Internal to parser only.
*
*=================================================================*/
void
xml_document_dumpRecursive( XML_Node * nodeptr,
                           xml_membuf * buf )
{
	char *nodeName = NULL;
	char *nodeValue = NULL;
	XML_Node *child = NULL,
					 *sibling = NULL;

	if( nodeptr != NULL ) {
		nodeName = ( char * )xml_node_getNodeName( nodeptr );
		nodeValue = xml_node_getNodeValue( nodeptr );

		switch ( xml_node_getNodeType( nodeptr ) ) {

			case eTEXT_NODE:
				copy_with_escape( buf, nodeValue );
				break;

			case eCDATA_SECTION_NODE:
				xml_membuf_append_str( buf, nodeValue );
				break;

			case ePROCESSING_INSTRUCTION_NODE:
				xml_membuf_append_str( buf, "<?" );
				xml_membuf_append_str( buf, nodeName );
				xml_membuf_append_str( buf, " " );
				xml_membuf_append_str( buf, nodeValue );
				xml_membuf_append_str( buf, "?>\n" );
				break;

			case eDOCUMENT_NODE:
				xml_document_dumpRecursive( xml_node_getFirstChild
						( nodeptr ), buf );
				break;

			case eATTRIBUTE_NODE:
				xml_membuf_append_str( buf, nodeName );
				xml_membuf_append_str( buf, "=\"" );
				if( nodeValue != NULL ) {
					xml_membuf_append_str( buf, nodeValue );
				}
				xml_membuf_append_str( buf, "\"" );
				if( nodeptr->nextSibling != NULL ) {
					xml_membuf_append_str( buf, " " );
					xml_document_dumpRecursive( nodeptr->nextSibling, buf );
				}
				break;

			case eELEMENT_NODE:
				xml_membuf_append_str( buf, "<" );
				xml_membuf_append_str( buf, nodeName );

				if( nodeptr->firstAttr != NULL ) {
					xml_membuf_append_str( buf, " " );
					xml_document_dumpRecursive( nodeptr->firstAttr, buf );
				}

				child = xml_node_getFirstChild( nodeptr );
				if( ( child != NULL )
						&& ( xml_node_getNodeType( child ) ==
							eELEMENT_NODE ) ) {
					xml_membuf_append_str( buf, ">\n" );
				} else {
					xml_membuf_append_str( buf, ">" );
				}

				//  output the children
				xml_document_dumpRecursive( xml_node_getFirstChild
						( nodeptr ), buf );

				// Done with children.  Output the end tag.
				xml_membuf_append_str( buf, "</" );
				xml_membuf_append_str( buf, nodeName );

				sibling = xml_node_getNextSibling( nodeptr );
				if( sibling != NULL
						&& xml_node_getNodeType( sibling ) == eTEXT_NODE ) {
					xml_membuf_append_str( buf, ">" );
				} else {
					xml_membuf_append_str( buf, ">\n" );
				}
				xml_document_dumpRecursive( xml_node_getNextSibling
						( nodeptr ), buf );
				break;

			default:
				break;
		}
	}
}

/*================================================================
*   xml_document_dump
*       Print a DOM tree.
*       Element, and Attribute nodes are handled differently.
*       We don't want to print the Element and Attribute nodes' sibling.
*       External function.
*
*=================================================================*/
void
xml_document_dump( XML_Node * nodeptr,
                  xml_membuf * buf )
{
	char *nodeName = NULL;
	char *nodeValue = NULL;
	XML_Node *child = NULL;

	if( ( nodeptr == NULL ) || ( buf == NULL ) ) {
		return;
	}

	nodeName = ( char * )xml_node_getNodeName( nodeptr );
	nodeValue = xml_node_getNodeValue( nodeptr );

	switch ( xml_node_getNodeType( nodeptr ) ) {

		case eTEXT_NODE:
		case eCDATA_SECTION_NODE:
		case ePROCESSING_INSTRUCTION_NODE:
		case eDOCUMENT_NODE:
			xml_document_dumpRecursive( nodeptr, buf );
			break;

		case eATTRIBUTE_NODE:
			xml_membuf_append_str( buf, nodeName );
			xml_membuf_append_str( buf, "=\"" );
			xml_membuf_append_str( buf, nodeValue );
			xml_membuf_append_str( buf, "\"" );
			break;

		case eELEMENT_NODE:
			xml_membuf_append_str( buf, "<" );
			xml_membuf_append_str( buf, nodeName );

			if( nodeptr->firstAttr != NULL ) {
				xml_membuf_append_str( buf, " " );
				xml_document_dumpRecursive( nodeptr->firstAttr, buf );
			}

			child = xml_node_getFirstChild( nodeptr );
			if( ( child != NULL )
					&& ( xml_node_getNodeType( child ) == eELEMENT_NODE ) ) {
				xml_membuf_append_str( buf, ">\n" );
			} else {
				xml_membuf_append_str( buf, ">" );
			}

			//  output the children
			xml_document_dumpRecursive( xml_node_getFirstChild( nodeptr ),
					buf );

			// Done with children.  Output the end tag.
			xml_membuf_append_str( buf, "</" );
			xml_membuf_append_str( buf, nodeName );
			xml_membuf_append_str( buf, ">\n" );
			break;

		default:
			break;
	}
}

/*================================================================
*   xml_document_toString
*       Converts a DOM tree into a text string
*       Element, and Attribute nodes are handled differently.
*       We don't want to print the Element and Attribute nodes' sibling.
*       External function.
*
*=================================================================*/
void
xml_document_toString( XML_Node * nodeptr,
                     xml_membuf * buf )
{
	char *nodeName = NULL;
	char *nodeValue = NULL;
	XML_Node *child = NULL;

	if( ( nodeptr == NULL ) || ( buf == NULL ) ) {
		return;
	}

	nodeName = ( char * )xml_node_getNodeName( nodeptr );
	nodeValue = xml_node_getNodeValue( nodeptr );

	switch ( xml_node_getNodeType( nodeptr ) ) {

		case eTEXT_NODE:
		case eCDATA_SECTION_NODE:
		case ePROCESSING_INSTRUCTION_NODE:
		case eDOCUMENT_NODE:
			xml_document_dumpRecursive( nodeptr, buf );
			break;

		case eATTRIBUTE_NODE:
			xml_membuf_append_str( buf, nodeName );
			xml_membuf_append_str( buf, "=\"" );
			xml_membuf_append_str( buf, nodeValue );
			xml_membuf_append_str( buf, "\"" );
			break;

		case eELEMENT_NODE:
			xml_membuf_append_str( buf, "<" );
			xml_membuf_append_str( buf, nodeName );

			if( nodeptr->firstAttr != NULL ) {
				xml_membuf_append_str( buf, " " );
				xml_document_dumpRecursive( nodeptr->firstAttr, buf );
			}

			child = xml_node_getFirstChild( nodeptr );
			if( ( child != NULL )
					&& ( xml_node_getNodeType( child ) == eELEMENT_NODE ) ) {
				xml_membuf_append_str( buf, ">" );
			} else {
				xml_membuf_append_str( buf, ">" );
			}

			//  output the children
			xml_document_dumpRecursive( xml_node_getFirstChild( nodeptr ),
					buf );

			// Done with children.  Output the end tag.
			xml_membuf_append_str( buf, "</" );
			xml_membuf_append_str( buf, nodeName );
			xml_membuf_append_str( buf, ">" );
			break;

		default:
			break;
    }
}

/*================================================================
*   xml_document_loadEx
*       Parses the given file, and returns the DOM tree from it.
*       External function.
*
*=================================================================*/
int
xml_document_loadEx( char *xmlFile,
                    XML_Document ** doc )
{
	if( ( xmlFile == NULL ) || ( doc == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	return xml_parser_LoadDocument( doc, xmlFile, TRUE );
}

/*================================================================
 *   xml_document_load
 *       Parses the given file, and returns the DOM tree from it.
 *       External function.
 *
 *=================================================================*/
XML_Document *
xml_document_load( char *xmlFile )
{
	XML_Document *doc = NULL;

	xml_document_loadEx( xmlFile, &doc );
	return doc;
}

/*================================================================
*   xml_document_parseEx
*       Parse xml file stored in buffer.
*       External function.
*
*=================================================================*/
int
xml_document_parseEx( char *buffer,
                   XML_Document ** retDoc )
{
	if( ( buffer == NULL ) || ( retDoc == NULL ) ) {
		return XML_INVALID_PARAMETER;
	}

	if( strlen( buffer ) == 0 ) {
		return XML_INVALID_PARAMETER;
	}

	return xml_parser_LoadDocument( retDoc, buffer, FALSE );
}

/*================================================================
*   xml_document_parse
*       Parse xml file stored in buffer.
*       External function.
*
*=================================================================*/
XML_Document *
xml_document_parse( char *buffer )
{
	XML_Document *doc = NULL;

	xml_document_parseEx( buffer, &doc );
	return doc;
}
