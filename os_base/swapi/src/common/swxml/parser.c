#include "swapi.h"
#include "swxml.h"
#include "element.h"
#include "node.h"
#include "membuf.h"
#include "parser.h"
#include "document.h"
#include "string_ext.h"
#include "swmem.h"
#include "swcommon_priv.h"

static char g_error_char = '\0';

static const char LESSTHAN = '<';
static const char GREATERTHAN = '>';
static const char SLASH = '/';
static const char EQUALS = '=';
static const char QUOTE = '\"';
static const char SINGLEQUOTE = '\'';

static const char *WHITESPACE = "\n\t\r ";
static const char *COMPLETETAG = "/>";
static const char *ENDTAG = "</";
static const char *XMLDECL = "<?xml ";
static const char *XMLDECL2 = "<?xml?";
static const char *BEGIN_COMMENT = "<!--";
static const char *END_COMMENT = "-->";
static const char *BEGIN_PI = "<?";
static const char *END_PI = "?>";
static const char *BEGIN_DOCTYPE = "<!DOCTYPE";
static const char *CDSTART = "<![CDATA[";
static const char *CDEND = "]]>";
static const char *DEC_NUMBERS = "0123456789";
static const char *HEX_NUMBERS = "0123456789ABCDEFabcdef";

typedef struct char_info {
	unsigned short l, h;
} char_info_t;

typedef char utf8char[8];

/*==============================================================================*
*	Letter table contains all characters in XML 1.0 plus ":", "_" and
*   ideographic.
*
*   This table contains all the characters that an element name can start with.
*   See XML 1.0 (2nd Edition) for more details.	
*
*===============================================================================*/
static char_info_t Letter[] = {
	{0x003A, 0x003A},           // character ":"
	{0x0041, 0x005A},
	{0x005F, 0x005F},           // character "_"
	{0x0061, 0x007A}, {0x00C0, 0x00D6}, {0x00D8, 0x00F6}, {0x00F8, 0x00FF},
	{0x0100, 0x0131}, {0x0134, 0x013E}, {0x0141, 0x0148}, {0x014A, 0x017E},
	{0x0180, 0x01C3}, {0x01CD, 0x01F0}, {0x01F4, 0x01F5}, {0x01FA, 0x0217},
	{0x0250, 0x02A8}, {0x02BB, 0x02C1}, {0x0386, 0x0386}, {0x0388, 0x038A},
	{0x038C, 0x038C}, {0x038E, 0x03A1}, {0x03A3, 0x03CE}, {0x03D0, 0x03D6},
	{0x03DA, 0x03DA},
	{0x03DC, 0x03DC}, {0x03DE, 0x03DE}, {0x03E0, 0x03E0}, {0x03E2, 0x03F3},
	{0x0401, 0x040C}, {0x040E, 0x044F}, {0x0451, 0x045C}, {0x045E, 0x0481},
	{0x0490, 0x04C4}, {0x04C7, 0x04C8}, {0x04CB, 0x04CC}, {0x04D0, 0x04EB},
	{0x04EE, 0x04F5}, {0x04F8, 0x04F9}, {0x0531, 0x0556}, {0x0559, 0x0559},
	{0x0561, 0x0586}, {0x05D0, 0x05EA}, {0x05F0, 0x05F2}, {0x0621, 0x063A},
	{0x0641, 0x064A}, {0x0671, 0x06B7}, {0x06BA, 0x06BE}, {0x06C0, 0x06CE},
	{0x06D0, 0x06D3}, {0x06D5, 0x06D5}, {0x06E5, 0x06E6}, {0x0905, 0x0939},
	{0x093D, 0x093D}, {0x0958, 0x0961}, {0x0985, 0x098C}, {0x098F, 0x0990},
	{0x0993, 0x09A8}, {0x09AA, 0x09B0}, {0x09B2, 0x09B2}, {0x09B6, 0x09B9},
	{0x09DC, 0x09DD}, {0x09DF, 0x09E1}, {0x09F0, 0x09F1}, {0x0A05, 0x0A0A},
	{0x0A0F, 0x0A10}, {0x0A13, 0x0A28}, {0x0A2A, 0x0A30}, {0x0A32, 0x0A33},
	{0x0A35, 0x0A36}, {0x0A38, 0x0A39}, {0x0A59, 0x0A5C}, {0x0A5E, 0x0A5E},
	{0x0A72, 0x0A74}, {0x0A85, 0x0A8B}, {0x0A8D, 0x0A8D}, {0x0A8F, 0x0A91},
	{0x0A93, 0x0AA8}, {0x0AAA, 0x0AB0}, {0x0AB2, 0x0AB3}, {0x0AB5, 0x0AB9},
	{0x0ABD, 0x0ABD}, {0x0AE0, 0x0AE0}, {0x0B05, 0x0B0C}, {0x0B0F, 0x0B10},
	{0x0B13, 0x0B28}, {0x0B2A, 0x0B30}, {0x0B32, 0x0B33}, {0x0B36, 0x0B39},
	{0x0B3D, 0x0B3D}, {0x0B5C, 0x0B5D}, {0x0B5F, 0x0B61}, {0x0B85, 0x0B8A},
	{0x0B8E, 0x0B90}, {0x0B92, 0x0B95}, {0x0B99, 0x0B9A}, {0x0B9C, 0x0B9C},
	{0x0B9E, 0x0B9F}, {0x0BA3, 0x0BA4}, {0x0BA8, 0x0BAA}, {0x0BAE, 0x0BB5},
	{0x0BB7, 0x0BB9}, {0x0C05, 0x0C0C}, {0x0C0E, 0x0C10}, {0x0C12, 0x0C28},
	{0x0C2A, 0x0C33}, {0x0C35, 0x0C39}, {0x0C60, 0x0C61}, {0x0C85, 0x0C8C},
	{0x0C8E, 0x0C90}, {0x0C92, 0x0CA8}, {0x0CAA, 0x0CB3}, {0x0CB5, 0x0CB9},
	{0x0CDE, 0x0CDE}, {0x0CE0, 0x0CE1}, {0x0D05, 0x0D0C}, {0x0D0E, 0x0D10},
	{0x0D12, 0x0D28}, {0x0D2A, 0x0D39}, {0x0D60, 0x0D61}, {0x0E01, 0x0E2E},
	{0x0E30, 0x0E30}, {0x0E32, 0x0E33}, {0x0E40, 0x0E45}, {0x0E81, 0x0E82},
	{0x0E84, 0x0E84}, {0x0E87, 0x0E88}, {0x0E8A, 0x0E8A}, {0x0E8D, 0x0E8D},
	{0x0E94, 0x0E97}, {0x0E99, 0x0E9F}, {0x0EA1, 0x0EA3}, {0x0EA5, 0x0EA5},
	{0x0EA7, 0x0EA7}, {0x0EAA, 0x0EAB}, {0x0EAD, 0x0EAE}, {0x0EB0, 0x0EB0},
	{0x0EB2, 0x0EB3}, {0x0EBD, 0x0EBD}, {0x0EC0, 0x0EC4}, {0x0F40, 0x0F47},
	{0x0F49, 0x0F69}, {0x10A0, 0x10C5}, {0x10D0, 0x10F6}, {0x1100, 0x1100},
	{0x1102, 0x1103}, {0x1105, 0x1107}, {0x1109, 0x1109}, {0x110B, 0x110C},
	{0x110E, 0x1112}, {0x113C, 0x113C}, {0x113E, 0x113E}, {0x1140, 0x1140},
	{0x114C, 0x114C}, {0x114E, 0x114E}, {0x1150, 0x1150}, {0x1154, 0x1155},
	{0x1159, 0x1159}, {0x115F, 0x1161}, {0x1163, 0x1163}, {0x1165, 0x1165},
	{0x1167, 0x1167}, {0x1169, 0x1169}, {0x116D, 0x116E}, {0x1172, 0x1173},
	{0x1175, 0x1175}, {0x119E, 0x119E}, {0x11A8, 0x11A8}, {0x11AB, 0x11AB},
	{0x11AE, 0x11AF}, {0x11B7, 0x11B8}, {0x11BA, 0x11BA}, {0x11BC, 0x11C2},
	{0x11EB, 0x11EB}, {0x11F0, 0x11F0}, {0x11F9, 0x11F9}, {0x1E00, 0x1E9B},
	{0x1EA0, 0x1EF9}, {0x1F00, 0x1F15}, {0x1F18, 0x1F1D}, {0x1F20, 0x1F45},
	{0x1F48, 0x1F4D}, {0x1F50, 0x1F57}, {0x1F59, 0x1F59}, {0x1F5B, 0x1F5B},
	{0x1F5D, 0x1F5D}, {0x1F5F, 0x1F7D}, {0x1F80, 0x1FB4}, {0x1FB6, 0x1FBC},
	{0x1FBE, 0x1FBE}, {0x1FC2, 0x1FC4}, {0x1FC6, 0x1FCC}, {0x1FD0, 0x1FD3},
	{0x1FD6, 0x1FDB}, {0x1FE0, 0x1FEC}, {0x1FF2, 0x1FF4}, {0x1FF6, 0x1FFC},
	{0x2126, 0x2126}, {0x212A, 0x212B}, {0x212E, 0x212E}, {0x2180, 0x2182},
	{0x3007, 0x3007}, {0x3021, 0x3029}, // these two are ideographic
	{0x3041, 0x3094}, {0x30A1, 0x30FA}, {0x3105, 0x312C},
	{0x4E00, 0x9FA5},           // ideographic
	{0xAC00, 0xD7A3}
};

#define LETTERTABLESIZE (sizeof(Letter)/sizeof(Letter[0]))

