#ifndef _IXMLPARSER_H
#define _IXMLPARSER_H

// Parser definitions
#define QUOT        "&quot;"
#define LT          "&lt;"
#define GT          "&gt;"
#define APOS        "&apos;"
#define AMP         "&amp;"
#define ESC_HEX     "&#x"
#define ESC_DEC     "&#"

typedef struct _XML_NamespaceURI 
{
	char                        *nsURI;
	char                        *prefix;
	struct _XML_NamespaceURI   *nextNsURI;
} XML_NamespaceURI;

typedef struct _XML_ElementStack
{
	char                    *element;
	char                    *prefix;
	char                    *namespaceUri;
	XML_NamespaceURI            *pNsURI;
	struct _XML_ElementStack    *nextElement;
} XML_ElementStack;

typedef enum
{
	eELEMENT,
	eATTRIBUTE,
	eCONTENT,
} PARSER_STATE;

typedef struct _Parser
{
	char            *dataBuffer;	//data buffer
	char            *curPtr;		//ptr to the token parsed 
	char            *savePtr;		//Saves for backup
	xml_membuf     lastElem;
	xml_membuf     tokenBuf;    

	XML_Node           *pNeedPrefixNode;
	XML_ElementStack   *pCurElement;
	XML_Node           *currentNodePtr;
	PARSER_STATE        state;

	BOOL                bHasTopLevel;
} Parser;

int     xml_parser_LoadDocument( XML_Document **retDoc, char * xmlFile, BOOL file);
int 	xml_parser_LoadDocument_Unclosed( XML_Document ** retDoc, char *xmlFileName, BOOL file, BOOL first_unclosed);
BOOL    xml_parser_isValidXmlName( char* name);
int     xml_parser_setNodePrefixAndLocalName(XML_Node *newXML_NodeXML_Attr);
void    xml_parser_freeNodeContent( XML_Node *XML_Nodeptr);
void    xml_parser_setErrorChar( char c );
void xml_parser_relax(char errorChar);

#endif  // _IXMLPARSER_H

