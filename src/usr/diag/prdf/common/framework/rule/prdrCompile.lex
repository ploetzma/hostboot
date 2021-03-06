/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/framework/rule/prdrCompile.lex $     */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2014                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/* Pre C stuff: headers, etc. */
%{

/** @file prdrCompile.lex
 *
 *  This file contains all of the flex code for parsing rule-table tokens.
 */

#include <stdlib.h>
// Portable formatting of uint64_t.  The ISO C99 standard requires
// __STDC_FORMAT_MACROS to be defined in order for PRIx64 etc. to be defined.
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <prdrToken.H>            // Token structure definition.
#include <prdrCompile.y.H>  // Token enums from yacc code.
%}

/* Suppress "yyunput defined but not used" warnings */
%option nounput

/* --- Basic type definitions --- */

/* Digits */
digit       [0-9]
hexdigit    [0-9a-fA-F]

/* Numerical constants */
integer     {digit}+
hexint      0[xX]{hexdigit}+
    /* Bit-string is a hex string between two back-ticks */
bitstring   `{hexdigit}+`

/* White space */
whitespace  [ \t]*
newline     \n

/* # starts a comment line */
comment     #.*{newline}

/* IDs are any letter or underscore followed by any number of letters/numbers */
id      [A-Za-z_][A-Za-z0-9_]*

/* --- end Basic type definitions --- */

/* Define special parse contexts for comments and .include headers */
%x DOX_COMMENT
%x INCLUDED

/* --- Begin Token Definitions --- */
%%

 /* Parse numerical constants to "INTEGER" type. */
{integer}   { sscanf(yytext, "%" PRIu64, &yylval.long_integer);
              return PRDR_INTEGER; }
{hexint}    { sscanf(yytext, "%" PRIx64, &yylval.long_integer);
              return PRDR_INTEGER; }

 /* Parse a bitstring to "BIT_STRING" type. */
{bitstring} {
            yylval.str_ptr = new std::string(yytext);
            return PRDR_BIT_STRING;
            }
 /* Parse a string to a "STRING" type.  Any number of characters between two
  * quotes.
  */
\"[^"]*\"   {
            yylval.str_ptr = new std::string(yytext);
            return PRDR_STRING;
            }
 /* Special end-of-file character. */
<<EOF>>     { return 0; }

 /* Various keyword tokens converted directly to the enum type. */
chipid      { return PRDR_CHIPID; }
sigoff      { return PRDR_SIGNATURE_OFFSET; }
PRDR_ERROR_SIGNATURE { return PRDR_SIGNATURE_EXTRA; }
targettype  { return PRDR_TARGETTYPE; }
register    { return PRDR_REGISTER; }
name        { return PRDR_NAME_KW; }
scomaddr    { return PRDR_SCOMADDR; }
scomlen     { return PRDR_SCOMLEN; }
access      { return PRDR_REGISTER_ACCESS; }
read_only   { return PRDR_REGISTER_READ_ACCESS; }
write_only  { return PRDR_REGISTER_WRITE_ACCESS; }
no_access   { return PRDR_REGISTER_NO_ACCESS; }
bit         { return PRDR_BIT_KW; }
reset       { return PRDR_RESET_ADDR; }
mask        { return PRDR_MASK_ADDR; }

chip        { return PRDR_CHIP; }
group       { return PRDR_GROUP; }
type        { return PRDR_TYPE; }  /* @jl04 a Add this for primary/secondary type.*/
actionclass { return PRDR_ACTIONCLASS; }
rule        { return PRDR_RULE; }

threshold   { return PRDR_ACT_THRESHOLD; }
analyse     { return PRDR_ACT_ANALYSE; }
analyze     { return PRDR_ACT_ANALYSE; }
try         { return PRDR_ACT_TRY; }
dump        { return PRDR_ACT_DUMP; }
funccall    { return PRDR_ACT_FUNCCALL; }
gard        { return PRDR_ACT_GARD; }
callout     { return PRDR_ACT_CALLOUT; }
flag        { return PRDR_ACT_FLAG; }
capture     { return PRDR_ACT_CAPTURE; }