/*==============================================================================*
*   NameChar table contains
*   CombiningChar, Extender, Digit, '-', '.', less '_', ':'
*   NameChar ::= Digit | '-' | '.' | CombiningChar | Extender
*   See XML 1.0 2nd Edition 
*
*===============================================================================*/
static char_info_t NameChar[] = {
	{0x002D, 0x002D},           // character "-"
	{0x002E, 0x002E},           // character "."
	{0x0030, 0x0039},           // digit
	{0x00B7, 0x00B7}, {0x02D0, 0x02D0}, {0x02D1, 0x02D1},   // extended
	{0x0300, 0x0345}, {0x0360, 0x0361},
	{0x0387, 0x0387},           // extended
	{0x0483, 0x0486}, {0x0591, 0x05A1}, {0x05A3, 0x05B9},
	{0x05BB, 0x05BD}, {0x05BF, 0x05BF}, {0x05C1, 0x05C2}, {0x05C4, 0x05C4},
	{0x0640, 0x0640},           // extended
	{0x064B, 0x0652},
	{0x0660, 0x0669},           // digit
	{0x0670, 0x0670},
	{0x06D6, 0x06DC}, {0x06DD, 0x06DF}, {0x06E0, 0x06E4}, {0x06E7, 0x06E8},
	{0x06EA, 0x06ED},
	{0x06F0, 0x06F9},           // digit
	{0x0901, 0x0903}, {0x093C, 0x093C},
	{0x093E, 0x094C}, {0x094D, 0x094D}, {0x0951, 0x0954}, {0x0962, 0x0963},
	{0x0966, 0x096F},           // digit
	{0x0981, 0x0983}, {0x09BC, 0x09BC}, {0x09BE, 0x09BE},
	{0x09BF, 0x09BF}, {0x09C0, 0x09C4}, {0x09C7, 0x09C8}, {0x09CB, 0x09CD},
	{0x09D7, 0x09D7}, {0x09E2, 0x09E3},
	{0x09E6, 0x09EF},           // digit
	{0x0A02, 0x0A02},
	{0x0A3C, 0x0A3C}, {0x0A3E, 0x0A3E}, {0x0A3F, 0x0A3F}, {0x0A40, 0x0A42},
	{0x0A47, 0x0A48}, {0x0A4B, 0x0A4D},
	{0x0A66, 0x0A6F},           // digit
	{0x0A70, 0x0A71},
	{0x0A81, 0x0A83}, {0x0ABC, 0x0ABC}, {0x0ABE, 0x0AC5}, {0x0AC7, 0x0AC9},
	{0x0ACB, 0x0ACD},
	{0x0AE6, 0x0AEF},           // digit
	{0x0B01, 0x0B03}, {0x0B3C, 0x0B3C},
	{0x0B3E, 0x0B43}, {0x0B47, 0x0B48}, {0x0B4B, 0x0B4D}, {0x0B56, 0x0B57},
	{0x0B66, 0x0B6F},           // digit
	{0x0B82, 0x0B83}, {0x0BBE, 0x0BC2}, {0x0BC6, 0x0BC8},
	{0x0BCA, 0x0BCD}, {0x0BD7, 0x0BD7},
	{0x0BE7, 0x0BEF},           // digit
	{0x0C01, 0x0C03},
	{0x0C3E, 0x0C44}, {0x0C46, 0x0C48}, {0x0C4A, 0x0C4D}, {0x0C55, 0x0C56},
	{0x0C66, 0x0C6F},           // digit
	{0x0C82, 0x0C83}, {0x0CBE, 0x0CC4}, {0x0CC6, 0x0CC8},
	{0x0CCA, 0x0CCD}, {0x0CD5, 0x0CD6},
	{0x0CE6, 0x0CEF},           // digit
	{0x0D02, 0x0D03},
	{0x0D3E, 0x0D43}, {0x0D46, 0x0D48}, {0x0D4A, 0x0D4D}, {0x0D57, 0x0D57},
	{0x0D66, 0x0D6F},           // digit
	{0x0E31, 0x0E31}, {0x0E34, 0x0E3A},
	{0x0E46, 0x0E46},           // extended
	{0x0E47, 0x0E4E},
	{0x0E50, 0x0E59},           // digit
	{0x0EB1, 0x0EB1}, {0x0EB4, 0x0EB9},
	{0x0EBB, 0x0EBC},
	{0x0EC6, 0x0EC6},           // extended
	{0x0EC8, 0x0ECD},
	{0x0ED0, 0x0ED9},           // digit
	{0x0F18, 0x0F19},
	{0x0F20, 0x0F29},           // digit
	{0x0F35, 0x0F35}, {0x0F37, 0x0F37},
	{0x0F39, 0x0F39}, {0x0F3E, 0x0F3E}, {0x0F3F, 0x0F3F}, {0x0F71, 0x0F84},
	{0x0F86, 0x0F8B}, {0x0F90, 0x0F95}, {0x0F97, 0x0F97}, {0x0F99, 0x0FAD},
	{0x0FB1, 0x0FB7}, {0x0FB9, 0x0FB9}, {0x20D0, 0x20DC}, {0x20E1, 0x20E1},
	{0x3005, 0x3005},           // extended
	{0x302A, 0x302F},
	{0x3031, 0x3035},           // extended 
	{0x3099, 0x3099}, {0x309A, 0x309A}, // combining char
	{0x309D, 0x309E}, {0x30FC, 0x30FE}  // extended
};

#define NAMECHARTABLESIZE   (sizeof(NameChar)/sizeof(NameChar[0]))

// functions used in this file
static void xml_parser_free( Parser * myParser );
static int xml_parser_skipDocType( char **pstr );
static int xml_parser_skipProlog( Parser * xmlParser );
static int xml_parser_skipMisc( Parser * xmlParser );
static void xml_parser_freeElementStackItem( XML_ElementStack * pItem );
static void xml_parser_freeNsURI( XML_NamespaceURI * pNsURI );

static int xml_parser_getNextNode( Parser * myParser, XML_Node * newNode, BOOL * isEnd );
static int xml_parser_getNextToken( Parser * myParser );
static int xml_parser_xmlNamespace( Parser * myParser, XML_Node * newNode );
static BOOL xml_parser_ElementPrefixDefined( Parser * myParser, XML_Node * newNode, char **nsURI );
static int xml_parser_setElementNamespace( XML_Element * newElement, char *nsURI );
static int xml_parser_parseDocument( XML_Document ** retDoc, Parser * domParser );
static BOOL xml_parser_hasDefaultNamespace( Parser * xmlParser, XML_Node * newNode, char **nsURI );
static int xml_parser_getChar( char *src, int *cLen );

/*==============================================================================*
*   xml_parser_isCharInTable
*       will determine whether character c is in the table of tbl
*       (either Letter table or NameChar table)
*   
*===============================================================================*/
static BOOL
xml_parser_isCharInTable( int c,
                      char_info_t * tbl,
                      int sz )
{
	int t = 0, b = sz, m;

	while( t <= b ) {
		m = ( t + b ) >> 1;
		if( c < tbl[m].l ) {
			b = m - 1;
		} else if( c > tbl[m].h ) {
			t = m + 1;
		} else {
			return TRUE;
		}
	}
	return FALSE;
}

/*==============================================================================*
*	xml_parser_isXmlChar
*	    see XML 1.0 (2nd Edition) 2.2.
*       Internal to parser only
*
*===============================================================================*/
static BOOL
xml_parser_isXmlChar( int c )
{
	return ( c == 0x9 || c == 0xA || c == 0xD ||
			( c >= 0x20 && c <= 0xD7FF ) ||
			( c >= 0xE000 && c <= 0xFFFD ) ||
			( c >= 0x10000 && c <= 0x10FFFF ) );
}

/*==============================================================================*
*   xml_parser_isNameChar
*       check whether c (int) is in LetterTable or NameCharTable
*       Internal to parser only.
*
*===============================================================================*/
static BOOL
xml_parser_isNameChar( int c, BOOL bNameChar )
{
	if( xml_parser_isCharInTable( c, Letter, LETTERTABLESIZE ) ) {
		return TRUE;
	}

	if( bNameChar
			&& xml_parser_isCharInTable( c, NameChar, NAMECHARTABLESIZE ) ) {
		return TRUE;
	}
	return FALSE;
}

/*==============================================================================*
*   xml_parser_isValidXmlName
*       Check to see whether name is a valid xml name.
*       External function.
*
*===============================================================================*/
BOOL
xml_parser_isValidXmlName( char* name )
{
	char *pstr = NULL;
	int i = 0,
			nameLen = 0;

	assert( name != NULL );

	nameLen = strlen( name );

	pstr = name;
	if( xml_parser_isNameChar( *pstr, FALSE ) == TRUE ) {
		for( i = 1; i < nameLen; i++ ) {
			if( xml_parser_isNameChar( *( pstr + 1 ), TRUE ) == FALSE ) {   //illegal char
				return FALSE;
			}
		}
	}

	return TRUE;
}

/*==============================================================================*
*   xml_parser_setErrorChar:	
*       If 'c' is 0 (default), the parser is strict about XML encoding :
*       invalid UTF-8 sequences or "&" entities are rejected, and the parsing 
*       aborts.
*       If 'c' is not 0, the parser is relaxed : invalid UTF-8 characters
*       are replaced by this character, and invalid "&" entities are left
*       untranslated. The parsing is then allowed to continue.
*       External function.
*   
*===============================================================================*/
void
xml_parser_setErrorChar( char c )
{
	g_error_char = c;
}


/*==============================================================================*
*   xml_parser_intToUTF8:	
*       Encoding a character to its UTF-8 character string, and return its length
*       internal function.
*   
*===============================================================================*/
static int
xml_parser_intToUTF8( int c,
                  utf8char s )
{
	if( c < 0 ) {
		return 0;
	}

	if( c <= 127 ) {
		s[0] = c;
		s[1] = 0;
		return 1;
	} else if( c <= 0x07FF ) {  // 0x0080 < c <= 0x07FF
		s[0] = 0xC0 | ( c >> 6 );
		s[1] = 0x80 | ( c & 0x3f );
		s[2] = 0;
		return 2;
	} else if( c <= 0xFFFF ) {  // 0x0800 < c <= 0xFFFF
		s[0] = 0xE0 | ( c >> 12 );
		s[1] = 0x80 | ( ( c >> 6 ) & 0x3f );
		s[2] = 0x80 | ( c & 0x3f );
		s[3] = 0;
		return 3;
	} else if( c <= 0x1FFFFF ) {    // 0x10000 < c <= 0x1FFFFF
		s[0] = 0xF0 | ( c >> 18 );
		s[1] = 0x80 | ( ( c >> 12 ) & 0x3f );
		s[2] = 0x80 | ( ( c >> 6 ) & 0x3f );
		s[3] = 0x80 | ( c & 0x3f );
		s[4] = 0;
		return 4;
	} else if( c <= 0x3FFFFFF ) {   // 0x200000 < c <= 3FFFFFF
		s[0] = 0xF8 | ( c >> 24 );
		s[1] = 0x80 | ( ( c >> 18 ) & 0x3f );
		s[2] = 0x80 | ( ( c >> 12 ) & 0x3f );
		s[3] = 0x80 | ( ( c >> 6 ) & 0x3f );
		s[4] = 0x80 | ( c & 0x3f );
		s[5] = 0;
		return 5;
	} else if( c <= 0x7FFFFFFF ) {  // 0x4000000 < c <= 7FFFFFFF
		s[0] = 0xFC | ( c >> 30 );
		s[1] = 0x80 | ( ( c >> 24 ) & 0x3f );
		s[2] = 0x80 | ( ( c >> 18 ) & 0x3f );
		s[3] = 0x80 | ( ( c >> 12 ) & 0x3f );
		s[4] = 0x80 | ( ( c >> 6 ) & 0x3f );
		s[5] = 0x80 | ( c & 0x3f );
		s[6] = 0;
		return 6;
	} else {                    // illegal
		return 0;
	}
}

