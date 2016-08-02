/*
 * This is a generated file. Modifications are futile.
 */

/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#ifndef POLYGRAPH__PGL_PGLPARSER_H
#define POLYGRAPH__PGL_PGLPARSER_H

#include "xparser/GParser.h"

class PglParser : public GParser {
	public:
		PglParser(Lexer *l);
		virtual ~PglParser();

	protected:
		virtual int actionTable(int, int);
		virtual ReduceInfo *reduceTable(int);
};

// these are needed by lexer, argh!
/* internal _tokens */
#define _ERR_TOKEN (0)
#define _EOF_TOKEN (1)
/* tokens defined in the language specs */
#define ID_TOKEN (2)
#define INT_TOKEN (3)
#define NUM_TOKEN (4)
#define TIME_SCALE_TOKEN (5)
#define TIME_TOKEN (6)
#define SIZE_SCALE_TOKEN (7)
#define SIZE_TOKEN (8)
#define BOOL_TRUE_TOKEN (9)
#define BOOL_FALSE_TOKEN (10)
#define SQW_STR_TOKEN (11)
#define DQW_STR_TOKEN (12)
#define BQW_STR_TOKEN (13)
#define NUM_PERC_TOKEN (14)
#define CMNT_LINE_TOKEN (15)
#define CMNT_OPEN_TOKEN (16)
#define CMNT_CLOSE_TOKEN (17)
#define EQUAL_TOKEN (18)
#define NEQUAL_TOKEN (19)
#define LT_TOKEN (20)
#define LTE_TOKEN (21)
#define GT_TOKEN (22)
#define GTE_TOKEN (23)
#define BOOL_AND_TOKEN (24)
#define BOOL_OR_TOKEN (25)
#define BOOL_XOR_TOKEN (26)
#define NOT_TOKEN (27)
#define PLUS_TOKEN (28)
#define MINUS_TOKEN (29)
#define MUL_TOKEN (30)
#define DIV_TOKEN (31)
#define MOD_TOKEN (32)
#define POWER_TOKEN (33)
#define ASGN_TOKEN (34)
#define LEFTPARENT_TOKEN (35)
#define RIGHTPARENT_TOKEN (36)
#define RIGHTBRACKET_TOKEN (37)
#define LEFTBRACKET_TOKEN (38)
#define LEFTBRACE_TOKEN (39)
#define RIGHTBRACE_TOKEN (40)
#define PERIOD_TOKEN (41)
#define SEMICOLON_TOKEN (42)
#define COLON_TOKEN (43)
#define COMMA_TOKEN (44)
#define POUND_TOKEN (45)
#define THRU_TOKEN (46)
#define CLONE_TOKEN (47)
#define kw_lmt_TOKEN (48)
#define kw_now_TOKEN (49)
#define kw_nmt_TOKEN (50)
#define kw_if_TOKEN (51)
#define kw_then_TOKEN (52)
#define kw_else_TOKEN (53)
#define kw_every_TOKEN (54)
#define kw_do_TOKEN (55)
#define RE_CUSTOM_TOKEN (56)
#define RE_DEFAULT_TOKEN (57)


#endif