connected   { return PRDR_CONNECTED; }
connected_peer   { return PRDR_CONNECTED_PEER; }
nonzero     { return PRDR_ACT_NONZERO; }
alternate   { return PRDR_ALTERNATE; }
procedure   { return PRDR_PROCEDURE; }

attntype    { return PRDR_ATTNTYPE; }
shared      { return PRDR_SHARED_KW; }
req         { return PRDR_REQUIRED_KW; }
field       { return PRDR_FLD_KW; }
mfg         { return PRDR_MFG_KW; }
mfg_file    { return PRDR_MFG_FILE_KW; }
sec         { return PRDR_TIME_SEC; }
min         { return PRDR_TIME_MIN; }
hour        { return PRDR_TIME_HOUR; }
day         { return PRDR_TIME_DAY; }

filter      { return PRDR_FILTER; }
singlebit   { return PRDR_FILTER_SINGLE_BIT; }
priority    { return PRDR_FILTER_PRIORITY; }
secondarybits   { return PRDR_FILTER_SECONDARY; }

"\<\<"      { return PRDR_OP_LEFTSHIFT; }
"\>\>"      { return PRDR_OP_RIGHTSHIFT; }

 /* Parse an "ID" type */
{id}        { yylval.str_ptr = new std::string(yytext); return PRDR_ID;}

 /* Ignore extra white space */
{whitespace}    { }
 /* Newline or comment line increments line count */
{newline}   { yyline++; }
{comment}   { yyline++; }

 /* Any other arbitrary character is returned unchanged (used for parens, |,
  * {, etc. in yacc code).
  */
.           { return yytext[0]; }

 /* When we find the .included directive, we need to enter a special parse
  * context.  There is a preprocessor that runs that changes .include directives
  * to a .included / .end_included pair.  This is used for line counting on
  * errors.
  */
"\.included"        BEGIN INCLUDED;
 /* Ignore extra whitespace */
<INCLUDED>{whitespace}  { }
 /* Find the name of the file that was included, push current file and line
  * number on to a "stack".  When the included file is complete, we pop a pair
  * of the stack to determine where we left off in the old file.
  */
<INCLUDED>\".*\"                {
                                    yyincfiles.push(
                                        std::pair<std::string,int>(
                                            std::string(yytext),
                                            yyline)
                                        );
                                    yyline = 1;
                                }
 /* The newline after the .included indicates the .included directive is
  * complete.  We then return to the "INITIAL" context to parse the included
  * file properly.
  */
<INCLUDED>{newline}                BEGIN INITIAL;
 /* The .end_included directive indicates an included file has ended.  Pop the
  * parent file/line number off the stack.
  */
"\.end_included"                 {
                                    yyline = yyincfiles.top().second;
                                    yyincfiles.pop();
                                }

 /* A "slash-star-star" indicates a special comment context.  This is used for
  * the doxygen-style commenting and HTML documentation generation.
  */
"/**"+[ \t]*                        BEGIN DOX_COMMENT;
 /* A "star-slash" indicates the end of a doxygen comment context. (just like
  * C++)
  */
<DOX_COMMENT>[ \t]*\*[/]        BEGIN INITIAL;
 /* Any number of tabs at the beginning of a line, followed by a star followed
  * by anything but a slash, followed by any number of tabs is ignored.
  */
<DOX_COMMENT>\n[ \t]*\*[^/][ \t]*        { yyline++; return PRDR_DOX_ENDL; }
 /* Find any comment line itself (non-star, non-newline) */
<DOX_COMMENT>[^*\n]*                {
                                    yylval.str_ptr = new std::string(yytext);
                                    return PRDR_DOX_COMMENT;
                                }
 /* New-line in a comment is a special token. */
<DOX_COMMENT>\n                        { yyline++; return PRDR_DOX_ENDL; }
%%

/* User Code */
int yywrap() { return 1;}; // We're only parsing one file, so always return 1.
                           // This is a lex-ism.

