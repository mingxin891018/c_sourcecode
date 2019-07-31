#ifndef _XML_DOCUMENT_H_
#define _XML_DOCUMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief 
 * 
 * @param doc 
 * 
 * @return 
 */
int xml_document_createDocumentEx(XML_Document** doc);

/** 
 * @brief 
 * 
 * @return 
 */
XML_Document* xml_document_createDocument();

/** 
 * @brief 
 * 
 * @param tagName The tag name of the new Element node
 * @param *rtElement Pointer to an Element where the new object will be stored
 * 
 * @return 
 */
int xml_document_createElementEx(const char* tagName,  XML_Element **rtElement);

/** 
 * @brief 
 * 
 * @param tagName The tag name of the new Element node
 * 
 * @return 
 */
XML_Element* xml_document_createElement(const char* tagName);

/** 
 * @brief 
 * 
 * @param data The data to associate with the new {\bf Text} node
 * @param textNode A pointer to a Node where the new object will be stored
 * 
 * @return 
 */
int xml_document_createTextNodeEx(const char* data, XML_Node** textNode );

/** 
 * @brief 
 * 
 * @param data The data to associate with the new {\bf Text} node
 * 
 * @return 
 */
XML_Node* xml_document_createTextNode(const char* data);

/** 
 * @brief 
 * 
 * @param data The data to associate with the new CDATASection node
 * @param cdNode A pointer to a Node where the new object will be stored
 * 
 * @return 
 */
int xml_document_createCDATASectionEx(char* data, XML_CDATASection** cdNode);

/** 
 * @brief 
 * 
 * @param data The data to associate with the new CDATASection node
 * 
 * @return 
 */
XML_CDATASection* xml_document_createCDATASection(char* data);

/** 
 * @brief 
 * 
 * @param name The name of the new attribute
 * 
 * @return 
 */
XML_Attr* xml_document_createAttribute(char *name);

/** 
 * @brief 
 * 
 * @param name The name of the new attribute
 * @param attrNode A pointer to a Attr where the new object will be stored
 * 
 * @return 
 */
int xml_document_createAttributeEx(char *name, XML_Attr** attrNode);

/** 
 * @brief 
 * 
 * @param doc The Document to search
 * @param tagName The tag name to find
 * 
 * @return 
 */
XML_NodeList* xml_document_getElementsByTagName(XML_Document *doc, char* tagName);

/** 
 * @brief 
 * 
 * @param namespaceURI The namespace URI for the new Element
 * @param qualifiedName The qualified name of the new Element
 * @param rtElement A pointer to an Element where the new object will be stored
 * 
 * @return 
 */
int xml_document_createElementNSEx(char* namespaceURI, char* qualifiedName, XML_Element** rtElement);

/** 
 * @brief 
 * 
 * @param namespaceURI The namespace URI for the new Element
 * @param qualifiedName The qualified name of the new Element
 * 
 * @return 
 */
XML_Element* xml_document_createElementNS(char* namespaceURI, char* qualifiedName);

/** 
 * @brief 
 * 
 * @param namespaceURI The namespace URI for the attribute
 * @param qualifiedName The qualified name of the attribute
 * @param attrNode A pointer to an Attr where the new object will be stored
 * 
 * @return 
 */
int xml_document_createAttributeNSEx(char* namespaceURI, char* qualifiedName, XML_Attr** attrNode);   

/** 
 * @brief 
 * 
 * @param namespaceURI The namespace URI for the attribute
 * @param qualifiedName The qualified name of the attribute
 * 
 * @return 
 */
XML_Attr* xml_document_createAttributeNS(char* namespaceURI, char* qualifiedName);   

/** 
 * @brief 
 * 
 * @param doc The Document to search
 * @param namespaceURI The namespace of the elements to find or "*" to match any namespace
 * @param localName The local name of the elements to find or "*" to match any local name
 * 
 * @return 
 */
XML_NodeList* xml_document_getElementsByTagNameNS(XML_Document* doc, char* namespaceURI, char* localName);

/** 
 * @brief 
 * 
 * @param doc The owner Document of the Element
 * @param tagName The name of the Element
 * 
 * @return 
 */
XML_Element* xml_document_getElementById(XML_Document* doc, char* tagName);

/** 
 * @brief 
 * 
 * @param doc The root of the Node tree to render to XML text
 * 
 * @return 
 */
char* xml_node_dump(XML_Node *doc);

/** 
 * @brief 
 * 
 * @param doc The root of the Node tree to render to XML text
 * 
 * @return 
 */
char* xml_node_toString(XML_Node *doc);

/** 
 * @brief 
 * 
 * @param errorChar 
 */
void xml_parser_relax(char errorChar);

/** 
 * @brief 
 * 
 * @param buffer The buffer that contains the XML text to convert to a Document
 * 
 * @return 
 */
XML_Document* xml_document_parse(char *buffer);

XML_Document *xml_document_parse_unclosed( char *buffer );

/** 
 * @brief 
 * 
 * @param buffer The buffer that contains the XML text to convert to a Document
 * @param doc A point to store the Document if file correctly parses or NULL on an error
 * 
 * @return 
 */
int xml_document_parseEx(char *buffer, XML_Document** doc);

/** 
 * @brief 
 * 
 * @param xmlFile The filename of the XML text to convert to a Document
 * 
 * @return 
 */
XML_Document* xml_document_load(char* xmlFile);

/** 
 * @brief 
 * 
 * @param xmlFile The filename of the XML text to convert to a Document
 * @param doc A pointer to the Document if file correctly parses or NULL on an error
 * 
 * @return 
 */
int xml_document_loadEx(char* xmlFile, XML_Document** doc);

/** 
 * @brief 
 * 
 * @param nodeptr
 * @param buf
 * 
 * @return 
 */
void xml_document_dump(XML_Node* nodeptr, xml_membuf* buf);

/** 
 * @brief 
 * 
 * @param nodeptr
 * @param buf
 * 
 * @return 
 */
void xml_document_toString( XML_Node * nodeptr, xml_membuf * buf );

#ifdef __cplusplus
}
#endif

#endif

