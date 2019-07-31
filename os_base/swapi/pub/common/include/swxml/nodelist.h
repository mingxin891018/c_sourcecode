#ifndef _XML_NODELIST_H_
#define _XML_NODELIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief 
 * 
 * @param nList The NodeList from which to retrieve the Node
 * @param index The index into the NodeList to retrieve
 * 
 * @return 
 */
XML_Node* xml_nodelist_getItem(XML_NodeList *nList, unsigned long index);

/** 
 * @brief 
 * 
 * @param *nList 
 * @param add 
 * 
 * @return 
 */
int xml_nodelist_addItem(XML_NodeList **nList, XML_Node *add);

/** 
 * @brief 
 * 
 * @param nList The NodeList for which to retrieve the number of Nodes
 * 
 * @return 
 */
unsigned long xml_nodelist_getSize(XML_NodeList *nList);

/** 
 * @brief 
 * 
 * @param nList The NodeList to free
 */
void xml_nodelist_clear(XML_NodeList *nList);

#ifdef __cplusplus
}
#endif

#endif

