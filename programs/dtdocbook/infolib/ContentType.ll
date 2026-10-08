%option noyywrap

%{
/* $XConsortium: ContentType.l /main/2 1996/11/19 16:54:22 drk $ */

#include <iostream>
#include <sstream>
using namespace std;

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "dti_excs/Exceptions.hh"

#include "Task.h"
#include "SGMLName.h"
#include "ExprList.h"
#include "Expression.h"
#include "OL_DataExpr.tab.h"
#include "ContentType.h"
#include "api/utility.h"

/* CurrentContentPtr is used as the communication media between
 * ol_dataparse and ContentType::Parse()
 */

ContentType *CurrentContentPtr;
  
/*
 * Forward declaration for my_input
 */
static int my_input ( char *, int );
extern       int ol_dataparse();

  
#undef YY_INPUT
#define YY_INPUT(b, r, ms ) ( r=my_input( ( char *)b,ms) )
  
static char *myinput;
static char *myinputptr;
static char *myinputlim;

// Debugging macro
#ifdef DEBUG
#define DBG(level) if ( dbgLevel >= level)
#else
#define DBG(level) if (0)
#endif

static int dbgLevel = -1;

%}

%%

"@"[a-zA-Z0-9]+                 {
                                  ol_datalval.name = SGMLName::intern((const char *)yytext+1 ,1); 
				  return( Reference );
				}
["][^"]*["]                     {
                                  if ( *(yytext + 1) != '"' ) {
                                     // get rid of the 2 quotes 
                                     int len = strlen(( const char *)yytext)-2;
                                     char *lit_str = new char [ len + 1 ];
                                     strncpy ( lit_str, 
                                               (const char *)yytext + 1, 
                                               len );

                                     *(lit_str + len) = '\0';
                                     ol_datalval.string = lit_str;
                                  }
                                  else {
                                     ol_datalval.string = 0;
                                  }

                                  DBG(50) cerr << "(DEBUG) literal \"string\" = "
                                               << ol_datalval.string << endl;

                                  return( Literal );
                                }
['][^']*[']                     {
                                  if ( *(yytext + 1) != '\'' ) {
                                     // get rid of the 2 quotes 
                                     int len = strlen(( const char *)yytext)-2;
                                     char *lit_str = new char [ len + 1 ];
                                     strncpy ( lit_str, 
                                               (const char *)yytext + 1, 
                                               len );

                                     *(lit_str + len) = '\0';
                                     ol_datalval.string = lit_str;
                                     
                                  }
                                  else {
                                     ol_datalval.string = 0;
                                  }

	                          DBG(50) cerr << "(DEBUG) literal 'string' = "
	                                       << ol_datalval.string << endl;
	 
                                  return( Literal );
                                }

[Aa][Tt][Tt][Rr]                { return( Attr );      }
[Cc][Oo][Nn][Cc][Aa][Tt]        { return( Concat );    }
[Ff][Ii][Rr][Ss][Tt][Oo][Ff]    { return( FirstOf);    }
"#"[Cc][Oo][Nn][Tt][Ee][Nn][Tt] { return( Content );   }
[^(,)\n\t ]+                    {
                                  ol_datalval.name = SGMLName::intern((const char *)yytext,1);
				  DBG(10) cerr << "(DEBUG) matches"
				               << (char *)SGMLName::lookup(ol_datalval.name)
					       << endl;
				  return ( Id );
				}
[(,)]                           {
                                  DBG(10) cerr << "(DEBUG) matches"
				               << (char *)yytext
					       << endl;
				  return ( yytext[0] );
				}
[\n\t ]+                        ;
.                               {
                                  throw(Unexpected(
						   "Syntax error in value expression"));

				}
%%

static int
my_input ( char *buf, int max_size )
{

  int remain = myinputlim - myinputptr;
  int n = ( max_size > remain ? remain : max_size );

  if ( n > 0 ) {
    memcpy ( buf, myinputptr, n );
    myinputptr += n;
  }
  return n;
}

//--------------------------------------------------------------
void
ol_dataerror(const char *str)
{
  throw(Unexpected(form("Syntax error in %s", myinput)));
}

//--------------------------------------------------------------
ContentType::ContentType()
{
  char *dbgStr;
  dbgStr = getenv ("OL_DEBUG");
  dbgLevel = ( dbgStr ? atoi ( dbgStr ) : 0 );
  exprlist  = 0;
}

//--------------------------------------------------------------
ContentType::~ContentType()
{

   OL_Expression *eptr = exprlist;
   while ( eptr ) {
      OL_Expression *tmp = eptr;
      eptr = eptr->next;
      delete tmp;
   }
}

//--------------------------------------------------------------
void
ContentType::Parse( char *str )
{

  DBG(10) cerr << "(DEBUG) ContentType::Parse() str = " << str << endl;

  myinput = str;
  myinputptr = str;
  myinputlim = str + strlen(str);

  CurrentContentPtr = this;
  
  ol_dataparse();
  
  BEGIN INITIAL;
  yyrestart(NULL);
}

//--------------------------------------------------------------
void
ContentType::init( OL_Expression *expr )
{
  assert(expr);
  if ( expr->next ) {
    
    /*
     * The same as CONCAT
     */
    ExprList *elist = new ExprList( expr );
    OL_Expression *new_expr = new OL_Expression ( CONCAT, -1, elist );
    assert(new_expr);

    exprlist = new_expr;
  }
  else {
    exprlist = expr;
  }
}