/*==============================================================================*
*   xml_parser_UTF8ToInt
*       In UTF-8, characters are encoded using sequences of 1 to 6 octets.
*       This functions will return a UTF-8 character value and its octets number.
*       Internal to parser only.
*       Internal to parser only
*          
*===============================================================================*/
static int
xml_parser_UTF8ToInt( char *ss,
                  int *len )
{

	unsigned char *s = ( unsigned char * )ss;
	int c = *s;

	if( c <= 127 ) {            // if c<=127, c is just the character.
		*len = 1;
		return c;
	} else if( ( c & 0xE0 ) == 0xC0 && ( s[1] & 0xc0 ) == 0x80 ) {  // a sequence of 110xxxxx and 10xxxxxx?
		*len = 2;
		return ( ( ( c & 0x1f ) << 6 ) | ( s[1] & 0x3f ) );
	} else if( ( c & 0xF0 ) == 0xE0 && ( s[1] & 0xc0 ) == 0x80 && ( s[2] & 0xc0 ) == 0x80 ) {   // a sequence of 1110xxxx,10xxxxxx and 10xxxxxx ?
		*len = 3;
		return ( ( ( c & 0xf ) << 12 ) | ( ( s[1] & 0x3f ) << 6 ) |
				( s[2] & 0x3f ) );
	} else if( ( c & 0xf8 ) == 0xf0 && ( s[1] & 0xc0 ) == 0x80 && ( s[2] & 0xc0 ) == 0x80 && ( s[3] & 0xc0 ) == 0x80 ) {    // a sequence of 11110xxx,10xxxxxx,10xxxxxx and 10xxxxxx ?
		*len = 4;
		return ( ( ( c & 0x7 ) << 18 ) | ( ( s[1] & 0x3f ) << 12 ) |
				( ( s[2] & 0x3f ) << 6 ) | ( s[3] & 0x3f ) );
	} else if( ( c & 0xfc ) == 0xf8 && ( s[1] & 0xc0 ) == 0x80 && ( s[2] & 0xc0 ) == 0x80 && ( s[3] & 0xc0 ) == 0x80 && ( s[4] & 0xc0 ) == 0x80 ) { // a sequence of 111110xx,10xxxxxx,10xxxxxx,10xxxxxx,10xxxxxx ?
		*len = 5;
		return ( ( ( c & 0x3 ) << 24 ) | ( ( s[1] & 0x3f ) << 18 ) |
				( ( s[2] & 0x3f ) << 12 ) | ( ( s[3] & 0x3f ) << 6 ) |
				( s[4] & 0x3f ) );
	} else if( ( c & 0xfe ) == 0xfc && ( s[1] & 0xc0 ) == 0x80 && ( s[2] & 0xc0 ) == 0x80 && ( s[3] & 0xc0 ) == 0x80 && ( s[4] & 0xc0 ) == 0x80 && ( s[5] & 0xc0 ) == 0x80 ) {  // a sequence of 1111110x,10xxxxxx,10xxxxxx,10xxxxxx,10xxxxxx and 10xxxxxx ?
		*len = 6;
		return ( ( ( c & 0x1 ) << 30 ) | ( ( s[1] & 0x3f ) << 24 ) |
				( ( s[2] & 0x3f ) << 18 ) | ( ( s[3] & 0x3f ) << 12 ) |
				( ( s[4] & 0x3f ) << 6 ) | ( s[5] & 0x3f ) );
	} else {                    // none of above, error
		if (g_error_char) {
			*len = 1;
			return g_error_char;
		} else {
			*len = 0;
			return -1;
		}
	}
}

/*==============================================================================*
*   xml_parser_init
*       Initializes a xml parser.
*       Internal to parser only
*			
*===============================================================================*/
static Parser *
xml_parser_init(  )
{
	Parser *newParser = NULL;

	newParser = ( Parser * ) malloc( sizeof( Parser ) );
	if( newParser == NULL ) {
		return NULL;
	}

	memset( newParser, 0, sizeof( Parser ) );

	xml_membuf_init( &( newParser->tokenBuf ) );
	xml_membuf_init( &( newParser->lastElem ) );

	return newParser;
}

/*================================================================
*   xml_parser_isValidEndElement
*       check if a new node->nodeName matches top of element stack.
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_isValidEndElement( Parser * xmlParser,
                          XML_Node * newNode )
{
	return ( strcmp( xmlParser->pCurElement->element, newNode->nodeName ) == 0 );
}

/*===============================================================
*   xml_parser_pushElement
*       push a new element onto element stack
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_pushElement( Parser * xmlParser,
                    XML_Node * newElement )
{

	XML_ElementStack *pCurElement = NULL;
	XML_ElementStack *pNewStackElement = NULL;

	assert( newElement );
	if( newElement != NULL ) {
		// push new element
		pNewStackElement =
			( XML_ElementStack * ) malloc( sizeof( XML_ElementStack ) );
		if( pNewStackElement == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}

		memset( pNewStackElement, 0, sizeof( XML_ElementStack ) );
		// the element member includes both prefix and name 

		pNewStackElement->element = strdup( newElement->nodeName );
		if( pNewStackElement->element == NULL ) {
			free( pNewStackElement );
			return XML_INSUFFICIENT_MEMORY;
		}

		if( newElement->prefix != 0 ) {
			pNewStackElement->prefix = strdup( newElement->prefix );
			if( pNewStackElement->prefix == NULL ) {
				xml_parser_freeElementStackItem( pNewStackElement );
				free( pNewStackElement );
				return XML_INSUFFICIENT_MEMORY;
			}
		}

		if( newElement->namespaceURI != 0 ) {
			pNewStackElement->namespaceUri =
				strdup( newElement->namespaceURI );
			if( pNewStackElement->namespaceUri == NULL ) {
				xml_parser_freeElementStackItem( pNewStackElement );
				free( pNewStackElement );
				return XML_INSUFFICIENT_MEMORY;
			}
		}

		pCurElement = xmlParser->pCurElement;

		// insert the new element into the top of the stack
		pNewStackElement->nextElement = pCurElement;
		xmlParser->pCurElement = pNewStackElement;

	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_parser_popElement
*       Remove element from element stack.
*       Internal to parser only.       
*
*=================================================================*/
static void
xml_parser_popElement( Parser * xmlParser )
{
	XML_ElementStack *pCur = NULL;
	XML_NamespaceURI *pnsUri = NULL,
									 *pNextNS = NULL;

	pCur = xmlParser->pCurElement;
	if( pCur != NULL ) {
		xmlParser->pCurElement = pCur->nextElement;

		xml_parser_freeElementStackItem( pCur );

		pnsUri = pCur->pNsURI;
		while( pnsUri != NULL ) {
			pNextNS = pnsUri->nextNsURI;

			xml_parser_freeNsURI( pnsUri );
			free( pnsUri );
			pnsUri = pNextNS;
		}

		free( pCur );
	}

}

/*================================================================
*   xml_parser_readFileOrBuffer
*       read a xml file or buffer contents into xml parser.
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_readFileOrBuffer( Parser * xmlParser,
                         char *xmlFileName,
                         BOOL file )
{
	int fileSize = 0;
	int bytesRead = 0;
	FILE *xmlFilePtr = NULL;

	if( file ) {
		xmlFilePtr = fopen( xmlFileName, "rb" );
		if( xmlFilePtr == NULL ) {
			return XML_NO_SUCH_FILE;
		} else {
			fseek( xmlFilePtr, 0, SEEK_END );
			fileSize = ftell( xmlFilePtr );
			if( fileSize == 0 ) {
				fclose( xmlFilePtr );
				return XML_SYNTAX_ERR;
			}

			xmlParser->dataBuffer = ( char * )malloc( fileSize + 1 );
			if( xmlParser->dataBuffer == NULL ) {
				fclose( xmlFilePtr );
				return XML_INSUFFICIENT_MEMORY;
			}

			fseek( xmlFilePtr, 0, SEEK_SET );
			bytesRead =
				fread( xmlParser->dataBuffer, 1, fileSize, xmlFilePtr );
			xmlParser->dataBuffer[bytesRead] = '\0';    // append null
			fclose( xmlFilePtr );
		}
	} else {
		xmlParser->dataBuffer = strdup( xmlFileName );
		if( xmlParser->dataBuffer == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}

	return XML_SUCCESS;
}

/*================================================================
*   xml_parser_LoadDocument
*       parses a xml file and return the DOM tree.
*       Internal to parser only
*
*=================================================================*/
int
xml_parser_LoadDocument( XML_Document ** retDoc,
                     char *xmlFileName,
                     BOOL file )
{
	int rc = XML_SUCCESS;
	Parser *xmlParser = NULL;

	xmlParser = xml_parser_init(  );
	if( xmlParser == NULL ) {
		return XML_INSUFFICIENT_MEMORY;
	}

	rc = xml_parser_readFileOrBuffer( xmlParser, xmlFileName, file );
	if( rc != XML_SUCCESS ) {
		xml_parser_free( xmlParser );
		return rc;
	}

	xmlParser->curPtr = xmlParser->dataBuffer;
	rc = xml_parser_parseDocument( retDoc, xmlParser );
	return rc;

}

