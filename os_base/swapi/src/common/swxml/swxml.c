#include "swapi.h"
#include "swmem.h"
#include "membuf.h"
#include "swxml.h"
#include "document.h"
#include "node.h"
#include "nodelist.h"
#include "element.h"

/* 从字符串缓存区数据生成节点 */
XML_Node* sw_xml_toNode( const char* buffer )
{
	return xml_document_parse( (char *)buffer );
}

/* 保存当前XML结点数据到字符串缓存区 */
char* sw_xml_toString( XML_Node* node )
{
	char* buffer = xml_node_toString( node );
	
	if( buffer == NULL )
		return strdup("");

	return buffer;
}

/* 创建节点 */
int sw_xml_createNode( const char* ns, const char* name, const char* value, XML_Node** result )
{
	XML_Node* node = NULL;
	char furname[128] = { 0 };

	if( name == NULL )
	{
		printf("[%s] line %d\n", __FILE__, __LINE__);
		return -1;
	}
	if( ns != NULL )
	{
		strlcat( furname, ns, sizeof(furname));
		strlcat( furname, ":", sizeof(furname) );
	}
	strlcat( furname, name, sizeof(furname) );
	node = (XML_Node *)xml_document_createElement( furname );
	if( node == NULL )
	{
		printf("[%s] line %d\n", __FILE__, __LINE__);
		return -1;
	}
	if( ns != NULL )
	{
		if( XML_SUCCESS != xml_node_setPrefix( node, (char *)ns ) )
		{
			printf("[%s] line %d\n", __FILE__, __LINE__);
			return -1;
		}
	}
	if( XML_SUCCESS != xml_node_setLocalName( node, (char *)name ) )
	{
		printf("[%s] line %d\n", __FILE__, __LINE__);
		return -1;
	}
	*result = node;
	if( value != NULL )
	{
		node = (XML_Node *)xml_document_createTextNode( value );
		if( node == NULL )
		{
			printf("[%s] line %d\n", __FILE__, __LINE__);
			xml_node_free( *result );
			return -1;
		}

		if( XML_SUCCESS != xml_node_appendChild( *result, node ) )
		{
			xml_node_free( *result );
			xml_node_free( node );
			return -1;
		}
	}
	
	return 0;
}

/* 释放节点 */
void sw_xml_freeNode( XML_Node* node )
{
	xml_node_free( node );
}

/* 为某结点添加孩子结点 */
int sw_xml_appendChildNode( XML_Node* parent, XML_Node* node )
{
	return xml_node_appendChild( parent, node );
}

/* 为某结点添加属性 */
int sw_xml_addAttribute( XML_Node* node, const char* ns, const char* name, const char* value )
{
	char ns_name[32];

	if ( ns != NULL && name != NULL ) 
	{
		memset(ns_name, 0, sizeof(ns_name));
		snprintf(ns_name, sizeof(ns_name), "%s:%s", ns, name);
		if( XML_SUCCESS != xml_element_setAttribute( node, (char *)ns_name, (char *)value ) )
			return -1;
	} else {
		if( XML_SUCCESS != xml_element_setAttribute( node, (char *)name, (char *)value ) )
			return -1;
	}

	return 0;
}

/* 得到某结点的属性值 */
int sw_xml_getNodeValue( XML_Node* node, XML_NODE_TYPE* type, const char** ns, /*const*/ char** name, /*const*/ char** value )
{
	XML_Node* child = NULL;

	if( node == NULL )
	{
		printf("[%s] line %d\n", __FILE__, __LINE__);
		return -1;
	}

	if( ns != NULL )
		*ns = node->prefix;
	if( type != NULL )
		*type = node->nodeType;
	if( name != NULL )
		*name = node->localName;
	if( value != NULL )
	{
		child = xml_node_getFirstChild( node );
		if( child != NULL )
			*value = child->nodeValue;
		else
			*value = NULL;
	}

	return 0;
}

/* 通过url描述得到一个结点 */
int sw_xml_getNodeByUrl( XML_Node* root, const char* url, XML_Node** node )
{
	int result = -1;
	XML_Node* n = root;
	char path[128] = {0}, path1[128] = {0};
	char* p=NULL, *q = NULL;

	if( root->localName )
		strlcpy( path, root->localName, sizeof(path));
	while( n->parentNode )
	{
		n = n->parentNode;
		if( n->localName )
		{
			strlcpy( path1, n->localName, sizeof(path1));
			strlcat( path1, ".", sizeof(path1));
			strlcat( path1, path, sizeof(path1));
			strlcpy( path, path1, sizeof(path));
		}
	}
	p = path;
	while((q=strstr(p,url)) != NULL)
	{
		if((p==q||*(q-1)=='.')&&(*(q+strlen(url))=='\0' ||*(q+strlen(url))=='.'))
		{
			*node = root;
			return 0;
		}
		p=strstr(q,".");
		if(p)
			p++;
		else
			break;
	}

	if( root->firstChild )
		result = sw_xml_getNodeByUrl( root->firstChild, url, node );

	if( result == 0 )
		return result;

	if( root->nextSibling )
		result = sw_xml_getNodeByUrl( root->nextSibling, url, node );

	if( result == 0 )
		return result;

	return -1;

}

/* 根据url得到该结点的值 */
int sw_xml_getValueByUrl( XML_Node* root, const char* url, char** value )
{
	XML_Node* node = NULL;
	if( sw_xml_getNodeByUrl( root, url, &node ) != 0 )
	{
		printf("[%s] line %d\n", __FILE__, __LINE__);
		return -1;
	}
	return sw_xml_getNodeValue( node, NULL, NULL, NULL, value );
}

/* 得到某结点拥有孩子结点的数量值 */
int sw_xml_getNodeChildNum( XML_Node* node, int* count )
{
	XML_NodeList* list = xml_node_getChildNodes( node );
	XML_NodeList* item = list;
	int num = 0;
	
	while( item != NULL )
	{
		num++;
		item = item->next;
	}
	xml_nodelist_clear( list );

	*count = num;

	return 0;
}

/* 得到某结点的第一个孩子几点 */
XML_Node* sw_xml_getNodeFirstChild( XML_Node* node )
{
	return xml_node_getFirstChild( node );
}

/* 得到某结点的第一个兄弟结点 */
XML_Node* sw_xml_getNodeNextSibling( XML_Node* node )
{
	return xml_node_getNextSibling( node );
}

