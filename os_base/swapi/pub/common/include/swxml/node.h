#ifndef _XML_NODE_H_
#define _XML_NODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief 
 * 
 * @param nodeptr Pointer to the node to retrieve the name
 * 
 * @return 
 */
const char* xml_node_getNodeName(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr Pointer to the Node to retrieve the value
 * 
 * @return 
 */
char* xml_node_getNodeValue(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node to which to assign a new value
 * @param newNodeValue The new value of the Node
 * 
 * @return 
 */
int xml_node_setNodeValue(XML_Node *nodeptr, char *newNodeValue);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the type
 * 
 * @return 
 */
unsigned short xml_node_getNodeType(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the parent
 * 
 * @return 
 */
XML_Node* xml_node_getParentNode(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the children
 * 
 * @return 
 */
XML_NodeList* xml_node_getChildNodes(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the first child
 * 
 * @return 
 */
XML_Node* xml_node_getFirstChild(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the last child
 * 
 * @return 
 */
XML_Node* xml_node_getLastChild(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node for which to retrieve the previous sibling
 * 
 * @return 
 */
XML_Node* xml_node_getPreviousSibling(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the next sibling
 * 
 * @return 
 */
XML_Node* xml_node_getNextSibling(XML_Node *nodeptr);
XML_Node* xml_node_getFirstAttr( XML_Node * nodeptr );
/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the attributes
 * 
 * @return 
 */
XML_NamedNodeMap* xml_node_getAttributes(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node for which to retrieve the namespace
 * 
 * @return 
 */
const char* xml_node_getNamespaceURI(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the prefix
 * 
 * @return 
 */
char* xml_node_getPrefix(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to set the prefix
 * @param prefix the prefix value
 * 
 * @return 
 */
int xml_node_setPrefix( XML_Node * nodeptr, char *prefix );

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the local name
 * 
 * @return 
 */
const char* xml_node_getLocalName(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node from which to retrieve the local name
 * @param localName The localName value
 * 
 * @return 
 */
int xml_node_setLocalName( XML_Node * nodeptr, char *localName );


/** 
 * @brief 
 * 
 * @param nodeptr The parent of the Node before which to insert the new child
 * @param newChild The Node to insert into the tree
 * @param refChild The reference child where the new Node should be inserted.
 *									The new Node will appear directly before the reference child
 * 
 * @return 
 */
int xml_node_insertBefore(XML_Node *nodeptr, XML_Node* newChild, XML_Node* refChild);

/** 
 * @brief 
 * 
 * @param nodeptr The parent of the Node which contains the child to replace
 * @param newChild The child with which to replace oldChild
 * @param oldChild The child to replace with newChild
 * @param returnNode Pointer to a Node to place the removed oldChild Node
 * 
 * @return 
 */
int xml_node_replaceChild(XML_Node *nodeptr, XML_Node* newChild, XML_Node* oldChild, XML_Node** returnNode);

/** 
 * @brief 
 * 
 * @param nodeptr The parent of the child to remove
 * @param oldChild The child Node to remove
 * @param *returnNode Pointer to a Node to place the removed oldChild Node
 * 
 * @return 
 */
int xml_node_removeChild(XML_Node *nodeptr, XML_Node* oldChild, XML_Node **returnNode);

/** 
 * @brief 
 * 
 * @param nodeptr The Node in which to append the new child
 * @param newChild The new child to append
 * 
 * @return 
 */
int xml_node_appendChild(XML_Node *nodeptr, XML_Node* newChild);

/** 
 * @brief 
 * 
 * @param nodeptr The Node to query for children
 * 
 * @return 
 */
BOOL xml_node_hasChildNodes(XML_Node *nodeptr);

/** 
 * @brief 
 * 
 * @param nodeptr The Node to clone
 * @param deep TRUE to clone the subtree also or FALSE to clone only nodeptr
 * 
 * @return 
 */
XML_Node* xml_node_cloneNode(XML_Node *nodeptr, BOOL deep);

/** 
 * @brief 
 * 
 * @param node The Node to query for attributes
 * 
 * @return 
 */
BOOL xml_node_hasAttributes(XML_Node *node);

/** 
 * @brief 
 * 
 * @param XML_Node The Node to free
 */
void xml_node_free(XML_Node *XML_Node);

/** 
 * @brief 
 * 
 * @param srcXML_Node 
 * @param destXML_Node 
 * 
 * @return 
 */
BOOL xml_node_compare(XML_Node *srcXML_Node, XML_Node *destXML_Node);

/** 
* @brief 
* 
* @param n 
* @param tagname 
* @param *list 
*/
void xml_node_getElementsByTagName( XML_Node *n, char *tagname, XML_NodeList **list);

/** 
 * @brief 
 * 
 * @param XML_Node 
 * @param namespaceURI 
 * @param localName 
 * @param *list 
 */
void xml_node_getElementsByTagNameNS( XML_Node *XML_Node, char *namespaceURI, char *localName, XML_NodeList **list);

/** 
 * @brief 
 * 
 * @param node 
 * @param src 
 * 
 * @return 
 */
int xml_node_setNodeProperties(XML_Node* node, XML_Node *src);

/** 
 * @brief 
 * 
 * @param node 
 * @param qualifiedName 
 * 
 * @return 
 */
int xml_node_setNodeName( XML_Node* node, char* qualifiedName);

/** 
 * @brief 
 * 
 * @param node 
 * 
 * @return 
 */
char* xml_node_dump( XML_Node * node );

/** 
 * @brief 
 * 
 * @param node 
 * 
 * @return 
 */
char* xml_node_toString( XML_Node * node );

#ifdef __cplusplus
}
#endif

#endif