/*================================================================
*   xml_parser_isTopLevelElement
*       decides whether we have top level element already.
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_isTopLevelElement( Parser * xmlParser )
{
	assert( xmlParser );
	return ( xmlParser->pCurElement == NULL );
}

/*================================================================
*   xml_parser_isDuplicateAttribute
*       Decide whether the new attribute is the same as an
*       existing one.
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_isDuplicateAttribute( Parser * xmlParser,
                      XML_Node * newAttrNode )
{
	XML_Node *elementNode = NULL;
	XML_Node *attrNode = NULL;

	elementNode = xmlParser->currentNodePtr;
	attrNode = elementNode->firstAttr;
	while( attrNode != NULL ) {
		if( strcmp( attrNode->nodeName, newAttrNode->nodeName ) == 0 ) {
			return TRUE;
		}

		attrNode = attrNode->nextSibling;
	}

	return FALSE;
}

/*================================================================
*   xml_parser_processAttributeName
*       processes the attribute name.
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_processAttributeName( XML_Document * rootDoc,
                             Parser * xmlParser,
                             XML_Node * newNode )
{
	XML_Attr *attr = NULL;
	int rc = XML_SUCCESS;

	if( xml_parser_isDuplicateAttribute( xmlParser, newNode ) == TRUE ) {
		return XML_SYNTAX_ERR;
	}

	rc = xml_document_createAttributeEx( newNode->nodeName, &attr );
	if( rc != XML_SUCCESS ) {
		return rc;
	}

	rc = xml_node_setNodeProperties( ( XML_Node * ) attr, newNode );
	if( rc != XML_SUCCESS ) {
		return rc;
	}

	rc = xml_element_setAttributeNode( ( XML_Element * ) xmlParser->
			currentNodePtr, attr, NULL );
	return rc;
}

/*================================================================
*   xml_parser_processElementName
*       Processes element name 
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_processElementName( XML_Document * rootDoc,
                           Parser * xmlParser,
                           XML_Node * newNode )
{
	XML_Element *newElement = NULL;
	char *nsURI = NULL;
	int rc = XML_SUCCESS;

	if( xmlParser->bHasTopLevel == TRUE ) {
		if( xml_parser_isTopLevelElement( xmlParser ) == TRUE ) {
			return XML_SYNTAX_ERR;
		}
	} else {
		xmlParser->bHasTopLevel = TRUE;
	}

	xmlParser->savePtr = xmlParser->curPtr;
	rc = xml_document_createElementEx( newNode->nodeName,
			&newElement );
	if( rc != XML_SUCCESS ) {
		return rc;
	}

	rc = xml_node_setNodeProperties( ( XML_Node * ) newElement, newNode );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
		return rc;
	}

	if( newNode->prefix != NULL ) { // element has namespace prefix 
		if( xml_parser_ElementPrefixDefined( xmlParser, newNode, &nsURI ) !=
				TRUE || nsURI == NULL ) {
			// read next node to see whether it includes namespace definition
			xmlParser->pNeedPrefixNode = ( XML_Node * ) newElement;
		} else {                // fill in the namespace
			xml_parser_setElementNamespace( newElement, nsURI );
		}
	} else                      // does element has default namespace
	{
		// the node may have default namespace definition
		if( xml_parser_hasDefaultNamespace( xmlParser, newNode, &nsURI ) ==
				TRUE ) {
			xml_parser_setElementNamespace( newElement, nsURI );
		} else if( xmlParser->state == eATTRIBUTE ) {
			// the default namespace maybe defined later
			xmlParser->pNeedPrefixNode = ( XML_Node * ) newElement;
		}
	}

	rc = xml_node_appendChild( xmlParser->currentNodePtr,
			( XML_Node * ) newElement );
	if( rc != XML_SUCCESS ) {
		xml_node_free( newElement );
		return rc;
	}

	xmlParser->currentNodePtr = ( XML_Node * ) newElement;

	// push element to stack
	rc = xml_parser_pushElement( xmlParser, ( XML_Node * ) newElement );
	return rc;
}

/*================================================================
*   xml_parser_eTagVerification
*       Verifies endof element tag is the same as the openning 
*       element tag.
*       Internal to parser only.
*
*=================================================================*/
static int
xml_parser_eTagVerification( Parser * xmlParser,
                         XML_Node * newNode )
{

	assert( newNode->nodeName );
	assert( xmlParser->currentNodePtr );

	if( newNode->nodeType == eELEMENT_NODE ) {
		if( xml_parser_isValidEndElement( xmlParser, newNode ) == TRUE ) {
			xml_parser_popElement( xmlParser );
		} else {                // syntax error
			return XML_SYNTAX_ERR;
		}
	}

	if( strcmp( newNode->nodeName, xmlParser->currentNodePtr->nodeName ) ==
			0 ) {
		xmlParser->currentNodePtr = xmlParser->currentNodePtr->parentNode;
	} else {
		return XML_SYNTAX_ERR;
	}

	return XML_SUCCESS;

}

/*================================================================
*   xml_parser_freeNodeContent
*       frees a node contents
*       Internal to parser only.
*
*=================================================================*/
void
xml_parser_freeNodeContent( XML_Node * nodeptr )
{

	if( nodeptr == NULL ) {
		return;
	}

	if( nodeptr->nodeName != NULL ) {
		free( nodeptr->nodeName );
	}

	if( nodeptr->nodeValue != NULL ) {
		free( nodeptr->nodeValue );
	}

	if( nodeptr->namespaceURI != NULL ) {
		free( nodeptr->namespaceURI );
	}

	if( nodeptr->prefix != NULL ) {
		free( nodeptr->prefix );
	}

	if( nodeptr->localName != NULL ) {
		free( nodeptr->localName );
	}

}

/*================================================================
*   xml_parser_parseDocument
*       Parses the xml file and returns the DOM document tree.
*       External function.
*
*=================================================================*/
static int
xml_parser_parseDocument( XML_Document ** retDoc,
                      Parser * xmlParser )
{

	XML_Document *gRootDoc = NULL;
	XML_Node newNode;
	BOOL bETag = FALSE;
	XML_Node *tempNode = NULL;
	int rc = XML_SUCCESS;
	XML_CDATASection *cdataSecNode = NULL;
	
	memset( &newNode, 0, sizeof(XML_Node) );
	rc = xml_document_createDocumentEx( &gRootDoc );
	if( rc != XML_SUCCESS ) {
		goto ErrorHandler;
	}

	xmlParser->currentNodePtr = ( XML_Node * ) gRootDoc;

	rc = xml_parser_skipProlog( xmlParser );
	if( rc != XML_SUCCESS ) {
		goto ErrorHandler;
	}

	while( bETag == FALSE ) {
		// clear the newNode contents
		memset( &newNode, 0, sizeof(XML_Node) );

		if( xml_parser_getNextNode( xmlParser, &newNode, &bETag ) ==
				XML_SUCCESS ) {
			if( bETag == FALSE ) {
				switch ( newNode.nodeType ) {
					case eELEMENT_NODE:
						rc = xml_parser_processElementName( gRootDoc,
								xmlParser,
								&newNode );
						if( rc != XML_SUCCESS ) {
							goto ErrorHandler;
						}
						break;

					case eTEXT_NODE:
						rc = xml_document_createTextNodeEx( newNode.
								nodeValue,
								&tempNode );
						if( rc != XML_SUCCESS ) {
							goto ErrorHandler;
						}

						rc = xml_node_appendChild( xmlParser->currentNodePtr, tempNode );
						if( rc != XML_SUCCESS ) {
							goto ErrorHandler;
						}

						break;

					case eCDATA_SECTION_NODE:
						rc = xml_document_createCDATASectionEx( newNode.
								nodeValue,
								&cdataSecNode );
						if( rc != XML_SUCCESS ) {
							goto ErrorHandler;
						}

						rc = xml_node_appendChild( xmlParser->currentNodePtr, (XML_Node*)cdataSecNode );
						if( rc != XML_SUCCESS ) {
							goto ErrorHandler;
						}
						break;

					case eATTRIBUTE_NODE:
						rc = xml_parser_processAttributeName( gRootDoc, xmlParser, &newNode );
						if( rc != XML_SUCCESS ) {
							goto ErrorHandler;
						}
						break;

					default:
						break;
				}
			} else              // ETag==TRUE, endof element tag.  
			{
				rc = xml_parser_eTagVerification( xmlParser, &newNode );
				if( rc != XML_SUCCESS ) {
					goto ErrorHandler;
				}
				xmlParser->state = eCONTENT;
			}

			// reset bETag flag
			bETag = FALSE;

		} else if( bETag == TRUE ) {    // file is done
			break;
		} else {
			rc = XML_FAILED;
			goto ErrorHandler;
		}
		xml_parser_freeNodeContent( &newNode );

	}

	if( xmlParser->pCurElement != NULL ) {
		rc = XML_SYNTAX_ERR;
		goto ErrorHandler;
	}

	*retDoc = ( XML_Document * ) gRootDoc;
	xml_parser_free( xmlParser );
	return rc;

ErrorHandler:
	xml_parser_freeNodeContent( &newNode );
	xml_node_free( gRootDoc );
	xml_parser_free( xmlParser );
	return rc;

}

/*==============================================================================*
*   xml_parser_setLastElem
*       set the last element to be the given string.
*       Internal to parser only.			
*
*===============================================================================*/
static int
xml_parser_setLastElem( Parser * xmlParser,
                    const char *s )
{
	int rc;

	if( ( xmlParser == NULL ) || ( s == NULL ) ) {
		return XML_FAILED;
	}

	rc = xml_membuf_assign_str( &( xmlParser->lastElem ), s );
	return rc;
}

/*==============================================================================*
*
*   xml_parser_clearTokenBuf
*       clear token buffer.
*       Internal to parser only.
*
*===============================================================================*/
static void
xml_parser_clearTokenBuf( Parser * xmlParser )
{
	xml_membuf_destroy( &( xmlParser->tokenBuf ) );
}

