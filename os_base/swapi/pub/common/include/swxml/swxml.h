#ifndef __SW_XML_H__
#define __SW_XML_H__

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/* Node Type */
typedef enum
{
	eINVALID_NODE                   = 0,
	eELEMENT_NODE                   = 1,
	eATTRIBUTE_NODE                 = 2,
	eTEXT_NODE                      = 3,
	eCDATA_SECTION_NODE             = 4,
	eENTITY_REFERENCE_NODE          = 5,
	eENTITY_NODE                    = 6,                
	ePROCESSING_INSTRUCTION_NODE    = 7,
	eCOMMENT_NODE                   = 8,
	eDOCUMENT_NODE                  = 9,
	eDOCUMENT_TYPE_NODE             = 10,
	eDOCUMENT_FRAGMENT_NODE         = 11,
	eNOTATION_NODE                  = 12
} XML_NODE_TYPE;

/* Error */
typedef enum 
{
	XML_INDEX_SIZE_ERR                 = 1,
	XML_DOMSTRING_SIZE_ERR             = 2,
	XML_HIERARCHY_REQUEST_ERR          = 3,
	XML_WRONG_DOCUMENT_ERR             = 4,
	XML_INVALID_CHARACTER_ERR          = 5,
	XML_NO_DATA_ALLOWED_ERR            = 6,
	XML_NO_MODIFICATION_ALLOWED_ERR    = 7,
	XML_NOT_FOUND_ERR                  = 8,
	XML_NOT_SUPPORTED_ERR              = 9,
	XML_INUSE_ATTRIBUTE_ERR            = 10,
	XML_INVALID_STATE_ERR              = 11,
	XML_SYNTAX_ERR                     = 12,
	XML_INVALID_MODIFICATION_ERR       = 13,
	XML_NAMESPACE_ERR                  = 14,
	XML_INVALID_ACCESS_ERR             = 15,

	XML_SUCCESS                        = 0,
	XML_NO_SUCH_FILE                   = 101,
	XML_INSUFFICIENT_MEMORY            = 102,
	XML_FILE_DONE                      = 104,
	XML_INVALID_PARAMETER              = 105,
	XML_FAILED                         = 106,
	XML_INVALID_ITEM_NUMBER            = 107
} XML_ERRORCODE;

#define DOCUMENTNODENAME    "#document"
#define TEXTNODENAME        "#text"
#define CDATANODENAME       "#cdata-section"

typedef struct _XML_Document	*Docptr;
typedef struct _XML_Node			*Nodeptr;

typedef struct _XML_Node
{
	char* nodeName;
	char* nodeValue;
	XML_NODE_TYPE nodeType;
	char* namespaceURI;
	char* prefix;
	char* localName;
	BOOL readOnly;
	Nodeptr parentNode;
	Nodeptr firstChild;
	Nodeptr prevSibling;
	Nodeptr nextSibling;
	Nodeptr firstAttr;

	//Element
	char* tagName;

	//Attribute
	BOOL specified;
	Nodeptr owner;
} XML_Node;

typedef XML_Node XML_Document;
typedef XML_Node XML_CDATASection;
typedef XML_Node XML_Element;
typedef XML_Node XML_Attr;

typedef struct _XML_NodeList
{
	Nodeptr nodeItem;
	struct _XML_NodeList *next;
} XML_NodeList;

typedef struct _XML_NamedNodeMap
{
	Nodeptr nodeItem;
	struct _XML_NamedNodeMap *next;
} XML_NamedNodeMap;

//extern HANDLE g_xml_mem;

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * @brief parser xml 严格按照xml语法节点都需要闭合
 * 		需要调用sw_xml_freeNode释放其所有的节点内存
 * 
 * @param buffer 
 * 
 * @return 
 */
XML_Node* sw_xml_toNode( const char* buffer );
/**
 * @brief 如果节点只有一层可以不用闭合
 */
XML_Node* sw_xml_toNode_UnClosed( const char* buffer);

/**
 * @brief 将xml节点转为字符串，需要调用放释放内存
 */
char* sw_xml_toString( XML_Node* node );

/** 
 * @brief create new xml node--用于组装xml文件
 * 
 * @param ns 
 * @param name 
 * @param value 
 * @param result 
 * 
 * @return 
 */
int sw_xml_createNode( const char* ns, const char* name, const char* value, XML_Node** result );

/** 
 * @brief frees all nodes under nodeptr subtree
 * 
 * @param node 
 */
void sw_xml_freeNode( XML_Node* node );

/** 
 * @brief --用于组装xml文件
 * 
 * @param parent 
 * @param node 
 * 
 * @return 
 */
int sw_xml_appendChildNode( XML_Node* parent, XML_Node* node );

/** 
 * @brief --用于组装xml文件
 * 
 * @param node 
 * @param ns 
 * @param name 
 * @param value 
 * 
 * @return 
 */
int sw_xml_addAttribute( XML_Node* node, const char* ns, const char* name, const char* value );

/** 
 * @brief---------must do not call free
 * 
 * @param node 
 * @param type <-------- node->nodeType
 * @param ns <-----------node->prefix
 * @param name <---------node->localName
 * @param value <--------node->firstChild->nodeValue;
 * 
 * @return 
 */
int sw_xml_getNodeValue( XML_Node* node, XML_NODE_TYPE* type, const char** ns, /*const*/ char** name, /*const*/ char** value );

/** 
 * @brief 从根节点拼接地址root.FRST.SECD.THRD.FOUTH.FIVE.**拼接到当前节点然后搜索对比url地址,如果节点名有重复最好是拼全url地址
 * 
 * @param root 最好传的是跟节点地址
 * @param url<------SECD.THRD
 * @param node 
 * 
 * @return 
 */
int sw_xml_getNodeByUrl( XML_Node* root, const char* url, XML_Node** node );

/** 
 * @brief 通过sw_xml_getNodeByUrl查找到对应的节点取出其节点的第一子节点nodeValue
 * 
 * @param root 
 * @param url 
 * @param value -----must do not call free
 * 
 * @return 
 */
int sw_xml_getValueByUrl( XML_Node* root, const char* url, char** value );

/** 
 * @brief 获取当前节点的子节点数
 * 
 * @param node
 * @param count 
 * 
 * @return 
 */
int sw_xml_getNodeChildNum( XML_Node* node, int* count );

/** 
 * @brief 获取第一子节点
 * 
 * @param node 
 * 
 * @return 
 */
XML_Node* sw_xml_getNodeFirstChild( XML_Node* node );

/** 
 * @brief 获取当前节点的兄弟节点
 * 
 * @param node 
 * 
 * @return 
 */
XML_Node* sw_xml_getNodeNextSibling( XML_Node* node );

/** 
 * @brief 获取当前节点的第一属性节点,后续的属性可以用兄弟节点获取出来
 * 
 * @param node 
 * 
 * @return 
 */
XML_Node* sw_xml_getNodeFirstAttr( XML_Node* node );

/** 
 * @brief 查找当前节点的url对应的属性值域
 * 
 * @param node 
 * @param url
 * @param ns <-----------n->prefix
 * @param value <--------n->nodeValue, must do not call free
 * 
 * @return 
 */
int sw_xml_getNodeAttrByUrl( XML_Node* node, const char *url, char **ns, char **value);

#ifdef __cplusplus
}
#endif

#endif
