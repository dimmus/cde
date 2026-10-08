 /*
  * $TOG: tokenStyle.l /main/6 1998/04/17 11:50:07 mgreess $
  *
  * Copyright (c) 1993 HAL Computer Systems International, Ltd.
  * All rights reserved.  Unpublished -- rights reserved under
  * the Copyright Laws of the United States.  USE OF A COPYRIGHT
  * NOTICE IS PRECAUTIONARY ONLY AND DOES NOT IMPLY PUBLICATION
  * OR DISCLOSURE.
  *
  * THIS SOFTWARE CONTAINS CONFIDENTIAL INFORMATION AND TRADE
  * SECRETS OF HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.  USE,
  * DISCLOSURE, OR REPRODUCTION IS PROHIBITED WITHOUT THE
  * PRIOR EXPRESS WRITTEN PERMISSION OF HAL COMPUTER SYSTEMS
  * INTERNATIONAL, LTD.
  *
  *                         RESTRICTED RIGHTS LEGEND
  * Use, duplication, or disclosure by the Government is subject
  * to the restrictions as set forth in subparagraph (c)(l)(ii)
  * of the Rights in Technical Data and Computer Software clause
  * at DFARS 252.227-7013.
  *
  *          HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.
  *                  1315 Dell Avenue
  *                  Campbell, CA  95008
  *
  */

%option noyywrap

%a 30000
%e 10000
%k 10000
%n 10000
%o 40000
%p 20000

%{
#include <string.h>
#include "ParserConst.h"
#include "Expression.h"
#include "FeatureValue.h"
#include "PathTable.h"
#include "SSPath.h"
#include "PathQualifier.h"
#include "StyleSheetExceptions.h"
#include "style.tab.h"
#include "Debug.h"
#include <iostream>

istream *g_stylein = 0;

#define YY_INPUT(buf,result,max_size)\
  {\
     if (g_stylein -> eof()) {\
        result=0;\
     } else {\
        g_stylein -> read((char*)buf, max_size-1); \
        result = g_stylein -> gcount(); \
        buf[result] = 0; \
     }\
  }

unsigned char* qstring_buf = new unsigned char[1024];
int qstring_buf_size = 1024;
int qstring_buf_content_size = 0;

char* commentBuffer = new char [1024];
int commentBufferSize = 1024;
int commentBufferContentSize = 0;

void addToQstringBuf(const unsigned char* str, int size)
{
   if ( size <= 0 ) return;

   if ( qstring_buf_size - qstring_buf_content_size < size ) {
      qstring_buf_size = 2*(size+qstring_buf_content_size);
      unsigned char* x = new unsigned char[qstring_buf_size];
      memcpy(x, qstring_buf, qstring_buf_content_size);
      delete [] qstring_buf;
      qstring_buf = x;
   }

   memcpy(qstring_buf + qstring_buf_content_size, str, size);
   qstring_buf_content_size += size;
   qstring_buf[qstring_buf_content_size] = 0;
}


%}
unit ([Ii][Nn]|[Ii][Nn][Cc][Hh]|[Pp][Cc]|[Pp][Ii][Cc][Aa]|[Pp][Tt]|[Pp][Oo][Ii][Nn][Tt]|[Pp][Ii][Xx][Ee][Ll]|[Cc][Mm])

%x block sgmlgimode quoted_string

%%

"#|"	BEGIN(block);

^"#".*	{
	   if ( commentBufferSize < yyleng ) {
              delete [] commentBuffer;
              commentBufferSize = 2 * yyleng ;
              commentBuffer = new char [commentBufferSize];
           }

	   commentBufferContentSize = yyleng-1;
           memcpy(commentBuffer, yytext+1, commentBufferContentSize); // copy everything except the #
           commentBuffer[commentBufferContentSize] = 0;
	}

"="	{
	   return(OPER_assign);
	}

"@"	{
	   return(OPER_attr);
	}

[+]	{
           stylelval.charData = yytext[0];
	   return(OPER_plus);
	}

[-]	{
           stylelval.charData = yytext[0];
	   return(OPER_minus);
	}

"/"	{
           stylelval.charData = yytext[0];
	   return(OPER_div);
	}

"."	{
	   return(OPER_period);
	}

"*"	{
           stylelval.charData = yytext[0];
	   return(OPER_star);
	}

":"	{
	   return(OPER_modify);
	}

"?"	{
	   return(OPER_oneof);
	}

"^"	{
	   return(OPER_parent);
	}