/*==============================================================================*
*
*   xml_parser_appendTokBufStr
*       Appends string s to token buffer
*       Internal to parser only.
*       
*===============================================================================*/
static int
xml_parser_appendTokBufStr( Parser * xmlParser,
                        const char *s )
{
	int rc = XML_SUCCESS;

	if( s != NULL ) {
		rc = xml_membuf_append_str( &( xmlParser->tokenBuf ), s );
	}

	return rc;
}

/*==============================================================================*
*
*   xml_parser_appendTokBufChar   
*       Appends c to token buffer.
*       Internal to parser only.
*
*===============================================================================*/
static int
xml_parser_appendTokBufChar( Parser * xmlParser,
                         char c )
{
	int rc;

	rc = xml_membuf_append( &( xmlParser->tokenBuf ), &c );
	return rc;
}

/*==============================================================================*
*
*   xml_parser_skipWhiteSpaces
*       skip white spaces 
*       Internal to parser only			
*
*===============================================================================*/
static void
xml_parser_skipWhiteSpaces( Parser * xmlParser )
{
	while( ( *( xmlParser->curPtr ) != 0 ) &&
			( strchr( WHITESPACE, *( xmlParser->curPtr ) ) != NULL ) ) {
		xmlParser->curPtr++;
	}

}

/*==============================================================================*
*   xml_parser_getChar	
*       returns next char value and its length			
*       Internal to parser only
*
*===============================================================================*/
static int
xml_parser_getChar( char *src,
                int *cLen )
{
	char *pnum;
	int sum;
	char c;
	int i;

	if( src == NULL || cLen == NULL ) {
		return -1;
	}

	*cLen = 0;

	if( *src != '&' ) {
		if( *src > 0 && xml_parser_isXmlChar( *src ) ) {
			*cLen = 1;
			return *src;
		}

		i = xml_parser_UTF8ToInt( src, cLen );
		if( !xml_parser_isXmlChar( i ) ) {
			return ( g_error_char ? g_error_char : -1 );
		}
		return i;
	} else if( strncasecmp( src, QUOT, strlen( QUOT ) ) == 0 ) {
		*cLen = strlen( QUOT );
		return '"';
	} else if( strncasecmp( src, LT, strlen( LT ) ) == 0 ) {
		*cLen = strlen( LT );
		return '<';
	} else if( strncasecmp( src, GT, strlen( GT ) ) == 0 ) {
		*cLen = strlen( GT );
		return '>';
	} else if( strncasecmp( src, APOS, strlen( APOS ) ) == 0 ) {
		*cLen = strlen( APOS );
		return '\'';
	} else if( strncasecmp( src, AMP, strlen( AMP ) ) == 0 ) {
		*cLen = strlen( AMP );
		return '&';
	} else if( strncasecmp( src, ESC_HEX, strlen( ESC_HEX ) ) == 0 ) {  // Read in escape characters of type &#xnn where nn is a hexadecimal value
		pnum = src + strlen( ESC_HEX );
		sum = 0;
		while( strchr( HEX_NUMBERS, *pnum ) != 0 ) {
			c = *pnum;
			if( c <= '9' ) {
				sum = sum * 16 + ( c - '0' );
			} else if( c <= 'F' ) {
				sum = sum * 16 + ( c - 'A' + 10 );
			} else {
				sum = sum * 16 + ( c - 'a' + 10 );
			}

			pnum++;
		}

		if( ( pnum == src ) || *pnum != ';' || !xml_parser_isXmlChar( sum ) ) {
			goto fail_entity;
		}

		*cLen = pnum - src + 1;
		return sum;

	} else if( strncasecmp( src, ESC_DEC, strlen( ESC_DEC ) ) == 0 ) {
		// Read in escape characters of type &#nn where nn is a decimal value
		pnum = src + strlen( ESC_DEC );
		sum = 0;
		while( strchr( DEC_NUMBERS, *pnum ) != 0 ) {
			sum = sum * 10 + ( *pnum - '0' );
			pnum++;
		}

		if( ( pnum == src ) || *pnum != ';' || !xml_parser_isXmlChar( sum ) ) {
			goto fail_entity;
		}

		*cLen = pnum - src + 1;
		return sum;
	}

fail_entity:
	if (g_error_char) {
		*cLen = 1;
		return '&';
	}
	return -1;
}

/*==============================================================================*
*   xml_parser_copyToken	
*       copy string in src into xml parser token buffer
*		Internal to parser only.	
*
*===============================================================================*/
static int
xml_parser_copyToken( Parser * xmlParser,
                  char *src,
                  int len )
{
	int i,
	c,
	cl;
	char *psrc,
			 *pend;
	utf8char uch;

	if( !src || len <= 0 ) {
		return XML_FAILED;
	}

	psrc = src;
	pend = src + len;

	while( psrc < pend ) {
		if( ( c = xml_parser_getChar( psrc, &cl ) ) <= 0 ) {
			return XML_FAILED;
		}

		if( cl == 1 ) {
			xml_parser_appendTokBufChar( xmlParser, ( char )c );
			psrc++;
		} else {

			i = xml_parser_intToUTF8( c, uch );
			if( i == 0 ) {
				return XML_FAILED;
			}

			xml_parser_appendTokBufStr( xmlParser, uch );
			psrc += cl;
		}
	}

	if( psrc > pend ) {
		return XML_FAILED;
	} else {
		return XML_SUCCESS;    // success
	}

}

/*==============================================================================*
*
*   xml_parser_skipString
*       Skips all characters in the string until it finds the skip key.
*       Then it skips the skip key and returns.
*       Internal to parser only
*
*===============================================================================*/
static int
xml_parser_skipString( char **pstrSrc,
                   const char *strSkipKey )
{
	if( !( *pstrSrc ) || !strSkipKey ) {
		return XML_FAILED;
	}

	while( ( **pstrSrc )
			&& strncmp( *pstrSrc, strSkipKey,
				strlen( strSkipKey ) ) != 0 ) {
		( *pstrSrc )++;
	}

	if( **pstrSrc == '\0' ) {
		return XML_SYNTAX_ERR;
	}
	*pstrSrc = *pstrSrc + strlen( strSkipKey );

	return XML_SUCCESS;        //success
}

/*==============================================================================*
*
* Function:	
* Returns:	
*			
*
*===============================================================================*/
static int
xml_parser_skipPI( char **pSrc )
{
	char *pEnd = NULL;

	assert( *pSrc );
	if( *pSrc == NULL ) {
		return XML_FAILED;
	}

	if( ( strncasecmp( *pSrc, ( char * )XMLDECL, strlen( XMLDECL ) ) == 0 ) ||
			( strncasecmp( *pSrc, ( char * )XMLDECL2, strlen( XMLDECL2 ) ) == 0 ) ) {    // not allowed
		return XML_SYNTAX_ERR;
	}

	if( strncasecmp( *pSrc, ( char * )BEGIN_PI, strlen( BEGIN_PI ) ) == 0 ) {
		pEnd = strstr( *pSrc, END_PI );
		if( ( pEnd != NULL ) && ( pEnd != *pSrc ) ) {
			*pSrc = pEnd + strlen( BEGIN_PI );
		} else {
			return XML_SYNTAX_ERR;
		}
	}

	return XML_SUCCESS;
}

/*==============================================================================*
*   xml_parser_skipXMLDecl:	
*       skips XML declarations.
*       Internal only to parser.			
*
*===============================================================================*/
static int
xml_parser_skipXMLDecl( Parser * xmlParser )
{
	int rc = XML_FAILED;

	assert( xmlParser );
	if( xmlParser == NULL ) {
		return rc;
	}

	rc = xml_parser_skipString( &( xmlParser->curPtr ), END_PI );
	xml_parser_skipWhiteSpaces( xmlParser );
	return rc;

}

/*==============================================================================*
*   xml_parser_skipProlog
*       skip prolog
*       Internal to parser only.			
*
*===============================================================================*/
static int
xml_parser_skipProlog( Parser * xmlParser )
{
	int rc = XML_SUCCESS;

	assert( xmlParser != NULL );
	if( xmlParser == NULL ) {
		return XML_FAILED;
	}

	xml_parser_skipWhiteSpaces( xmlParser );

	if( strncmp( xmlParser->curPtr, ( char * )XMLDECL, strlen( XMLDECL ) ) == 0 ) { // <?xml
		rc = xml_parser_skipXMLDecl( xmlParser );
		if( rc != XML_SUCCESS ) {
			return rc;
		}
	}

	rc = xml_parser_skipMisc( xmlParser );
	if( ( rc == XML_SUCCESS ) && strncmp( xmlParser->curPtr, ( char * )BEGIN_DOCTYPE, strlen( BEGIN_DOCTYPE ) ) == 0 ) {   // <! DOCTYPE
		xmlParser->curPtr++;
		rc = xml_parser_skipDocType( &( xmlParser->curPtr ) );
	}

	if( rc == XML_SUCCESS ) {
		rc = xml_parser_skipMisc( xmlParser );
	}

	return rc;
}

/*==============================================================================*
*
* Function:
* Returns:
*       Skips all characters in the string until it finds the skip key.
*       Then it skips the skip key and returns.
*
*===============================================================================*/
static int
xml_parser_skipComment( char **pstrSrc )
{
	char *pStrFound = NULL;

	assert( ( *pstrSrc ) != NULL );
	if( *pstrSrc == NULL ) {
		return XML_FAILED;
	}

	pStrFound = strstr( *pstrSrc, END_COMMENT );
	if( ( pStrFound != NULL ) && ( pStrFound != *pstrSrc ) &&
			( *( pStrFound - 1 ) != '-' ) ) {
		*pstrSrc = pStrFound + strlen( END_COMMENT );
	} else {
		return XML_SYNTAX_ERR;
	}

	return XML_SUCCESS;
}

/*==============================================================================*
*   xml_parser_skipDocType
*       skips document type declaration
*
*===============================================================================*/
static int
xml_parser_skipDocType( char **pstr )
{
	char *pCur = *pstr;
	char *pNext = NULL;         // default there is no nested <
	int num = 1;

	assert( ( *pstr ) != NULL );
	if( *pstr == NULL ) {
		return XML_FAILED;
	}

	while( ( pCur != NULL ) && ( num != 0 ) && ( *pCur != 0 ) ) {
		if( *pCur == '<' ) {
			num++;
		} else if( *pCur == '>' ) {
			num--;
		} else if( *pCur == '"' ) {
			pNext = strchr( pCur + 1, '"' );
			if( pNext == NULL ) {
				return XML_SYNTAX_ERR;
			}

			pCur = pNext;
		}

		pCur++;
	}

	if( num == 0 ) {
		*pstr = pCur;
		return XML_SUCCESS;
	} else {
		return XML_SYNTAX_ERR;
	}
}

