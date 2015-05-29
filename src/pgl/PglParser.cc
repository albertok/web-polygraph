/*
 * This is a generated file. Modifications are futile.
 */

/* Web Polygraph       http://www.web-polygraph.org/
 * Copyright 2003-2011 The Measurement Factory
 * Licensed under the Apache License, Version 2.0 */

#include "pgl/pgl.h"

#include "xparser/ParsSym.h"
#include "pgl/PglParser.h"





static
SynSym *p_StatementSeq_1(SynSym **) {
	// StatementSeq = .
	ParsSym *p = new ParsSym("StatementSeq", 0);
	return p;
}

static
SynSym *p_SimpleStatement_2(SynSym **) {
	// SimpleStatement = .
	ParsSym *p = new ParsSym("SimpleStatement", 0);
	return p;
}

static
SynSym *p_Code_3(SynSym **base) {
	// Code = StatementSeq .
	ParsSym *p = new ParsSym("Code", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_StatementSeq_4(SynSym **base) {
	// StatementSeq = Statement StatementSeq .
	ParsSym *p = new ParsSym("StatementSeq", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Statement_5(SynSym **base) {
	// Statement = SimpleStatement SEMICOLON .
	ParsSym *p = new ParsSym("Statement", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Statement_6(SynSym **base) {
	// Statement = IfCode .
	ParsSym *p = new ParsSym("Statement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Statement_7(SynSym **base) {
	// Statement = EveryCode .
	ParsSym *p = new ParsSym("Statement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Statement_8(SynSym **base) {
	// Statement = LEFTBRACE Code RIGHTBRACE .
	ParsSym *p = new ParsSym("Statement", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_SimpleStatement_9(SynSym **base) {
	// SimpleStatement = ExprStatement .
	ParsSym *p = new ParsSym("SimpleStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_SimpleStatement_10(SynSym **base) {
	// SimpleStatement = DeclStatement .
	ParsSym *p = new ParsSym("SimpleStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_DeclStatement_11(SynSym **base) {
	// DeclStatement = PureDecl .
	ParsSym *p = new ParsSym("DeclStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_DeclStatement_12(SynSym **base) {
	// DeclStatement = ExprInitDecl .
	ParsSym *p = new ParsSym("DeclStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_DeclStatement_13(SynSym **base) {
	// DeclStatement = CodeInitDecl .
	ParsSym *p = new ParsSym("DeclStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_ExprStatement_14(SynSym **base) {
	// ExprStatement = Assignment .
	ParsSym *p = new ParsSym("ExprStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_ExprStatement_15(SynSym **base) {
	// ExprStatement = Call .
	ParsSym *p = new ParsSym("ExprStatement", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_PureDecl_16(SynSym **base) {
	// PureDecl = TypeName ObjName .
	ParsSym *p = new ParsSym("PureDecl", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_ExprInitDecl_17(SynSym **base) {
	// ExprInitDecl = TypeName ObjName ASGN Expression .
	ParsSym *p = new ParsSym("ExprInitDecl", 4);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	return p;
}

static
SynSym *p_Expression_18(SynSym **base) {
	// Expression = Bool .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_19(SynSym **base) {
	// Expression = Int .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_20(SynSym **base) {
	// Expression = Num .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_21(SynSym **base) {
	// Expression = Time .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_22(SynSym **base) {
	// Expression = Size .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_23(SynSym **base) {
	// Expression = DQW_STR .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_24(SynSym **base) {
	// Expression = SQW_STR .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_ObjName_25(SynSym **base) {
	// ObjName = ID .
	ParsSym *p = new ParsSym("ObjName", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_26(SynSym **base) {
	// Expression = ID RE_CUSTOM .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Expression_27(SynSym **base) {
	// Expression = ID RE_DEFAULT .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_List_28(SynSym **) {
	// List = .
	ParsSym *p = new ParsSym("List", 0);
	return p;
}

static
SynSym *p_Expression_29(SynSym **base) {
	// Expression = Qualif .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_30(SynSym **base) {
	// Expression = ObjName .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_31(SynSym **base) {
	// Expression = PLUS Expression .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Expression_32(SynSym **base) {
	// Expression = MINUS Expression .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Expression_33(SynSym **base) {
	// Expression = NOT Expression .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Expression_34(SynSym **base) {
	// Expression = Expression DIV Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_35(SynSym **base) {
	// Expression = Expression MOD Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_36(SynSym **base) {
	// Expression = Expression BOOL_AND Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_37(SynSym **base) {
	// Expression = Expression BOOL_OR Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_38(SynSym **base) {
	// Expression = Expression BOOL_XOR Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_39(SynSym **base) {
	// Expression = Expression EQUAL Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_40(SynSym **base) {
	// Expression = Expression NEQUAL Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_41(SynSym **base) {
	// Expression = Expression LT Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_42(SynSym **base) {
	// Expression = Expression LTE Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_43(SynSym **base) {
	// Expression = Expression GT Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_44(SynSym **base) {
	// Expression = Expression GTE Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_45(SynSym **base) {
	// Expression = Expression POWER Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_46(SynSym **base) {
	// Expression = Expression CLONE Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_47(SynSym **base) {
	// Expression = Expression THRU Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Array_48(SynSym **) {
	// Array = .
	ParsSym *p = new ParsSym("Array", 0);
	return p;
}

static
SynSym *p_Array_49(SynSym **base) {
	// Array = Expression .
	ParsSym *p = new ParsSym("Array", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Array_50(SynSym **base) {
	// Array = Expression COLON Expression .
	ParsSym *p = new ParsSym("Array", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_51(SynSym **base) {
	// Expression = Call .
	ParsSym *p = new ParsSym("Expression", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_52(SynSym **base) {
	// Expression = LEFTPARENT Expression RIGHTPARENT .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Bool_53(SynSym **base) {
	// Bool = BOOL_TRUE .
	ParsSym *p = new ParsSym("Bool", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Bool_54(SynSym **base) {
	// Bool = BOOL_FALSE .
	ParsSym *p = new ParsSym("Bool", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Int_55(SynSym **base) {
	// Int = INT .
	ParsSym *p = new ParsSym("Int", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Num_56(SynSym **base) {
	// Num = NUM .
	ParsSym *p = new ParsSym("Num", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Num_57(SynSym **base) {
	// Num = NUM_PERC .
	ParsSym *p = new ParsSym("Num", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Time_58(SynSym **base) {
	// Time = TIME .
	ParsSym *p = new ParsSym("Time", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Size_59(SynSym **base) {
	// Size = SIZE .
	ParsSym *p = new ParsSym("Size", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Qualif_60(SynSym **base) {
	// Qualif = kw_lmt .
	ParsSym *p = new ParsSym("Qualif", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Qualif_61(SynSym **base) {
	// Qualif = kw_now .
	ParsSym *p = new ParsSym("Qualif", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Qualif_62(SynSym **base) {
	// Qualif = kw_nmt .
	ParsSym *p = new ParsSym("Qualif", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_63(SynSym **base) {
	// Expression = LEFTBRACKET Array RIGHTBRACKET .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Array_64(SynSym **base) {
	// Array = Array COMMA Expression .
	ParsSym *p = new ParsSym("Array", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Array_65(SynSym **base) {
	// Array = Array COMMA Expression COLON Expression .
	ParsSym *p = new ParsSym("Array", 5);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	return p;
}

static
SynSym *p_Expression_66(SynSym **base) {
	// Expression = Expression MUL Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_67(SynSym **base) {
	// Expression = Expression MINUS Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_List_68(SynSym **base) {
	// List = Expression .
	ParsSym *p = new ParsSym("List", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_List_69(SynSym **base) {
	// List = List COMMA Expression .
	ParsSym *p = new ParsSym("List", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Call_70(SynSym **base) {
	// Call = ID LEFTPARENT List RIGHTPARENT .
	ParsSym *p = new ParsSym("Call", 4);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	return p;
}

static
SynSym *p_ObjName_71(SynSym **base) {
	// ObjName = ID PERIOD ObjName .
	ParsSym *p = new ParsSym("ObjName", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_72(SynSym **base) {
	// Expression = Expression PLUS Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_CodeInitDecl_73(SynSym **base) {
	// CodeInitDecl = TypeName ObjName ASGN LEFTBRACE Code RIGHTBRACE .
	ParsSym *p = new ParsSym("CodeInitDecl", 6);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	p->append(*base[6]);
	return p;
}

static
SynSym *p_Assignment_74(SynSym **base) {
	// Assignment = ObjName ASGN Expression .
	ParsSym *p = new ParsSym("Assignment", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Assignment_75(SynSym **base) {
	// Assignment = ObjName ASGN LEFTBRACE Code RIGHTBRACE .
	ParsSym *p = new ParsSym("Assignment", 5);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	return p;
}

static
SynSym *p_Assignment_76(SynSym **base) {
	// Assignment = LEFTBRACKET Array RIGHTBRACKET ASGN Expression .
	ParsSym *p = new ParsSym("Assignment", 5);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	return p;
}

static
SynSym *p_TypeName_77(SynSym **base) {
	// TypeName = ID .
	ParsSym *p = new ParsSym("TypeName", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_TypeName_78(SynSym **base) {
	// TypeName = ID LEFTBRACKET RIGHTBRACKET .
	ParsSym *p = new ParsSym("TypeName", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_IfCode_79(SynSym **base) {
	// IfCode = kw_if Expression kw_then LEFTBRACE Code RIGHTBRACE .
	ParsSym *p = new ParsSym("IfCode", 6);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	p->append(*base[6]);
	return p;
}

static
SynSym *p_IfCode_80(SynSym **base) {
	// IfCode = kw_if Expression kw_then LEFTBRACE Code RIGHTBRACE kw_else LEFTBRACE Code RIGHTBRACE .
	ParsSym *p = new ParsSym("IfCode", 10);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	p->append(*base[6]);
	p->append(*base[7]);
	p->append(*base[8]);
	p->append(*base[9]);
	p->append(*base[10]);
	return p;
}

static
SynSym *p_EveryCode_81(SynSym **base) {
	// EveryCode = kw_every Expression kw_do LEFTBRACE Code RIGHTBRACE .
	ParsSym *p = new ParsSym("EveryCode", 6);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	p->append(*base[5]);
	p->append(*base[6]);
	return p;
}

#line 17 "gparserc.tpl"


PglParser::PglParser(Lexer *l) : GParser(l) {
	// static int _condTable[] = {};

	acceptState = 3;
	// condTable = _condTable;
	maxState = 143;
}

PglParser::~PglParser() {
}

int PglParser::actionTable(int state, int sym) {
	static int _actionTable[143][81] = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -1, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, 0, 0, -2, 0, 0, 0, 0, 0, 
0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 2, 4, 5, 7, 9, 10, 14, 15, 16, 17, 18, 
19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -3, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -1, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 0, 6, 5, 7, 9, 10, 14, 15, 16, 17, 
18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -4, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -5, -5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -5, -5, -5, 0, -5, 0, 0, 0, 0, 0, 
0, 0, 0, -5, 0, 0, -5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -6, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -6, -6, -6, 0, -6, 0, 0, 0, 0, 0, 
0, 0, 0, -6, 0, 0, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -7, -7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -7, -7, -7, 0, -7, 0, 0, 0, 0, 0, 
0, 0, 0, -7, 0, 0, -7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 12, 4, 5, 7, 9, 10, 14, 15, 16, 17, 
18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -8, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -8, -8, -8, 0, -8, 0, 0, 0, 0, 0, 
0, 0, 0, -8, 0, 0, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -9, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -10, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -11, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -12, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -13, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -14, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -15, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, -16, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 110, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 24, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, 0, 0, 0, 0, 0, -17, 0, 0, 
0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 109, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -18, -18, -18, -18, 
-18, -18, -18, -18, -18, 0, -18, -18, -18, -18, -18, -18, 0, 0, -18, -18, 
0, 0, 0, 0, -18, -18, -18, 0, -18, -18, 0, 0, 0, 0, -18, 0, 0, -18, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -19, -19, -19, -19, 
-19, -19, -19, -19, -19, 0, -19, -19, -19, -19, -19, -19, 0, 0, -19, -19, 
0, 0, 0, 0, -19, -19, -19, 0, -19, -19, 0, 0, 0, 0, -19, 0, 0, -19, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -20, -20, -20, -20, 
-20, -20, -20, -20, -20, 0, -20, -20, -20, -20, -20, -20, 0, 0, -20, -20, 
0, 0, 0, 0, -20, -20, -20, 0, -20, -20, 0, 0, 0, 0, -20, 0, 0, -20, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -21, -21, -21, -21, 
-21, -21, -21, -21, -21, 0, -21, -21, -21, -21, -21, -21, 0, 0, -21, -21, 
0, 0, 0, 0, -21, -21, -21, 0, -21, -21, 0, 0, 0, 0, -21, 0, 0, -21, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -22, -22, -22, -22, 
-22, -22, -22, -22, -22, 0, -22, -22, -22, -22, -22, -22, 0, 0, -22, -22, 
0, 0, 0, 0, -22, -22, -22, 0, -22, -22, 0, 0, 0, 0, -22, 0, 0, -22, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -23, -23, -23, -23, 
-23, -23, -23, -23, -23, 0, -23, -23, -23, -23, -23, -23, 0, 0, -23, -23, 
0, 0, 0, 0, -23, -23, -23, 0, -23, -23, 0, 0, 0, 0, -23, 0, 0, -23, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -24, -24, -24, -24, 
-24, -24, -24, -24, -24, 0, -24, -24, -24, -24, -24, -24, 0, 0, -24, -24, 
0, 0, 0, 0, -24, -24, -24, 0, -24, -24, 0, 0, 0, 0, -24, 0, 0, -24, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -25, -25, -25, -25, 
-25, -25, -25, -25, -25, 0, -25, -25, -25, -25, -25, -25, 0, 36, -25, -25, 
0, 0, 0, 106, -25, -25, -25, 0, -25, -25, 0, 0, 0, 0, -25, 0, 0, -25, 
34, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -26, -26, -26, -26, 
-26, -26, -26, -26, -26, 0, -26, -26, -26, -26, -26, -26, 0, 0, -26, -26, 
0, 0, 0, 0, -26, -26, -26, 0, -26, -26, 0, 0, 0, 0, -26, 0, 0, -26, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -27, -27, -27, -27, 
-27, -27, -27, -27, -27, 0, -27, -27, -27, -27, -27, -27, 0, 0, -27, -27, 
0, 0, 0, 0, -27, -27, -27, 0, -27, -27, 0, 0, 0, 0, -27, 0, 0, -27, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, -28, 0, 75, 0, 0, 0, 0, 0, 
-28, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 101, 0, 26, 27, 28, 29, 30, 37, 102, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -29, -29, -29, -29, 
-29, -29, -29, -29, -29, 0, -29, -29, -29, -29, -29, -29, 0, 0, -29, -29, 
0, 0, 0, 0, -29, -29, -29, 0, -29, -29, 0, 0, 0, 0, -29, 0, 0, -29, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -30, -30, -30, -30, 
-30, -30, -30, -30, -30, 0, -30, -30, -30, -30, -30, -30, 0, 0, -30, -30, 
0, 0, 0, 0, -30, -30, -30, 0, -30, -30, 0, 0, 0, 0, -30, 0, 0, -30, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 40, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -31, -31, -31, -31, 
-31, -31, -31, -31, -31, 0, -31, -31, 44, 47, 49, 69, 0, 0, -31, -31, 0, 
0, 0, 0, -31, -31, -31, 0, -31, -31, 0, 0, 0, 0, -31, 0, 0, -31, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 100, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 43, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -32, -32, -32, -32, 
-32, -32, -32, -32, -32, 0, -32, -32, 44, 47, 49, 69, 0, 0, -32, -32, 0, 
0, 0, 0, -32, -32, -32, 0, -32, -32, 0, 0, 0, 0, -32, 0, 0, -32, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 99, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 46, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
-33, -33, -33, 0, 25, 41, 44, 47, 49, 69, 0, 0, -33, -33, 0, 0, 0, 0, -33, 
-33, -33, 0, -33, -33, 0, 0, 0, 0, -33, 0, 0, -33, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 48, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -34, -34, -34, -34, 
-34, -34, -34, -34, -34, 0, -34, -34, -34, -34, -34, -34, 0, 0, -34, -34, 
0, 0, 0, 0, -34, -34, -34, 0, -34, -34, 0, 0, 0, 0, -34, 0, 0, -34, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 50, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -35, -35, -35, -35, 
-35, -35, -35, -35, -35, 0, -35, -35, -35, -35, -35, -35, 0, 0, -35, -35, 
0, 0, 0, 0, -35, -35, -35, 0, -35, -35, 0, 0, 0, 0, -35, 0, 0, -35, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 52, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
-36, -36, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, -36, -36, 0, 0, 0, 0, -36, 
-36, -36, 0, -36, -36, 0, 0, 0, 0, -36, 0, 0, -36, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 54, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, -37, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, -37, -37, 0, 0, 0, 0, -37, 
-37, -37, 0, -37, -37, 0, 0, 0, 0, -37, 0, 0, -37, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 56, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
-38, -38, -38, 0, 25, 41, 44, 47, 49, 69, 0, 0, -38, -38, 0, 0, 0, 0, -38, 
-38, -38, 0, -38, -38, 0, 0, 0, 0, -38, 0, 0, -38, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 58, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 63, 65, 67, 
-39, -39, -39, 0, 25, 41, 44, 47, 49, 69, 0, 0, -39, -39, 0, 0, 0, 0, -39, 
-39, -39, 0, -39, -39, 0, 0, 0, 0, -39, 0, 0, -39, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 60, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 61, 63, 65, 67, 
-40, -40, -40, 0, 25, 41, 44, 47, 49, 69, 0, 0, -40, -40, 0, 0, 0, 0, -40, 
-40, -40, 0, -40, -40, 0, 0, 0, 0, -40, 0, 0, -40, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 62, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -41, -41, 0, 0, 0, 0, 
-41, -41, -41, 0, 25, 41, 44, 47, 49, 69, 0, 0, -41, -41, 0, 0, 0, 0, -41, 
-41, -41, 0, -41, -41, 0, 0, 0, 0, -41, 0, 0, -41, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 64, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -42, -42, 0, 0, 0, 0, 
-42, -42, -42, 0, 25, 41, 44, 47, 49, 69, 0, 0, -42, -42, 0, 0, 0, 0, -42, 
-42, -42, 0, -42, -42, 0, 0, 0, 0, -42, 0, 0, -42, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 66, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -43, -43, 0, 0, 0, 0, 
-43, -43, -43, 0, 25, 41, 44, 47, 49, 69, 0, 0, -43, -43, 0, 0, 0, 0, -43, 
-43, -43, 0, -43, -43, 0, 0, 0, 0, -43, 0, 0, -43, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 68, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -44, -44, 0, 0, 0, 0, 
-44, -44, -44, 0, 25, 41, 44, 47, 49, 69, 0, 0, -44, -44, 0, 0, 0, 0, -44, 
-44, -44, 0, -44, -44, 0, 0, 0, 0, -44, 0, 0, -44, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 70, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -45, -45, -45, -45, 
-45, -45, -45, -45, -45, 0, -45, -45, -45, -45, -45, -45, 0, 0, -45, -45, 
0, 0, 0, 0, -45, -45, -45, 0, -45, -45, 0, 0, 0, 0, -45, 0, 0, -45, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 72, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, -46, -46, 0, 0, 0, 0, -46, 
-46, -46, 0, 0, 0, 0, 0, 0, 0, -46, 0, 0, -46, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 74, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, -47, -47, 0, 0, 0, 0, -47, 
-47, -47, 0, 0, 0, 0, 0, 0, 0, -47, 0, 0, -47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, -48, 75, 0, 0, 0, 0, 0, 
-48, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 76, 93, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, -49, 0, 0, 0, 0, 0, 77, 
-49, 0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 78, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, -50, 0, 0, 0, 0, 0, 0, 
-50, 0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -51, -51, -51, -51, 
-51, -51, -51, -51, -51, 0, -51, -51, -51, -51, -51, -51, 0, 0, -51, -51, 
0, 0, 0, 0, -51, -51, -51, 0, -51, -51, 0, 0, 0, 0, -51, 0, 0, -51, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 81, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 82, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -52, -52, -52, -52, 
-52, -52, -52, -52, -52, 0, -52, -52, -52, -52, -52, -52, 0, 0, -52, -52, 
0, 0, 0, 0, -52, -52, -52, 0, -52, -52, 0, 0, 0, 0, -52, 0, 0, -52, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -53, -53, -53, -53, 
-53, -53, -53, -53, -53, 0, -53, -53, -53, -53, -53, -53, 0, 0, -53, -53, 
0, 0, 0, 0, -53, -53, -53, 0, -53, -53, 0, 0, 0, 0, -53, 0, 0, -53, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -54, -54, -54, -54, 
-54, -54, -54, -54, -54, 0, -54, -54, -54, -54, -54, -54, 0, 0, -54, -54, 
0, 0, 0, 0, -54, -54, -54, 0, -54, -54, 0, 0, 0, 0, -54, 0, 0, -54, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -55, -55, -55, -55, 
-55, -55, -55, -55, -55, 0, -55, -55, -55, -55, -55, -55, 0, 0, -55, -55, 
0, 0, 0, 0, -55, -55, -55, 0, -55, -55, 0, 0, 0, 0, -55, 0, 0, -55, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -56, -56, -56, -56, 
-56, -56, -56, -56, -56, 0, -56, -56, -56, -56, -56, -56, 0, 0, -56, -56, 
0, 0, 0, 0, -56, -56, -56, 0, -56, -56, 0, 0, 0, 0, -56, 0, 0, -56, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -57, -57, -57, -57, 
-57, -57, -57, -57, -57, 0, -57, -57, -57, -57, -57, -57, 0, 0, -57, -57, 
0, 0, 0, 0, -57, -57, -57, 0, -57, -57, 0, 0, 0, 0, -57, 0, 0, -57, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -58, -58, -58, -58, 
-58, -58, -58, -58, -58, 0, -58, -58, -58, -58, -58, -58, 0, 0, -58, -58, 
0, 0, 0, 0, -58, -58, -58, 0, -58, -58, 0, 0, 0, 0, -58, 0, 0, -58, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -59, -59, -59, -59, 
-59, -59, -59, -59, -59, 0, -59, -59, -59, -59, -59, -59, 0, 0, -59, -59, 
0, 0, 0, 0, -59, -59, -59, 0, -59, -59, 0, 0, 0, 0, -59, 0, 0, -59, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -60, -60, -60, -60, 
-60, -60, -60, -60, -60, 0, -60, -60, -60, -60, -60, -60, 0, 0, -60, -60, 
0, 0, 0, 0, -60, -60, -60, 0, -60, -60, 0, 0, 0, 0, -60, 0, 0, -60, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -61, -61, -61, -61, 
-61, -61, -61, -61, -61, 0, -61, -61, -61, -61, -61, -61, 0, 0, -61, -61, 
0, 0, 0, 0, -61, -61, -61, 0, -61, -61, 0, 0, 0, 0, -61, 0, 0, -61, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -62, -62, -62, -62, 
-62, -62, -62, -62, -62, 0, -62, -62, -62, -62, -62, -62, 0, 0, -62, -62, 
0, 0, 0, 0, -62, -62, -62, 0, -62, -62, 0, 0, 0, 0, -62, 0, 0, -62, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0, 0, 0, 0, 0, 95, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -63, -63, -63, -63, 
-63, -63, -63, -63, -63, 0, -63, -63, -63, -63, -63, -63, 0, 0, -63, -63, 
0, 0, 0, 0, -63, -63, -63, 0, -63, -63, 0, 0, 0, 0, -63, 0, 0, -63, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 96, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, -64, 0, 0, 0, 0, 0, 97, 
-64, 0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 98, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, -65, 0, 0, 0, 0, 0, 0, 
-65, 0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -66, -66, -66, -66, 
-66, -66, -66, -66, -66, 0, -66, -66, -66, -66, -66, -66, 0, 0, -66, -66, 
0, 0, 0, 0, -66, -66, -66, 0, -66, -66, 0, 0, 0, 0, -66, 0, 0, -66, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -67, -67, -67, -67, 
-67, -67, -67, -67, -67, 0, -67, -67, 44, 47, 49, 69, 0, 0, -67, -67, 0, 
0, 0, 0, -67, -67, -67, 0, -67, -67, 0, 0, 0, 0, -67, 0, 0, -67, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, -68, 0, 0, 0, 0, 0, 0, 0, 
-68, 0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 105, 0, 0, 0, 0, 0, 0, 0, 103, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 104, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, -69, 0, 0, 0, 0, 0, 0, 0, 
-69, 0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -70, -70, -70, -70, 
-70, -70, -70, -70, -70, 0, -70, -70, -70, -70, -70, -70, 0, 0, -70, -70, 
0, 0, 0, 0, -70, -70, -70, 0, -70, -70, 0, 0, 0, 0, -70, 0, 0, -70, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
108, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -25, -25, -25, -25, 
-25, -25, -25, -25, -25, 0, -25, -25, -25, -25, -25, -25, -25, 0, -25, 
-25, 0, 0, 0, 106, -25, -25, -25, 0, -25, -25, 0, 0, 0, 0, -25, 0, 0, -25, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -71, -71, -71, -71, 
-71, -71, -71, -71, -71, 0, -71, -71, -71, -71, -71, -71, -71, 0, -71, 
-71, 0, 0, 0, 0, -71, -71, -71, 0, -71, -71, 0, 0, 0, 0, -71, 0, 0, -71, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -72, -72, -72, -72, 
-72, -72, -72, -72, -72, 0, -72, -72, 44, 47, 49, 69, 0, 0, -72, -72, 0, 
0, 0, 0, -72, -72, -72, 0, -72, -72, 0, 0, 0, 0, -72, 0, 0, -72, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 111, 4, 5, 7, 9, 10, 14, 15, 16, 
17, 18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 112, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -73, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 114, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 116, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 115, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, 0, 0, 0, 0, 0, -74, 0, 0, 
0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 117, 4, 5, 7, 9, 10, 14, 15, 16, 
17, 18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 118, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -75, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, -48, 75, 0, 0, 0, 0, 0, 
-48, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 76, 120, 26, 27, 28, 29, 30, 37, 0, 0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 121, 0, 0, 0, 0, 0, 0, 95, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 122, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 123, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, 0, 0, 0, 0, 0, -76, 0, 0, 
0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, -77, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, -25, 36, 0, 0, 125, 0, 0, 106, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 126, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, -78, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 128, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 73, 71, 0, 0, 0, 0, 129, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 130, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 131, 4, 5, 7, 9, 10, 14, 15, 16, 
17, 18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 132, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -79, -79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -79, -79, -79, 0, -79, 0, 0, 
0, 0, 0, 0, 0, 0, -79, 0, 133, -79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 134, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 135, 4, 5, 7, 9, 10, 14, 15, 16, 
17, 18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -80, -80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -80, -80, -80, 0, -80, 0, 0, 
0, 0, 0, 0, 0, 0, -80, 0, 0, -80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 85, 86, 88, 89, 83, 84, 32, 31, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 45, 39, 42, 0, 0, 0, 0, 0, 80, 0, 0, 75, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 90, 91, 92, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 79, 0, 38, 138, 0, 26, 27, 28, 29, 30, 37, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 57, 59, 61, 63, 65, 67, 
51, 53, 55, 0, 25, 41, 44, 47, 49, 69, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 73, 71, 0, 0, 0, 0, 0, 0, 0, 139, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 140, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 119, 11, -1, 0, -2, 0, 0, 0, 0, 
0, 0, 0, 0, 127, 0, 0, 137, 0, 0, 0, 141, 4, 5, 7, 9, 10, 14, 15, 16, 
17, 18, 19, 20, 21, 113, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -81, -81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -81, -81, -81, 0, -81, 0, 0, 
0, 0, 0, 0, 0, 0, -81, 0, 0, -81, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
	return _actionTable[state][sym];
}

ReduceInfo *PglParser::reduceTable(int i) {
	static ReduceInfo _reduceTable[] = {
{ 0, 0, 0, 0},
{ 57, 0, 0, p_StatementSeq_1, 0 },
{ 59, 0, 0, p_SimpleStatement_2, 0 },
{ 56, 1, 1, p_Code_3, 0 },
{ 57, 2, 2, p_StatementSeq_4, 0 },
{ 58, 2, 2, p_Statement_5, 0 },
{ 58, 1, 1, p_Statement_6, 0 },
{ 58, 1, 1, p_Statement_7, 0 },
{ 58, 3, 3, p_Statement_8, 0 },
{ 59, 1, 1, p_SimpleStatement_9, 0 },
{ 59, 1, 1, p_SimpleStatement_10, 0 },
{ 63, 1, 1, p_DeclStatement_11, 0 },
{ 63, 1, 1, p_DeclStatement_12, 0 },
{ 63, 1, 1, p_DeclStatement_13, 0 },
{ 62, 1, 1, p_ExprStatement_14, 0 },
{ 62, 1, 1, p_ExprStatement_15, 0 },
{ 64, 2, 2, p_PureDecl_16, 0 },
{ 65, 4, 4, p_ExprInitDecl_17, 0 },
{ 71, 1, 1, p_Expression_18, 0 },
{ 71, 1, 1, p_Expression_19, 0 },
{ 71, 1, 1, p_Expression_20, 0 },
{ 71, 1, 1, p_Expression_21, 0 },
{ 71, 1, 1, p_Expression_22, 0 },
{ 71, 1, 1, p_Expression_23, 0 },
{ 71, 1, 1, p_Expression_24, 0 },
{ 70, 1, 1, p_ObjName_25, 0 },
{ 71, 2, 2, p_Expression_26, 0 },
{ 71, 2, 2, p_Expression_27, 0 },
{ 79, 0, 0, p_List_28, 0 },
{ 71, 1, 1, p_Expression_29, 0 },
{ 71, 1, 1, p_Expression_30, 0 },
{ 71, 2, 2, p_Expression_31, 0 },
{ 71, 2, 2, p_Expression_32, 0 },
{ 71, 2, 2, p_Expression_33, 0 },
{ 71, 3, 3, p_Expression_34, 0 },
{ 71, 3, 3, p_Expression_35, 0 },
{ 71, 3, 3, p_Expression_36, 0 },
{ 71, 3, 3, p_Expression_37, 0 },
{ 71, 3, 3, p_Expression_38, 0 },
{ 71, 3, 3, p_Expression_39, 0 },
{ 71, 3, 3, p_Expression_40, 0 },
{ 71, 3, 3, p_Expression_41, 0 },
{ 71, 3, 3, p_Expression_42, 0 },
{ 71, 3, 3, p_Expression_43, 0 },
{ 71, 3, 3, p_Expression_44, 0 },
{ 71, 3, 3, p_Expression_45, 0 },
{ 71, 3, 3, p_Expression_46, 0 },
{ 71, 3, 3, p_Expression_47, 0 },
{ 72, 0, 0, p_Array_48, 0 },
{ 72, 1, 1, p_Array_49, 0 },
{ 72, 3, 3, p_Array_50, 0 },
{ 71, 1, 1, p_Expression_51, 0 },
{ 71, 3, 3, p_Expression_52, 0 },
{ 73, 1, 1, p_Bool_53, 0 },
{ 73, 1, 1, p_Bool_54, 0 },
{ 74, 1, 1, p_Int_55, 0 },
{ 75, 1, 1, p_Num_56, 0 },
{ 75, 1, 1, p_Num_57, 0 },
{ 76, 1, 1, p_Time_58, 0 },
{ 77, 1, 1, p_Size_59, 0 },
{ 78, 1, 1, p_Qualif_60, 0 },
{ 78, 1, 1, p_Qualif_61, 0 },
{ 78, 1, 1, p_Qualif_62, 0 },
{ 71, 3, 3, p_Expression_63, 0 },
{ 72, 3, 3, p_Array_64, 0 },
{ 72, 5, 5, p_Array_65, 0 },
{ 71, 3, 3, p_Expression_66, 0 },
{ 71, 3, 3, p_Expression_67, 0 },
{ 79, 1, 1, p_List_68, 0 },
{ 79, 3, 3, p_List_69, 0 },
{ 68, 4, 4, p_Call_70, 0 },
{ 70, 3, 3, p_ObjName_71, 0 },
{ 71, 3, 3, p_Expression_72, 0 },
{ 66, 6, 6, p_CodeInitDecl_73, 0 },
{ 67, 3, 3, p_Assignment_74, 0 },
{ 67, 5, 5, p_Assignment_75, 0 },
{ 67, 5, 5, p_Assignment_76, 0 },
{ 69, 1, 1, p_TypeName_77, 0 },
{ 69, 3, 3, p_TypeName_78, 0 },
{ 60, 6, 6, p_IfCode_79, 0 },
{ 60, 10, 10, p_IfCode_80, 0 },
{ 61, 6, 6, p_EveryCode_81, 0 }};
	return(&_reduceTable[i]);
}


#line 113 "pgl.syn"

// postlude


