#ifndef _XML_MAP_H_
#define _XML_MAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap from which to retrieve the size
 * 
 * @return 
 */
unsigned long xml_nodemap_getSize(XML_NamedNodeMap *nnMap);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap to search
 * @param name The name of the Node to find.
 * 
 * @return 
 */
XML_Node* xml_nodemap_getNamedItem(XML_NamedNodeMap *nnMap, char* name);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap in which to add the new Node
 * @param arg The new Node to add to the NamedNodeMap
 * 
 * @return 
 */
XML_Node* xml_nodemap_setNamedItem(XML_NamedNodeMap *nnMap, XML_Node *arg);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap from which to remove the item
 * @param name The name of the item to remove
 * 
 * @return 
 */
XML_Node* xml_nodemap_removeNamedItem(XML_NamedNodeMap *nnMap, char* name);

/** 
 * @brief 
 * 
 * @param *nnMap 
 * @param add 
 * 
 * @return 
 */
int xml_nodemap_addItem(XML_NamedNodeMap **nnMap, XML_Node *add);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap from which to remove the Node
 * @param index The index into the map to remove
 * 
 * @return 
 */
XML_Node* xml_nodemap_getItem(XML_NamedNodeMap *nnMap, unsigned long index);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap from which to remove the Node
 * @param namespaceURI The namespace URI of the Node to remove
 * @param localName The local name of the Node to remove
 * 
 * @return 
 */
XML_Node* xml_nodemap_getNamedItemNS(XML_NamedNodeMap *nnMap, char* *namespaceURI, char* localName);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap in which to add the Node
 * @param arg The Node to add to the map
 * 
 * @return 
 */
XML_Node* xml_nodemap_setNamedItemNS(XML_NamedNodeMap *nnMap, XML_Node *arg);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap from which to remove the Node
 * @param namespaceURI The namespace URI of the Node to remove
 * @param localName The local name of the Node to remove
 * 
 * @return 
 */
XML_Node* xml_nodemap_removeNamedItemNS(XML_NamedNodeMap *nnMap, char* namespaceURI, char* localName);

/** 
 * @brief 
 * 
 * @param nnMap The NamedNodeMap to free
 */
void xml_nodemap_free(XML_NamedNodeMap *nnMap);

#ifdef __cplusplus
}
#endif

#endif