/*==============================================================================*
*
*   xml_parser_skipMisc:	
*       skip comment, PI and white space 
*			
*
*===============================================================================*/
static int
xml_parser_skipMisc( Parser * xmlParser )
{
	int rc = XML_SUCCESS;
	int done = FALSE;

	while( ( done == FALSE ) && ( rc == XML_SUCCESS ) ) {
		if( strncasecmp( xmlParser->curPtr, ( char * )BEGIN_COMMENT, strlen( BEGIN_COMMENT ) ) == 0 ) { // <!--
			rc = xml_parser_skipComment( &( xmlParser->curPtr ) );

		} else if( ( strncasecmp( xmlParser->curPtr, ( char * )XMLDECL, strlen( XMLDECL ) ) == 0 ) || ( strncasecmp( xmlParser->curPtr, ( char * )XMLDECL2, strlen( XMLDECL2 ) ) == 0 ) ) { // <?xml or <?xml?
			rc = XML_SYNTAX_ERR;
		} else if( strncasecmp( xmlParser->curPtr, ( char * )BEGIN_PI, strlen( BEGIN_PI ) ) == 0 ) {    // <?
			rc = xml_parser_skipString( &( xmlParser->curPtr ), END_PI );
		} else {
			done = TRUE;
		}

		xml_parser_skipWhiteSpaces( xmlParser );
	}

	return rc;
}

/*==============================================================================*
*
*   xml_parser_getNextToken
*       return the length of next token in tokenBuff
*			
*
*===============================================================================*/
static int
xml_parser_getNextToken( Parser * xmlParser )
{
	int tokenLength = 0;
	int temp,
			tlen;
	int rc;

	xml_parser_clearTokenBuf( xmlParser );

	if( *( xmlParser->curPtr ) == '\0' ) {
		return 0;
	}
	// skip XML instructions
	rc = xml_parser_skipMisc( xmlParser );
	if( rc != XML_SUCCESS ) {
		return 0;
	}
	// Attribute value logic must come first, since all text untokenized until end-quote
	if( *( xmlParser->curPtr ) == QUOTE ) {
		tokenLength = 1;
	} else if( *( xmlParser->curPtr ) == SINGLEQUOTE ) {
		tokenLength = 1;
	} else if( *( xmlParser->curPtr ) == LESSTHAN ) {   // Check for start tags
		temp = xml_parser_UTF8ToInt( xmlParser->curPtr + 1, &tlen );
		if( temp == '/' ) {
			tokenLength = 2;    // token is '</' end tag
		} else if( xml_parser_isNameChar( temp, FALSE ) == TRUE ) {
			tokenLength = 1;    // '<' found, so return '<' token
		} else {
			return 0;           //error
		}
	} else if( *( xmlParser->curPtr ) == EQUALS ) { // Check for '=' token, return it as a token
		tokenLength = 1;
	} else if( *( xmlParser->curPtr ) == SLASH ) {
		if( *( xmlParser->curPtr + 1 ) == GREATERTHAN ) {   // token '/>' found
			tokenLength = 2;
			xmlParser->savePtr = xmlParser->curPtr; // fix
		}
	} else if( *( xmlParser->curPtr ) == GREATERTHAN ) {    // > found, so return it as a token
		tokenLength = 1;
	} else if( xml_parser_isNameChar( xml_parser_UTF8ToInt( xmlParser->curPtr, &tlen ), FALSE ) ) { // Check for name tokens, name found, so find out how long it is
		int iIndex = tlen;

		while( xml_parser_isNameChar
				( xml_parser_UTF8ToInt( xmlParser->curPtr + iIndex, &tlen ),
					TRUE ) ) {
			iIndex += tlen;
		}
		tokenLength = iIndex;
	} else {
		return 0;
	}

	// Copy the token to the return string
	if( xml_parser_copyToken( xmlParser, xmlParser->curPtr, tokenLength ) !=
			XML_SUCCESS ) {
		return 0;
	}

	xmlParser->curPtr += tokenLength;
	return tokenLength;
}

/*==============================================================================*
*
*   xml_parser_getNameSpace	
*       return the namespce as defined as prefix.
*       Internal to parser only			
*
*===============================================================================*/
static char *
xml_parser_getNameSpace( Parser * xmlParser,
                     char *prefix )
{
	XML_ElementStack *pCur;
	XML_NamespaceURI *pNsUri;

	pCur = xmlParser->pCurElement;
	if( strcmp( pCur->prefix, prefix ) != 0 ) {
		pNsUri = pCur->pNsURI;
		while( pNsUri != NULL ) {
			if( strcmp( pNsUri->prefix, prefix ) == 0 ) {
				return pNsUri->nsURI;
			}
			pNsUri = pNsUri->nextNsURI;
		}
	} else {
		return pCur->namespaceUri;
	}

	return NULL;

}

