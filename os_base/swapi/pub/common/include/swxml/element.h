#ifndef _XML_ELEMENT_H_
#define _XML_ELEMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief 
 * 
 * @param element The Element from which to retrieve the name
 * 
 * @return 
 */
const char* xml_element_getTagName(XML_Element* element);

/** 
* @brief 
* 
* @param element 
* @param tagName 
* 
* @return 
*/
int xml_element_setTagName(XML_Element *element, char *tagName);

/** 
 * @brief 
 * 
 * @param element The Element from which to retrieve the attribute
 * @param name The name of the attribute to retrieve
 * 
 * @return 
 */
char* xml_element_getAttribute(XML_Element* element, char* name);

/** 
 * @brief 
 * 
 * @param element The Element on which to set the attribute
 * @param name The name of the attribute
 * @param value The value of the attribute.  Note that this is a non-parsed string and any markup must be escaped
 * 
 * @return 
 */
int xml_element_setAttribute(XML_Element* element, char* name, char* value);

/** 
 * @brief 
 * 
 * @param element The Element from which to remove the attribute
 * @param name The name of the attribute to remove
 * 
 * @return 
 */
int xml_element_removeAttribute(XML_Element* element, char* name);              

/** 
 * @brief 
 * 
 * @param element The Element from which to get the attribute node
 * @param name The name of the attribute node to find
 * 
 * @return 
 */
XML_Attr* xml_element_getAttributeNode(XML_Element* element, char* name);

/** 
 * @brief 
 * 
 * @param element The Element in which to add the new attribute
 * @param newAttr The new {\bf Attr} to add
 * @param rtAttr A pointer to an {\bf Attr} where the old Attr will be stored.  This will have  
 *									a NULL if no prior node existed
 * 
 * @return 
 */
int xml_element_setAttributeNode(XML_Element* element, XML_Attr* newAttr, XML_Attr** rtAttr);

/** 
 * @brief 
 * 
 * @param element The Element from which to remove the attribute
 * @param oldAttr The attribute to remove from the Element
 * @param rtAttr A pointer to an attribute in which to place the removed attribute
 * 
 * @return 
 */
int xml_element_removeAttributeNode(XML_Element* element, XML_Attr* oldAttr, XML_Attr** rtAttr);

/** 
 * @brief 
 * 
 * @param element The Element from which to start the search
 * @param tagName The name of the tag for which to search
 * 
 * @return 
 */
XML_NodeList* xml_element_getElementsByTagName(XML_Element* element, char* tagName);

/** 
 * @brief 
 * 
 * @param element The Element from which to get the attribute value
 * @param namespaceURI The namespace URI of the attribute
 * @param localname The local name of the attribute
 * 
 * @return 
 */
char* xml_element_getAttributeNS(XML_Element* element, char* namespaceURI, char* localname);

/** 
 * @brief 
 * 
 * @param element The Element on which to set the attribute
 * @param namespaceURI The namespace URI of the new attribute
 * @param qualifiedName The qualified name of the attribute
 * @param value The new value for the attribute
 * 
 * @return 
 */
int xml_element_setAttributeNS(XML_Element* element, char* namespaceURI, char* qualifiedName, char* value);

/** 
 * @brief 
 * 
 * @param element The Element from which to remove the the attribute
 * @param namespace URI The namespace URI of the attribute
 * @param localName The local name of the attribute
 * 
 * @return 
 */
int xml_element_removeAttributeNS(XML_Element* element, char* namespaceURI, char* localName);

/** 
 * @brief 
 * 
 * @param element The Element from which to get the attribute
 * @param namespaceURI The namespace URI of the attribute
 * @param localName The local name of the attribute
 * 
 * @return 
 */
XML_Attr* xml_element_getAttributeNodeNS(XML_Element* element, char* namespaceURI, char* localName);

/** 
 * @brief 
 * 
 * @param element The Element in which to add the attribute node
 * @param newAttr The new Attr to add
 * @param rcAttr A pointer to the replaced Attr, if it exists
 * 
 * @return 
 */
int xml_element_setAttributeNodeNS(XML_Element* element, XML_Attr* newAttr, XML_Attr** rcAttr);

/** 
 * @brief 
 * 
 * @param element The Element from which to start the search
 * @param namespaceURI The namespace URI of the Elements to find
 * @param localName The local name of the Elements to find
 * 
 * @return 
 */
XML_NodeList* xml_element_getElementsByTagNameNS(XML_Element* element, char* namespaceURI, char* localName);

/** 
 * @brief 
 * 
 * @param element The Element on which to check for an attribute
 * @param name The name of the attribute for which to check
 * 
 * @return 
 */
BOOL xml_element_hasAttribute(XML_Element* element, char* name);

/** 
 * @brief 
 * 
 * @param element The Element on which to check for the attribute
 * @param namespaceURI The namespace URI of the attribute
 * @param localNameThe local name of the attribute 
 * 
 * @return 
 */
BOOL xml_element_hasAttributeNS(XML_Element* element, char* namespaceURI, char* localName);

#ifdef __cplusplus
}
#endif

#endif