","	{
	   return(SEPARATOR);
	}

"{"	{
	   return(FSOPEN);
	}

"}"	{
	   return(FSCLOSE);
	}

"["	{
	   return(ARRAYOPEN);
	}

"]"	{
	   return(ARRAYCLOSE);
	}

"("	{
	   return(OPER_parenopen);
	}

")"	{
	   return(OPER_parenclose);
	}

"||"	{
	   return(OPER_or);
	}

"&&"	{
	   return(OPER_and);
	}

"=="|"!="	{
           if ( strcmp((const char*)yytext, "==") == 0 )
              stylelval.intData = EQUAL;
 	   else
              stylelval.intData = NOT_EQUAL;

	   return(OPER_equality);
	}

"!"	{
	   return(OPER_logicalnegate);
	}

"<="|"<"|">="|">"	{
           if ( strcmp((const char*)yytext, "<=") == 0 )
              stylelval.intData = LESS_OR_EQUAL;
 	   else
           if ( strcmp((const char*)yytext, "<") == 0 )
              stylelval.intData = LESS;
 	   else
           if ( strcmp((const char*)yytext, ">=") == 0 )
              stylelval.intData = GREATER_OR_EQUAL;
 	   else
              stylelval.intData = GREATER;

	   return(OPER_relational);
	}

"GICaseSensitive"	{
		return(GI_CASE_SENSITIVE);
			}

[Tt][Rr][Uu][Ee]	{
                stylelval.boolData = true;
		return(BOOLVAL);
			}

[Ff][Aa][Ll][Ss][Ee]	{
                stylelval.boolData = false;
		return(BOOLVAL);
			}

[On][Nn]	{
                stylelval.boolData = true;
		return(BOOLVAL);
		}

[Oo][Ff][Ff]	{
                stylelval.boolData = false;
		return(BOOLVAL);
		}

[0-9]+("."[0-9]+)?{unit} 	{
		stylelval.charPtrData =
                  (unsigned char*)strdup((const char*)yytext);
		return(DIMENSION);
		}

[0-9]+		{
		stylelval.intData = atoi((char*)yytext);
		return(INTEGER);
		}

[0-9]+"."[0-9]+	{
		stylelval.realData = atof((char*)yytext);
		return(REAL);
		}

\"		{
		BEGIN quoted_string;
		}

<quoted_string>\"	{

		stylelval.charPtrData =
			new unsigned char[qstring_buf_content_size+1];
		memcpy( stylelval.charPtrData,
			qstring_buf,
			qstring_buf_content_size+1
		      );

   		qstring_buf_content_size = 0;
		BEGIN 0;

		return(QUOTED_STRING);
		}

<quoted_string>\\	{
		int c = yyinput();
		switch (c) {
		   case '"':
		     addToQstringBuf((unsigned char*)"\"", 1);
		     break;
		   case '\\':
		     addToQstringBuf((unsigned char*)"\\", 1);
		     break;
		   default:
                     throw(CASTSSEXCEPT StyleSheetException());
		}
		}

<quoted_string>[^\\\"]*	{
		addToQstringBuf((unsigned char*)yytext, yyleng);
		}

<quoted_string>.	{
		addToQstringBuf((unsigned char*)yytext, yyleng);
		}

{unit}		{
		stylelval.charPtrData =
                  (unsigned char*)strdup((const char*)yytext);
		  return(UNIT_STRING);
		}

[^ \t\n\".=@+*\/\.\*:?\^,{}\[\]()!]+	{
		stylelval.charPtrData =
                  (unsigned char*)strdup((const char*)yytext);
		return(NORMAL_STRING);
		}

<sgmlgimode>[0-9a-zA-Z\.\-]+ {
		stylelval.charPtrData =
                  (unsigned char*)strdup((const char*)yytext);
		BEGIN 0;
		return(SGMLGI_STRING);
	}

[\t]	{
	}

[\n]	{
	   yylineno++;
	}

.	{
	}



<block>"|#"	{ BEGIN(0); }
<block>.	;
<block>\n	;
%%

void enter_sgmlgi_context()
{
   BEGIN sgmlgimode;
}


void report_error_location()
{
   if ( commentBufferContentSize > 0 ) {
      cerr << commentBuffer << "\n";
   }
}

void styleerror(char* msg)
{
#ifdef DEBUG
   cerr << "line " << yylineno << ": " << msg << "\n";
#endif
   throw(CASTSSSEEXCEPT StyleSheetSyntaxError());
}