/*==============================================================================*
*
*   xml_parser_addNamespace
*       Add a namespace definition
*       Internal to parser only			
*
*===============================================================================*/
static int
xml_parser_addNamespace( Parser * xmlParser )
{
	XML_Node *pNode;
	XML_ElementStack *pCur;
	char *namespaceUri;

	pNode = xmlParser->pNeedPrefixNode;
	pCur = xmlParser->pCurElement;

	if( pNode->prefix == NULL ) {   // element does not have prefix
		if( strcmp( pNode->nodeName, pCur->element ) != 0 ) {
			return XML_FAILED;
		}
		if( pCur->namespaceUri != NULL ) {
			// it would be wrong that pNode->namespace != NULL.
			assert( pNode->namespaceURI == NULL );

			pNode->namespaceURI = strdup( pCur->namespaceUri );
			if( pNode->namespaceURI == NULL ) {
				return XML_INSUFFICIENT_MEMORY;
			}
		}

		xmlParser->pNeedPrefixNode = NULL;

	} else {
		if( ( strcmp( pNode->nodeName, pCur->element ) != 0 ) &&
				( strcmp( pNode->prefix, pCur->prefix ) != 0 ) ) {
			return XML_FAILED;
		}

		namespaceUri = xml_parser_getNameSpace( xmlParser, pCur->prefix );
		if( namespaceUri != NULL ) {
			pNode->namespaceURI = strdup( namespaceUri );
			if( pNode->namespaceURI == NULL ) {
				return XML_INSUFFICIENT_MEMORY;
			}

			xmlParser->pNeedPrefixNode = NULL;
		}
	}
	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_setNodePrefixAndLocalName
*       set the node prefix and localName as defined by the nodeName
*       in the form of ns:name
*       Internal to parser only.			
*
*===============================================================================*/
int
xml_parser_setNodePrefixAndLocalName( XML_Node * node )
{

	char *pStrPrefix = NULL;
	char *pLocalName;
	int nPrefix;

	assert( node != NULL );
	if( node == NULL ) {
		return XML_FAILED;
	}

	pStrPrefix = strchr( node->nodeName, ':' );
	if( pStrPrefix == NULL ) {
		node->prefix = NULL;
		node->localName = strdup( node->nodeName );
		if( node->localName == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}

	} else {                    // fill in the local name and prefix

		pLocalName = ( char * )pStrPrefix + 1;
		nPrefix = pStrPrefix - node->nodeName;
		node->prefix = malloc( nPrefix + 1 );
		if( node->prefix == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}

		memset( node->prefix, 0, nPrefix + 1 );
		strlcpy( node->prefix, node->nodeName, nPrefix + 1);

		node->localName = strdup( pLocalName );
		if( node->localName == NULL ) {
			free( node->prefix );
			node->prefix = NULL;    //no need to free really, main loop will frees it
			//when return code is not success
			return XML_INSUFFICIENT_MEMORY;
		}
	}

	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_xmlNamespace
*       add namespace definition. 
*       internal to parser only.			
*
*===============================================================================*/
static int
xml_parser_xmlNamespace( Parser * xmlParser,
                     XML_Node * newNode )
{

	XML_ElementStack *pCur = xmlParser->pCurElement;
	XML_NamespaceURI *pNewNs = NULL,
									 *pNs = NULL,
									 *pPrevNs = NULL;
	int rc;

	// if the newNode contains a namespace definition
	assert( newNode->nodeName != NULL );

	if( strcmp( newNode->nodeName, "xmlns" ) == 0 ) // default namespace def.
	{
		if( pCur->namespaceUri != NULL ) {
			free( pCur->namespaceUri );
		}

		pCur->namespaceUri = strdup( newNode->nodeValue ? newNode->nodeValue : "" );
		if( pCur->namespaceUri == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}

	} else if( strncmp( newNode->nodeName, "xmlns:", strlen( "xmlns:" ) ) == 0 ) {  // namespace definition
		rc = xml_parser_setNodePrefixAndLocalName( newNode );
		if( rc != XML_SUCCESS ) {
			return rc;
		}

		assert( newNode->localName != NULL );

		if( pCur == NULL ) {
			return XML_FAILED;
		}

		if( ( pCur->prefix != NULL )
				&& ( strcmp( pCur->prefix, newNode->localName ) == 0 ) ) {
			pCur->namespaceUri = strdup( newNode->nodeValue ? newNode->nodeValue:"" );
			if( pCur->namespaceUri == NULL ) {
				return XML_INSUFFICIENT_MEMORY;
			}
		} else {
			pPrevNs = pCur->pNsURI;
			pNs = pPrevNs;
			while( pNs != NULL ) {
				if( ( pNs->prefix != NULL ) &&
						( strcmp( pNs->prefix, newNode->localName ) == 0 ) ) {
					break;      // replace namespace definition
				} else {
					pPrevNs = pNs;
					pNs = pNs->nextNsURI;
				}
			}

			if( pNs == NULL )   // a new definition
			{
				pNewNs =
					( XML_NamespaceURI * )
					malloc( sizeof( XML_NamespaceURI ) );
				if( pNewNs == NULL ) {
					return XML_INSUFFICIENT_MEMORY;
				}
				memset( pNewNs, 0, sizeof( XML_NamespaceURI ) );

				pNewNs->prefix = strdup( newNode->localName ? newNode->localName : "" );
				if( pNewNs->prefix == NULL ) {
					free( pNewNs );
					return XML_INSUFFICIENT_MEMORY;
				}

				pNewNs->nsURI = strdup( newNode->nodeValue ? newNode->nodeValue:"" );
				if( pNewNs->nsURI == NULL ) {
					xml_parser_freeNsURI( pNewNs );
					free( pNewNs );
					return XML_INSUFFICIENT_MEMORY;
				}

				if( pCur->pNsURI == NULL ) {
					pCur->pNsURI = pNewNs;
				} else {
					pPrevNs->nextNsURI = pNewNs;
				}
			} else              // udpate the namespace
			{
				if( pNs->nsURI != NULL ) {
					free( pNs->nsURI );
				}

				pNs->nsURI = strdup( newNode->nodeValue ? newNode->nodeValue:"" );
				if( pNs->nsURI == NULL ) {
					return XML_INSUFFICIENT_MEMORY;
				}
			}
		}
	}

	if( xmlParser->pNeedPrefixNode != NULL ) {
		rc = xml_parser_addNamespace( xmlParser );
		return rc;
	} else {
		return XML_SUCCESS;
	}

}

/*==============================================================================*
*
*   xml_parser_processSTag:	
*       Processes the STag as defined by XML spec. 
*       Internal to parser only.			
*
*===============================================================================*/
static int
xml_parser_processSTag( Parser * xmlParser,
                    XML_Node * node )
{
	char *pCurToken = NULL;
	int rc;

	if( xml_parser_getNextToken( xmlParser ) == 0 ) {
		return XML_SYNTAX_ERR;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( pCurToken != NULL ) {
		node->nodeName = strdup( pCurToken );
		if( node->nodeName == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	} else {
		return XML_SYNTAX_ERR;
	}

	rc = xml_parser_setLastElem( xmlParser, node->nodeName );
	if( rc != XML_SUCCESS ) {  // no need to free node->nodeName, main loop will free it
		return XML_FAILED;
	}

	rc = xml_parser_setNodePrefixAndLocalName( node );
	if( rc != XML_SUCCESS ) {  // no need to free node->nodeName, main loop will free it
		return XML_FAILED;
	}

	node->nodeValue = NULL;
	node->nodeType = eELEMENT_NODE;

	xmlParser->savePtr = xmlParser->curPtr;
	if( xml_parser_getNextToken( xmlParser ) == 0 ) {   // no need to free node->nodeName, main loop will free it
		return XML_SYNTAX_ERR;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	// check to see what is the next token
	if( strcmp( pCurToken, "/>" ) == 0 )    // empty element 
	{
		xmlParser->state = eELEMENT;
		xmlParser->curPtr = xmlParser->savePtr; // backup to /> 
	} else if( strcmp( pCurToken, ">" ) == 0 )  // expecting text node
	{
		xmlParser->state = eCONTENT;
	} else {
		xmlParser->state = eATTRIBUTE;
		xmlParser->curPtr = xmlParser->savePtr;
	}

	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_hasDefaultNamespace   	
*       decide whether the current element has default namespace
*       Internal to parser only.       			
*
*===============================================================================*/
static BOOL
xml_parser_hasDefaultNamespace( Parser * xmlParser,
                            XML_Node * newNode,
                            char **nsURI )
{
	XML_ElementStack *pCur = xmlParser->pCurElement;

	while( pCur != NULL ) {
		if( ( pCur->prefix == NULL ) && ( pCur->namespaceUri != NULL ) ) {
			*nsURI = pCur->namespaceUri;
			return TRUE;
		} else {
			pCur = pCur->nextElement;
		}
	}

	return FALSE;

}

/*==============================================================================*
*
*   xml_parser_ElementPrefixDefined
*       decides whether element's prefix is already defined.
*       Internal to parser only. 
*
*===============================================================================*/
static BOOL
xml_parser_ElementPrefixDefined( Parser * xmlParser,
                             XML_Node * newNode,
                             char **nsURI )
{

	XML_ElementStack *pCur = xmlParser->pCurElement;
	XML_NamespaceURI *pNsUri;

	while( pCur != NULL ) {
		if( ( pCur->prefix != NULL )
				&& ( strcmp( pCur->prefix, newNode->prefix ) == 0 ) ) {
			*nsURI = pCur->namespaceUri;
			return TRUE;
		} else {
			pNsUri = pCur->pNsURI;

			while( pNsUri != NULL ) {
				if( strcmp( pNsUri->prefix, newNode->prefix ) == 0 ) {
					*nsURI = pNsUri->nsURI;
					return TRUE;
				} else {
					pNsUri = pNsUri->nextNsURI;
				}
			}
		}

		pCur = pCur->nextElement;

	}

	return FALSE;

}

/*==============================================================================*
*
*   xml_parser_processCDSect   
*       Processes CDSection as defined by XML spec.
*       Internal to parser only.
*
*===============================================================================*/
static int
xml_parser_processCDSect( char **pSrc,
                      XML_Node * node )
{

	char *pEnd;
	int tokenLength = 0;
	char *pCDataStart;

	if( *pSrc == NULL ) {
		return XML_FAILED;
	}

	pCDataStart = *pSrc + strlen( CDSTART );
	pEnd = pCDataStart;
	while( ( xml_parser_isXmlChar( *pEnd ) == TRUE ) && ( *pEnd != '\0' ) ) {
		if( strncmp( pEnd, CDEND, strlen( CDEND ) ) == 0 ) {
			break;
		} else {
			pEnd++;
		}
	}

	if( ( pEnd - pCDataStart > 0 ) && ( *pEnd != '\0' ) ) {
		tokenLength = pEnd - pCDataStart;
		node->nodeValue = ( char * )malloc( tokenLength + 1 );
		if( node->nodeValue == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
		strlcpy( node->nodeValue, pCDataStart, tokenLength + 1);
		node->nodeValue[tokenLength] = '\0';

		node->nodeName = strdup( CDATANODENAME );
		if( node->nodeName == NULL ) {
			// no need to free node->nodeValue at all, bacause node contents
			// will be freed by the main loop.
			return XML_INSUFFICIENT_MEMORY;
		}

		node->nodeType = eCDATA_SECTION_NODE;
		*pSrc = pEnd + strlen( CDEND );
		return XML_SUCCESS;
	} else {
		return XML_SYNTAX_ERR;
	}

}

/*==============================================================================*
*
*   xml_parser_setElementNamespace
*       set element's namespace
*       Internal to parser only.			
*
*===============================================================================*/
static int
xml_parser_setElementNamespace( XML_Element * newElement,
                            char *nsURI )
{
	if( newElement != NULL && nsURI != NULL ) {
		if( newElement->namespaceURI != NULL ) {
			return XML_SYNTAX_ERR;
		} else {
			newElement->namespaceURI = strdup( nsURI );
			if( newElement->namespaceURI == NULL ) {
				return XML_INSUFFICIENT_MEMORY;
			}
		}
	}

	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_processContent
*       processes the CONTENT as defined in XML spec.
*       Internal to parser only			
*
*===============================================================================*/
static int
xml_parser_processContent( Parser * xmlParser,
                       XML_Node * node )
{
	char *pEndContent;
	BOOL bReadContent;
	int tokenLength;
	char *notAllowed = "]]>";
	char *pCurToken = NULL;

	// save pointer for backup
	xmlParser->savePtr = xmlParser->curPtr;
	xml_parser_skipWhiteSpaces( xmlParser );

	if( *( xmlParser->curPtr ) == '\0' ) {  // end of file is reached
		return XML_SUCCESS;
	}

	pEndContent = xmlParser->curPtr;
	if( *pEndContent == LESSTHAN ) {
		if( strncmp( pEndContent, ( char * )CDSTART, strlen( CDSTART ) ) ==
				0 ) {
			if( xml_parser_processCDSect( &pEndContent, node ) !=
					XML_SUCCESS ) {
				return XML_SYNTAX_ERR;
			} else {
				xmlParser->curPtr = pEndContent;
			}
		} else
			if( strncmp
					( pEndContent, ( char * )BEGIN_COMMENT,
						strlen( BEGIN_COMMENT ) ) == 0 ) {
				if( xml_parser_skipComment( &pEndContent ) != XML_SUCCESS ) {
					return XML_SYNTAX_ERR;
				} else {
					xmlParser->curPtr = pEndContent;
				}
			} else
				if( strncmp
						( pEndContent, ( char * )BEGIN_PI,
							strlen( BEGIN_PI ) ) == 0 ) {
					if( xml_parser_skipPI( &pEndContent ) != XML_SUCCESS ) {
						return XML_SYNTAX_ERR;
					} else {
						xmlParser->curPtr = pEndContent;
					}
				} else                  // empty content
				{
					xmlParser->state = eELEMENT;
				}
	} else {
		// backup 
		xmlParser->curPtr = xmlParser->savePtr;
		pEndContent = xmlParser->curPtr;

		while( ( *pEndContent != LESSTHAN ) &&
				( strncmp
					( pEndContent, ( const char * )notAllowed,
						strlen( notAllowed ) ) != 0 ) && *pEndContent ) {
			pEndContent++;
		}

		if( *pEndContent == '\0' ) {
			bReadContent = FALSE;
		}

		if( strncmp
				( pEndContent, ( const char * )notAllowed,
					strlen( notAllowed ) ) == 0 ) {
			return XML_SYNTAX_ERR;
		}

		tokenLength = pEndContent - xmlParser->curPtr;
		xml_parser_clearTokenBuf( xmlParser );

		if( xml_parser_copyToken( xmlParser, xmlParser->curPtr, tokenLength )
				!= XML_SUCCESS ) {
			return XML_SYNTAX_ERR;
		}

		pCurToken = ( xmlParser->tokenBuf ).buf;
		if( pCurToken != NULL ) {
			node->nodeValue = strdup( pCurToken );
			if( node->nodeValue == NULL ) {
				return XML_INSUFFICIENT_MEMORY;
			}
		} else {
			return XML_SYNTAX_ERR;
		}

		node->nodeName = strdup( TEXTNODENAME );
		if( node->nodeName == NULL ) {
			return XML_SYNTAX_ERR;
		}
		node->nodeType = eTEXT_NODE;

		// adjust curPtr
		xmlParser->curPtr += tokenLength;

	}

	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_processETag
*       process ETag as defined by XML spec.
*       Internal to parser only.			
*
*===============================================================================*/
static int
xml_parser_processETag( Parser * xmlParser,
                    XML_Node * node,
                    BOOL * bETag )
{
	char *pCurToken = NULL;

	assert( xmlParser != NULL );
	if( xml_parser_getNextToken( xmlParser ) == 0 ) {
		return XML_SYNTAX_ERR;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( pCurToken == NULL ) {
		return XML_SYNTAX_ERR;
	}
	node->nodeName = strdup( pCurToken );
	if( node->nodeName == NULL ) {
		return XML_INSUFFICIENT_MEMORY;
	}

	node->nodeValue = NULL;
	node->nodeType = eELEMENT_NODE;

	xml_parser_skipWhiteSpaces( xmlParser );

	// read the > 
	if( xml_parser_getNextToken( xmlParser ) == 0 ) {
		return XML_SYNTAX_ERR;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( pCurToken == NULL ) {   // no need to free node->nodeName, it is freed by main loop
		return XML_SYNTAX_ERR;
	}

	if( strcmp( pCurToken, ">" ) != 0 ) {
		return XML_SYNTAX_ERR;
	}

	*bETag = TRUE;
	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_freeElementStackItem
*       frees one ElementStack item.
*       Internal to parser only.			
*
*===============================================================================*/
static void
xml_parser_freeElementStackItem( XML_ElementStack * pItem )
{
	assert( pItem != NULL );
	if( pItem->element != NULL ) {
		free( pItem->element );
		pItem->element = NULL;
	}
	if( pItem->namespaceUri != NULL ) {
		free( pItem->namespaceUri );
		pItem->namespaceUri = NULL;
	}
	if( pItem->prefix != NULL ) {
		free( pItem->prefix );
		pItem->prefix = NULL;
	}

}

/*==============================================================================*
*
*   xml_parser_freeNsURI
*       frees namespaceURI item.
*       Internal to parser only. 		
*
*===============================================================================*/
static void
xml_parser_freeNsURI( XML_NamespaceURI * pNsURI )
{
	assert( pNsURI != NULL );
	if( pNsURI->nsURI != NULL ) {
		free( pNsURI->nsURI );
	}
	if( pNsURI->prefix != NULL ) {
		free( pNsURI->prefix );
	}
}

/*==============================================================================*
*
*   xml_parser_free
*       frees all temporary memory allocated by xmlparser.
*       Internal to parser only       
*			
*
*===============================================================================*/
static void
xml_parser_free( Parser * xmlParser )
{
	XML_ElementStack *pElement,
									 *pNextElement;
	XML_NamespaceURI *pNsURI,
									 *pNextNsURI;

	if( xmlParser == NULL ) {
		return;
	}

	if( xmlParser->dataBuffer != NULL ) {
		free( xmlParser->dataBuffer );
	}

	xml_membuf_destroy( &( xmlParser->tokenBuf ) );
	xml_membuf_destroy( &( xmlParser->lastElem ) );

	pElement = xmlParser->pCurElement;
	while( pElement != NULL ) {
		xml_parser_freeElementStackItem( pElement );

		pNsURI = pElement->pNsURI;
		while( pNsURI != NULL ) {
			pNextNsURI = pNsURI->nextNsURI;
			xml_parser_freeNsURI( pNsURI );
			free( pNsURI );
			pNsURI = pNextNsURI;
		}

		pNextElement = pElement->nextElement;
		free( pElement );
		pElement = pNextElement;
	}

	free( xmlParser );

}

/*==============================================================================*
*
*   xml_parser_parseReference	
*       return XML_SUCCESS or not
*			
*
*===============================================================================*/
static int
xml_parser_parseReference( char *pStr )
{
	// place holder for future implementation
	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_processAttribute	
*       processes attribute.
*       Internal to parser only.
*       returns XML_SUCCESS or failure
*			
*
*===============================================================================*/
static int
xml_parser_processAttribute( Parser * xmlParser,
                         XML_Node * node )
{

	char *strEndQuote = NULL;
	int tlen = 0;
	char *pCur = NULL;
	char *pCurToken = NULL;

	assert( xmlParser );
	if( xmlParser == NULL ) {
		return XML_FAILED;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( pCurToken == NULL ) {
		return XML_SYNTAX_ERR;
	}

	if( xml_parser_isNameChar( xml_parser_UTF8ToInt( pCurToken, &tlen ), FALSE ) ==
			FALSE ) {
		return XML_SYNTAX_ERR;
	}
	// copy in the attribute name
	node->nodeName = strdup( pCurToken );
	if( node->nodeName == NULL ) {
		return XML_INSUFFICIENT_MEMORY;
	}
	// read in the "=" sign 
	if( xml_parser_getNextToken( xmlParser ) == 0 ) {
		return XML_SYNTAX_ERR;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( *pCurToken != EQUALS ) {
		return XML_SYNTAX_ERR;
	}
	// read in the single quote or double quote
	if( xml_parser_getNextToken( xmlParser ) == 0 ) {
		return XML_SYNTAX_ERR;
	}
	// pCurToken is either quote or singlequote
	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( ( *pCurToken != QUOTE ) && ( *pCurToken != SINGLEQUOTE ) ) {
		return XML_SYNTAX_ERR;
	}

	strEndQuote = strstr( xmlParser->curPtr, pCurToken );
	if( strEndQuote == NULL ) {
		return XML_SYNTAX_ERR;
	}
	// check between curPtr and strEndQuote, whether there are illegal chars.
	pCur = xmlParser->curPtr;
	while( pCur < strEndQuote ) {
		if( *pCur == '<' ) {
			return XML_SYNTAX_ERR;
		}

		if( *pCur == '&' ) {
			xml_parser_parseReference( ++pCur );
		}
		pCur++;
	}
	//clear token buffer
	xml_parser_clearTokenBuf( xmlParser );
	if( strEndQuote != xmlParser->curPtr ) {
		if( xml_parser_copyToken( xmlParser, xmlParser->curPtr,
					strEndQuote - xmlParser->curPtr ) !=
				XML_SUCCESS ) {
			return XML_SYNTAX_ERR;
		}
	}
	// skip the ending quote
	xmlParser->curPtr = strEndQuote + 1;

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( pCurToken != NULL ) {   // attribute has value, like a="c"
		node->nodeValue = strdup( pCurToken );
		if( node->nodeValue == NULL ) {
			return XML_INSUFFICIENT_MEMORY;
		}
	}
	// if attribute doesn't have value, like a="", fine
	node->nodeType = eATTRIBUTE_NODE;

	// check whether this is a new namespace definition
	if( xml_parser_xmlNamespace( xmlParser, node ) != XML_SUCCESS ) {
		return XML_FAILED;
	}
	// read ahead to see whether we have more attributes
	xmlParser->savePtr = xmlParser->curPtr;
	if( xml_parser_getNextToken( xmlParser ) == 0 ) {
		return XML_SYNTAX_ERR;
	}

	pCurToken = ( xmlParser->tokenBuf ).buf;
	if( strcmp( pCurToken, "<" ) == 0 ) {
		return XML_FAILED;
	} else if( strcmp( pCurToken, ">" ) != 0 )  // more attribute?
	{                           // backup
		xmlParser->curPtr = xmlParser->savePtr;
	} else {
		xmlParser->state = eCONTENT;
	}

	return XML_SUCCESS;
}

/*==============================================================================*
*
*   xml_parser_getNextNode
*       return next node 
*   returns XML_SUCCESS or 
*			
*
*===============================================================================*/
static int
xml_parser_getNextNode( Parser * xmlParser,
                    XML_Node * node,
                    BOOL * bETag )
{
	char *pCurToken = NULL;
	char *lastElement = NULL;

	// endof file reached?
	if( *( xmlParser->curPtr ) == '\0' ) {
		*bETag = TRUE;
		return XML_FILE_DONE;
	}

	if( xmlParser->state == eCONTENT ) {
		if( xml_parser_processContent( xmlParser, node ) != XML_SUCCESS ) {
			return XML_FAILED;
		}
	} else {
		xml_parser_skipWhiteSpaces( xmlParser );

		if( ( xml_parser_getNextToken( xmlParser ) == 0 ) &&
				( xmlParser->pCurElement == NULL ) &&
				( *( xmlParser->curPtr ) == '\0' ) ) {   // comments after the xml doc
			return XML_SUCCESS;
		} else if( ( xmlParser->tokenBuf ).length == 0 ) {
			return XML_SYNTAX_ERR;
		}

		pCurToken = ( xmlParser->tokenBuf ).buf;
		if( *pCurToken == GREATERTHAN ) {
			return XML_SUCCESS;
		} else if( strcmp( pCurToken, ENDTAG ) == 0 ) { //  we got </, read next element
			return xml_parser_processETag( xmlParser, node, bETag );
		} else if( *pCurToken == LESSTHAN ) {
			return xml_parser_processSTag( xmlParser, node );
		} else if( strcmp( pCurToken, COMPLETETAG ) == 0 ) {
			lastElement = ( xmlParser->lastElem ).buf;
			if( lastElement == NULL ) {
				goto ErrorHandler;
			}

			node->nodeName = strdup( lastElement );
			if( node->nodeName == NULL ) {
				return XML_INSUFFICIENT_MEMORY;
			}
			node->nodeType = eELEMENT_NODE;
			*bETag = TRUE;

			return XML_SUCCESS;
		} else if( xmlParser->state == eATTRIBUTE ) {
			if( xml_parser_processAttribute( xmlParser, node ) !=
					XML_SUCCESS ) {
				return XML_SYNTAX_ERR;
			}
		} else {
			return XML_SYNTAX_ERR;
		}
	}

	return XML_SUCCESS;

ErrorHandler:

	return XML_SYNTAX_ERR;

}

/*================================================================
*   xml_parser_relax
*       Makes the XML parser more tolerant to malformed text.
*       External function.
*
*=================================================================*/
void
xml_parser_relax(char errorChar)
{
	xml_parser_setErrorChar( errorChar );
}
