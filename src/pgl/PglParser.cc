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
SynSym *p_Name_26(SynSym **base) {
	// Name = ID .
	ParsSym *p = new ParsSym("Name", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Expression_27(SynSym **base) {
	// Expression = ID RE_CUSTOM .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
	return p;
}

static
SynSym *p_Expression_28(SynSym **base) {
	// Expression = ID RE_DEFAULT .
	ParsSym *p = new ParsSym("Expression", 2);
	p->append(*base[1]);
	p->append(*base[2]);
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
SynSym *p_Name_54(SynSym **base) {
	// Name = BOOL_TRUE .
	ParsSym *p = new ParsSym("Name", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Bool_55(SynSym **base) {
	// Bool = BOOL_FALSE .
	ParsSym *p = new ParsSym("Bool", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Name_56(SynSym **base) {
	// Name = BOOL_FALSE .
	ParsSym *p = new ParsSym("Name", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Int_57(SynSym **base) {
	// Int = INT .
	ParsSym *p = new ParsSym("Int", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Num_58(SynSym **base) {
	// Num = NUM .
	ParsSym *p = new ParsSym("Num", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Num_59(SynSym **base) {
	// Num = NUM_PERC .
	ParsSym *p = new ParsSym("Num", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Time_60(SynSym **base) {
	// Time = TIME_SCALE .
	ParsSym *p = new ParsSym("Time", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Name_61(SynSym **base) {
	// Name = TIME_SCALE .
	ParsSym *p = new ParsSym("Name", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Time_62(SynSym **base) {
	// Time = TIME .
	ParsSym *p = new ParsSym("Time", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Size_63(SynSym **base) {
	// Size = SIZE_SCALE .
	ParsSym *p = new ParsSym("Size", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Name_64(SynSym **base) {
	// Name = SIZE_SCALE .
	ParsSym *p = new ParsSym("Name", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Size_65(SynSym **base) {
	// Size = SIZE .
	ParsSym *p = new ParsSym("Size", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Qualif_66(SynSym **base) {
	// Qualif = kw_lmt .
	ParsSym *p = new ParsSym("Qualif", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Qualif_67(SynSym **base) {
	// Qualif = kw_now .
	ParsSym *p = new ParsSym("Qualif", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_Qualif_68(SynSym **base) {
	// Qualif = kw_nmt .
	ParsSym *p = new ParsSym("Qualif", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_List_69(SynSym **) {
	// List = .
	ParsSym *p = new ParsSym("List", 0);
	return p;
}

static
SynSym *p_List_70(SynSym **base) {
	// List = Expression .
	ParsSym *p = new ParsSym("List", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_List_71(SynSym **base) {
	// List = List COMMA Expression .
	ParsSym *p = new ParsSym("List", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Call_72(SynSym **base) {
	// Call = Name LEFTPARENT List RIGHTPARENT .
	ParsSym *p = new ParsSym("Call", 4);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	p->append(*base[4]);
	return p;
}

static
SynSym *p_ObjNameTail_73(SynSym **base) {
	// ObjNameTail = Name .
	ParsSym *p = new ParsSym("ObjNameTail", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_ObjNameTail_74(SynSym **base) {
	// ObjNameTail = Name PERIOD ObjNameTail .
	ParsSym *p = new ParsSym("ObjNameTail", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_ObjName_75(SynSym **base) {
	// ObjName = Name PERIOD ObjNameTail .
	ParsSym *p = new ParsSym("ObjName", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_76(SynSym **base) {
	// Expression = LEFTBRACKET Array RIGHTBRACKET .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Array_77(SynSym **base) {
	// Array = Array COMMA Expression .
	ParsSym *p = new ParsSym("Array", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Array_78(SynSym **base) {
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
SynSym *p_Expression_79(SynSym **base) {
	// Expression = Expression MUL Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_80(SynSym **base) {
	// Expression = Expression MINUS Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Expression_81(SynSym **base) {
	// Expression = Expression PLUS Expression .
	ParsSym *p = new ParsSym("Expression", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_CodeInitDecl_82(SynSym **base) {
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
SynSym *p_Assignment_83(SynSym **base) {
	// Assignment = ObjName ASGN Expression .
	ParsSym *p = new ParsSym("Assignment", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_Assignment_84(SynSym **base) {
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
SynSym *p_Assignment_85(SynSym **base) {
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
SynSym *p_TypeName_86(SynSym **base) {
	// TypeName = ID .
	ParsSym *p = new ParsSym("TypeName", 1);
	p->append(*base[1]);
	return p;
}

static
SynSym *p_TypeName_87(SynSym **base) {
	// TypeName = ID LEFTBRACKET RIGHTBRACKET .
	ParsSym *p = new ParsSym("TypeName", 3);
	p->append(*base[1]);
	p->append(*base[2]);
	p->append(*base[3]);
	return p;
}

static
SynSym *p_IfCode_88(SynSym **base) {
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
SynSym *p_IfCode_89(SynSym **base) {
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
SynSym *p_EveryCode_90(SynSym **base) {
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
	maxState = 155;
}

PglParser::~PglParser() {
}

int PglParser::actionTable(int state, int sym) {
	static int _actionTable[155][85] = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -1, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, 0, 0, -2, 
0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 2, 4, 5, 7, 9, 10, 14, 
15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -3, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -1, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 0, 6, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, -4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -4, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -5, -5, 0, 0, -5, 0, -5, 0, -5, -5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -5, -5, -5, 0, -5, 0, 0, 
0, 0, 0, 0, 0, 0, -5, 0, 0, -5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -6, -6, 0, 0, -6, 0, -6, 0, -6, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -6, -6, -6, 0, -6, 0, 0, 
0, 0, 0, 0, 0, 0, -6, 0, 0, -6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -7, -7, 0, 0, -7, 0, -7, 0, -7, -7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -7, -7, -7, 0, -7, 0, 0, 
0, 0, 0, 0, 0, 0, -7, 0, 0, -7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 12, 4, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -8, -8, 0, 0, -8, 0, -8, 0, -8, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -8, -8, -8, 0, -8, 0, 0, 
0, 0, 0, 0, 0, 0, -8, 0, 0, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -9, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -10, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -11, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -12, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -13, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -14, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -15, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 153, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 154, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 0, 0, 0, 0, 0, 0, 0, -16, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 120, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 24, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, 0, 0, 0, 0, 0, 
-17, 0, 0, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 119, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -18, -18, -18, 
-18, -18, -18, -18, -18, -18, 0, -18, -18, -18, -18, -18, -18, 0, 0, -18, 
-18, 0, 0, 0, 0, -18, -18, -18, 0, -18, -18, 0, 0, 0, 0, -18, 0, 0, -18, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -19, -19, -19, 
-19, -19, -19, -19, -19, -19, 0, -19, -19, -19, -19, -19, -19, 0, 0, -19, 
-19, 0, 0, 0, 0, -19, -19, -19, 0, -19, -19, 0, 0, 0, 0, -19, 0, 0, -19, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -20, -20, -20, 
-20, -20, -20, -20, -20, -20, 0, -20, -20, -20, -20, -20, -20, 0, 0, -20, 
-20, 0, 0, 0, 0, -20, -20, -20, 0, -20, -20, 0, 0, 0, 0, -20, 0, 0, -20, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -21, -21, -21, 
-21, -21, -21, -21, -21, -21, 0, -21, -21, -21, -21, -21, -21, 0, 0, -21, 
-21, 0, 0, 0, 0, -21, -21, -21, 0, -21, -21, 0, 0, 0, 0, -21, 0, 0, -21, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -22, -22, -22, 
-22, -22, -22, -22, -22, -22, 0, -22, -22, -22, -22, -22, -22, 0, 0, -22, 
-22, 0, 0, 0, 0, -22, -22, -22, 0, -22, -22, 0, 0, 0, 0, -22, 0, 0, -22, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -23, -23, -23, 
-23, -23, -23, -23, -23, -23, 0, -23, -23, -23, -23, -23, -23, 0, 0, -23, 
-23, 0, 0, 0, 0, -23, -23, -23, 0, -23, -23, 0, 0, 0, 0, -23, 0, 0, -23, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -24, -24, -24, 
-24, -24, -24, -24, -24, -24, 0, -24, -24, -24, -24, -24, -24, 0, 0, -24, 
-24, 0, 0, 0, 0, -24, -24, -24, 0, -24, -24, 0, 0, 0, 0, -24, 0, 0, -24, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -25, -25, -25, 
-25, -25, -25, -25, -25, -25, 0, -25, -25, -25, -25, -25, -25, 0, -26, 
-25, -25, 0, 0, 0, -26, -25, -25, -25, 0, -25, -25, 0, 0, 0, 0, -25, 0, 0, 
-25, 34, 35, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -27, -27, -27, 
-27, -27, -27, -27, -27, -27, 0, -27, -27, -27, -27, -27, -27, 0, 0, -27, 
-27, 0, 0, 0, 0, -27, -27, -27, 0, -27, -27, 0, 0, 0, 0, -27, 0, 0, -27, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -28, -28, -28, 
-28, -28, -28, -28, -28, -28, 0, -28, -28, -28, -28, -28, -28, 0, 0, -28, 
-28, 0, 0, 0, 0, -28, -28, -28, 0, -28, -28, 0, 0, 0, 0, -28, 0, 0, -28, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -29, -29, -29, 
-29, -29, -29, -29, -29, -29, 0, -29, -29, -29, -29, -29, -29, 0, 0, -29, 
-29, 0, 0, 0, 0, -29, -29, -29, 0, -29, -29, 0, 0, 0, 0, -29, 0, 0, -29, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -30, -30, -30, 
-30, -30, -30, -30, -30, -30, 0, -30, -30, -30, -30, -30, -30, 0, 0, -30, 
-30, 0, 0, 0, 0, -30, -30, -30, 0, -30, -30, 0, 0, 0, 0, -30, 0, 0, -30, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 39, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -31, -31, -31, 
-31, -31, -31, -31, -31, -31, 0, -31, -31, 43, 46, 48, 68, 0, 0, -31, -31, 
0, 0, 0, 0, -31, -31, -31, 0, -31, -31, 0, 0, 0, 0, -31, 0, 0, -31, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 118, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 42, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -32, -32, -32, 
-32, -32, -32, -32, -32, -32, 0, -32, -32, 43, 46, 48, 68, 0, 0, -32, -32, 
0, 0, 0, 0, -32, -32, -32, 0, -32, -32, 0, 0, 0, 0, -32, 0, 0, -32, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 117, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 45, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, -33, -33, -33, 0, 25, 40, 43, 46, 48, 68, 0, 0, -33, -33, 0, 0, 
0, 0, -33, -33, -33, 0, -33, -33, 0, 0, 0, 0, -33, 0, 0, -33, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 47, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -34, -34, -34, 
-34, -34, -34, -34, -34, -34, 0, -34, -34, -34, -34, -34, -34, 0, 0, -34, 
-34, 0, 0, 0, 0, -34, -34, -34, 0, -34, -34, 0, 0, 0, 0, -34, 0, 0, -34, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 

{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 49, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -35, -35, -35, 
-35, -35, -35, -35, -35, -35, 0, -35, -35, -35, -35, -35, -35, 0, 0, -35, 
-35, 0, 0, 0, 0, -35, -35, -35, 0, -35, -35, 0, 0, 0, 0, -35, 0, 0, -35, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 51, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, -36, -36, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, -36, -36, 0, 0, 0, 
0, -36, -36, -36, 0, -36, -36, 0, 0, 0, 0, -36, 0, 0, -36, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 53, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, -37, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, -37, -37, 0, 0, 0, 
0, -37, -37, -37, 0, -37, -37, 0, 0, 0, 0, -37, 0, 0, -37, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 55, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, -38, -38, -38, 0, 25, 40, 43, 46, 48, 68, 0, 0, -38, -38, 0, 0, 
0, 0, -38, -38, -38, 0, -38, -38, 0, 0, 0, 0, -38, 0, 0, -38, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 57, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 62, 64, 
66, -39, -39, -39, 0, 25, 40, 43, 46, 48, 68, 0, 0, -39, -39, 0, 0, 0, 0, 
-39, -39, -39, 0, -39, -39, 0, 0, 0, 0, -39, 0, 0, -39, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 59, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 60, 62, 64, 
66, -40, -40, -40, 0, 25, 40, 43, 46, 48, 68, 0, 0, -40, -40, 0, 0, 0, 0, 
-40, -40, -40, 0, -40, -40, 0, 0, 0, 0, -40, 0, 0, -40, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 61, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -41, -41, 0, 0, 
0, 0, -41, -41, -41, 0, 25, 40, 43, 46, 48, 68, 0, 0, -41, -41, 0, 0, 0, 
0, -41, -41, -41, 0, -41, -41, 0, 0, 0, 0, -41, 0, 0, -41, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 63, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -42, -42, 0, 0, 
0, 0, -42, -42, -42, 0, 25, 40, 43, 46, 48, 68, 0, 0, -42, -42, 0, 0, 0, 
0, -42, -42, -42, 0, -42, -42, 0, 0, 0, 0, -42, 0, 0, -42, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 65, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -43, -43, 0, 0, 
0, 0, -43, -43, -43, 0, 25, 40, 43, 46, 48, 68, 0, 0, -43, -43, 0, 0, 0, 
0, -43, -43, -43, 0, -43, -43, 0, 0, 0, 0, -43, 0, 0, -43, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 67, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -44, -44, 0, 0, 
0, 0, -44, -44, -44, 0, 25, 40, 43, 46, 48, 68, 0, 0, -44, -44, 0, 0, 0, 
0, -44, -44, -44, 0, -44, -44, 0, 0, 0, 0, -44, 0, 0, -44, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 69, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -45, -45, -45, 
-45, -45, -45, -45, -45, -45, 0, -45, -45, -45, -45, -45, -45, 0, 0, -45, 
-45, 0, 0, 0, 0, -45, -45, -45, 0, -45, -45, 0, 0, 0, 0, -45, 0, 0, -45, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 71, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, -46, -46, 0, 0, 0, 
0, -46, -46, -46, 0, 0, 0, 0, 0, 0, 0, -46, 0, 0, -46, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 

{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 73, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, -47, -47, 0, 0, 0, 
0, -47, -47, -47, 0, 0, 0, 0, 0, 0, 0, -47, 0, 0, -47, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, -48, 74, 0, 0, 0, 
0, 0, -48, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 78, 0, 37, 75, 111, 26, 27, 28, 29, 30, 36, 0, 94, 0, 
0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, -49, 0, 0, 0, 0, 
0, 76, -49, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 77, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, -50, 0, 0, 0, 0, 
0, 0, -50, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -51, -51, -51, 
-51, -51, -51, -51, -51, -51, 0, -51, -51, -51, -51, -51, -51, 0, 0, -51, 
-51, 0, 0, 0, 0, -51, -51, -51, 0, -51, -51, 0, 0, 0, 0, -51, 0, 0, -51, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 80, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 81, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -52, -52, -52, 
-52, -52, -52, -52, -52, -52, 0, -52, -52, -52, -52, -52, -52, 0, 0, -52, 
-52, 0, 0, 0, 0, -52, -52, -52, 0, -52, -52, 0, 0, 0, 0, -52, 0, 0, -52, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -53, -53, -53, 
-53, -53, -53, -53, -53, -53, 0, -53, -53, -53, -53, -53, -53, 0, -54, 
-53, -53, 0, 0, 0, -54, -53, -53, -53, 0, -53, -53, 0, 0, 0, 0, -53, 0, 0, 
-53, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -55, -55, -55, 
-55, -55, -55, -55, -55, -55, 0, -55, -55, -55, -55, -55, -55, 0, -56, 
-55, -55, 0, 0, 0, -56, -55, -55, -55, 0, -55, -55, 0, 0, 0, 0, -55, 0, 0, 
-55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -57, -57, -57, 
-57, -57, -57, -57, -57, -57, 0, -57, -57, -57, -57, -57, -57, 0, 0, -57, 
-57, 0, 0, 0, 0, -57, -57, -57, 0, -57, -57, 0, 0, 0, 0, -57, 0, 0, -57, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -58, -58, -58, 
-58, -58, -58, -58, -58, -58, 0, -58, -58, -58, -58, -58, -58, 0, 0, -58, 
-58, 0, 0, 0, 0, -58, -58, -58, 0, -58, -58, 0, 0, 0, 0, -58, 0, 0, -58, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -59, -59, -59, 
-59, -59, -59, -59, -59, -59, 0, -59, -59, -59, -59, -59, -59, 0, 0, -59, 
-59, 0, 0, 0, 0, -59, -59, -59, 0, -59, -59, 0, 0, 0, 0, -59, 0, 0, -59, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -60, -60, -60, 
-60, -60, -60, -60, -60, -60, 0, -60, -60, -60, -60, -60, -60, 0, -61, 
-60, -60, 0, 0, 0, -61, -60, -60, -60, 0, -60, -60, 0, 0, 0, 0, -60, 0, 0, 
-60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -62, -62, -62, 
-62, -62, -62, -62, -62, -62, 0, -62, -62, -62, -62, -62, -62, 0, 0, -62, 
-62, 0, 0, 0, 0, -62, -62, -62, 0, -62, -62, 0, 0, 0, 0, -62, 0, 0, -62, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -63, -63, -63, 
-63, -63, -63, -63, -63, -63, 0, -63, -63, -63, -63, -63, -63, 0, -64, 
-63, -63, 0, 0, 0, -64, -63, -63, -63, 0, -63, -63, 0, 0, 0, 0, -63, 0, 0, 
-63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -65, -65, -65, 
-65, -65, -65, -65, -65, -65, 0, -65, -65, -65, -65, -65, -65, 0, 0, -65, 
-65, 0, 0, 0, 0, -65, -65, -65, 0, -65, -65, 0, 0, 0, 0, -65, 0, 0, -65, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -66, -66, -66, 
-66, -66, -66, -66, -66, -66, 0, -66, -66, -66, -66, -66, -66, 0, 0, -66, 
-66, 0, 0, 0, 0, -66, -66, -66, 0, -66, -66, 0, 0, 0, 0, -66, 0, 0, -66, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -67, -67, -67, 
-67, -67, -67, -67, -67, -67, 0, -67, -67, -67, -67, -67, -67, 0, 0, -67, 
-67, 0, 0, 0, 0, -67, -67, -67, 0, -67, -67, 0, 0, 0, 0, -67, 0, 0, -67, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -68, -68, -68, 
-68, -68, -68, -68, -68, -68, 0, -68, -68, -68, -68, -68, -68, 0, 0, -68, 
-68, 0, 0, 0, 0, -68, -68, -68, 0, -68, -68, 0, 0, 0, 0, -68, 0, 0, -68, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 95, 0, 0, 0, 0, 0, 101, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, -69, 0, 74, 0, 0, 0, 
0, 0, -69, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 78, 0, 37, 96, 0, 26, 27, 28, 29, 30, 36, 97, 94, 0, 
0}, 

{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, -70, 0, 0, 0, 0, 0, 
0, 0, -70, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 98, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 99, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, -71, 0, 0, 0, 0, 0, 
0, 0, -71, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -72, -72, -72, 
-72, -72, -72, -72, -72, -72, 0, -72, -72, -72, -72, -72, -72, 0, 0, -72, 
-72, 0, 0, 0, 0, -72, -72, -72, 0, -72, -72, 0, 0, 0, 0, -72, 0, 0, -72, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 102, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 107, 110, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -26, -26, -26, 
-26, -26, -26, -26, -26, -26, 0, -26, -26, -26, -26, -26, -26, -26, 0, 
-26, -26, 0, 0, 0, -26, -26, -26, -26, 0, -26, -26, 0, 0, 0, 0, -26, 0, 0, 
-26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -61, -61, -61, 
-61, -61, -61, -61, -61, -61, 0, -61, -61, -61, -61, -61, -61, -61, -61, 
-61, -61, 0, 0, 0, -61, -61, -61, -61, 0, -61, -61, 0, 0, 0, 0, -61, 0, 0, 
-61, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -64, -64, -64, 
-64, -64, -64, -64, -64, -64, 0, -64, -64, -64, -64, -64, -64, -64, -64, 
-64, -64, 0, 0, 0, -64, -64, -64, -64, 0, -64, -64, 0, 0, 0, 0, -64, 0, 0, 
-64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -56, -56, -56, 
-56, -56, -56, -56, -56, -56, 0, -56, -56, -56, -56, -56, -56, -56, -56, 
-56, -56, 0, 0, 0, -56, -56, -56, -56, 0, -56, -56, 0, 0, 0, 0, -56, 0, 0, 
-56, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -54, -54, -54, 
-54, -54, -54, -54, -54, -54, 0, -54, -54, -54, -54, -54, -54, -54, -54, 
-54, -54, 0, 0, 0, -54, -54, -54, -54, 0, -54, -54, 0, 0, 0, 0, -54, 0, 0, 
-54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -73, -73, -73, 
-73, -73, -73, -73, -73, -73, 0, -73, -73, -73, -73, -73, -73, -73, 0, 
-73, -73, 0, 0, 0, 108, -73, -73, -73, 0, -73, -73, 0, 0, 0, 0, -73, 0, 0, 
-73, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 102, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 107, 109, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -74, -74, -74, 
-74, -74, -74, -74, -74, -74, 0, -74, -74, -74, -74, -74, -74, -74, 0, 
-74, -74, 0, 0, 0, 0, -74, -74, -74, 0, -74, -74, 0, 0, 0, 0, -74, 0, 0, 
-74, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -75, -75, -75, 
-75, -75, -75, -75, -75, -75, 0, -75, -75, -75, -75, -75, -75, -75, 0, 
-75, -75, 0, 0, 0, 0, -75, -75, -75, 0, -75, -75, 0, 0, 0, 0, -75, 0, 0, 
-75, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 112, 0, 0, 0, 0, 0, 0, 113, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -76, -76, -76, 
-76, -76, -76, -76, -76, -76, 0, -76, -76, -76, -76, -76, -76, 0, 0, -76, 
-76, 0, 0, 0, 0, -76, -76, -76, 0, -76, -76, 0, 0, 0, 0, -76, 0, 0, -76, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 114, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, -77, 0, 0, 0, 0, 
0, 115, -77, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 116, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, -78, 0, 0, 0, 0, 
0, 0, -78, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -79, -79, -79, 
-79, -79, -79, -79, -79, -79, 0, -79, -79, -79, -79, -79, -79, 0, 0, -79, 
-79, 0, 0, 0, 0, -79, -79, -79, 0, -79, -79, 0, 0, 0, 0, -79, 0, 0, -79, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -80, -80, -80, 
-80, -80, -80, -80, -80, -80, 0, -80, -80, 43, 46, 48, 68, 0, 0, -80, -80, 
0, 0, 0, 0, -80, -80, -80, 0, -80, -80, 0, 0, 0, 0, -80, 0, 0, -80, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -81, -81, -81, 
-81, -81, -81, -81, -81, -81, 0, -81, -81, 43, 46, 48, 68, 0, 0, -81, -81, 
0, 0, 0, 0, -81, -81, -81, 0, -81, -81, 0, 0, 0, 0, -81, 0, 0, -81, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0}, 

{0, 0, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 121, 4, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 122, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -82, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 124, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 126, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 125, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, 0, 0, 0, 0, 0, 
-83, 0, 0, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 127, 4, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -84, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, -48, 74, 0, 0, 0, 
0, 0, -48, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 78, 0, 37, 75, 130, 26, 27, 28, 29, 30, 36, 0, 94, 0, 
0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 131, 0, 0, 0, 0, 0, 0, 113, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 132, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 133, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, 0, 0, 0, 0, 0, 
-85, 0, 0, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, -86, 0, 0, -86, 0, -86, 0, -86, -86, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -25, -26, 0, 0, 135, 0, 0, 
-26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 136, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, -87, 0, 0, -87, 0, -87, 0, -87, -87, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 138, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 72, 70, 0, 0, 0, 0, 139, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 140, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 141, 4, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 142, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -88, -88, 0, 0, -88, 0, -88, 0, -88, -88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -88, -88, -88, 0, 
-88, 0, 0, 0, 0, 0, 0, 0, 0, -88, 0, 143, -88, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 144, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 

{0, 0, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 145, 4, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 146, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -89, -89, 0, 0, -89, 0, -89, 0, -89, -89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -89, -89, -89, 0, 
-89, 0, 0, 0, 0, 0, 0, 0, 0, -89, 0, 0, -89, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 33, 84, 85, 87, 88, 89, 90, 82, 83, 32, 31, 0, 86, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 44, 38, 41, 0, 0, 0, 0, 0, 79, 0, 0, 74, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 91, 92, 93, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 78, 0, 37, 148, 0, 26, 27, 28, 29, 30, 36, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 56, 58, 60, 62, 
64, 66, 50, 52, 54, 0, 25, 40, 43, 46, 48, 68, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 72, 70, 0, 0, 0, 0, 0, 0, 0, 149, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 150, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 134, 0, 0, 103, 0, 104, 0, 106, 105, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 129, 11, -1, 0, 
-2, 0, 0, 0, 0, 0, 0, 0, 0, 137, 0, 0, 147, 0, 0, 0, 151, 4, 5, 7, 9, 10, 
14, 15, 16, 17, 18, 19, 20, 21, 123, 0, 0, 0, 0, 0, 0, 0, 0, 0, 94, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 152, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, -90, -90, 0, 0, -90, 0, -90, 0, -90, -90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -90, -90, -90, 0, 
-90, 0, 0, 0, 0, 0, 0, 0, 0, -90, 0, 0, -90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -25, 0, 0, 0, 0, 0, 0, -26, -25, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 101, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
	return _actionTable[state][sym];
}

ReduceInfo *PglParser::reduceTable(int i) {
	static ReduceInfo _reduceTable[] = {
{ 0, 0, 0, 0},
{ 59, 0, 0, p_StatementSeq_1, 0 },
{ 61, 0, 0, p_SimpleStatement_2, 0 },
{ 58, 1, 1, p_Code_3, 0 },
{ 59, 2, 2, p_StatementSeq_4, 0 },
{ 60, 2, 2, p_Statement_5, 0 },
{ 60, 1, 1, p_Statement_6, 0 },
{ 60, 1, 1, p_Statement_7, 0 },
{ 60, 3, 3, p_Statement_8, 0 },
{ 61, 1, 1, p_SimpleStatement_9, 0 },
{ 61, 1, 1, p_SimpleStatement_10, 0 },
{ 65, 1, 1, p_DeclStatement_11, 0 },
{ 65, 1, 1, p_DeclStatement_12, 0 },
{ 65, 1, 1, p_DeclStatement_13, 0 },
{ 64, 1, 1, p_ExprStatement_14, 0 },
{ 64, 1, 1, p_ExprStatement_15, 0 },
{ 66, 2, 2, p_PureDecl_16, 0 },
{ 67, 4, 4, p_ExprInitDecl_17, 0 },
{ 73, 1, 1, p_Expression_18, 0 },
{ 73, 1, 1, p_Expression_19, 0 },
{ 73, 1, 1, p_Expression_20, 0 },
{ 73, 1, 1, p_Expression_21, 0 },
{ 73, 1, 1, p_Expression_22, 0 },
{ 73, 1, 1, p_Expression_23, 0 },
{ 73, 1, 1, p_Expression_24, 0 },
{ 72, 1, 1, p_ObjName_25, 0 },
{ 82, 1, 1, p_Name_26, 0 },
{ 73, 2, 2, p_Expression_27, 0 },
{ 73, 2, 2, p_Expression_28, 0 },
{ 73, 1, 1, p_Expression_29, 0 },
{ 73, 1, 1, p_Expression_30, 0 },
{ 73, 2, 2, p_Expression_31, 0 },
{ 73, 2, 2, p_Expression_32, 0 },
{ 73, 2, 2, p_Expression_33, 0 },
{ 73, 3, 3, p_Expression_34, 0 },
{ 73, 3, 3, p_Expression_35, 0 },
{ 73, 3, 3, p_Expression_36, 0 },
{ 73, 3, 3, p_Expression_37, 0 },
{ 73, 3, 3, p_Expression_38, 0 },
{ 73, 3, 3, p_Expression_39, 0 },
{ 73, 3, 3, p_Expression_40, 0 },
{ 73, 3, 3, p_Expression_41, 0 },
{ 73, 3, 3, p_Expression_42, 0 },
{ 73, 3, 3, p_Expression_43, 0 },
{ 73, 3, 3, p_Expression_44, 0 },
{ 73, 3, 3, p_Expression_45, 0 },
{ 73, 3, 3, p_Expression_46, 0 },
{ 73, 3, 3, p_Expression_47, 0 },
{ 74, 0, 0, p_Array_48, 0 },
{ 74, 1, 1, p_Array_49, 0 },
{ 74, 3, 3, p_Array_50, 0 },
{ 73, 1, 1, p_Expression_51, 0 },
{ 73, 3, 3, p_Expression_52, 0 },
{ 75, 1, 1, p_Bool_53, 0 },
{ 82, 1, 1, p_Name_54, 0 },
{ 75, 1, 1, p_Bool_55, 0 },
{ 82, 1, 1, p_Name_56, 0 },
{ 76, 1, 1, p_Int_57, 0 },
{ 77, 1, 1, p_Num_58, 0 },
{ 77, 1, 1, p_Num_59, 0 },
{ 78, 1, 1, p_Time_60, 0 },
{ 82, 1, 1, p_Name_61, 0 },
{ 78, 1, 1, p_Time_62, 0 },
{ 79, 1, 1, p_Size_63, 0 },
{ 82, 1, 1, p_Name_64, 0 },
{ 79, 1, 1, p_Size_65, 0 },
{ 80, 1, 1, p_Qualif_66, 0 },
{ 80, 1, 1, p_Qualif_67, 0 },
{ 80, 1, 1, p_Qualif_68, 0 },
{ 81, 0, 0, p_List_69, 0 },
{ 81, 1, 1, p_List_70, 0 },
{ 81, 3, 3, p_List_71, 0 },
{ 70, 4, 4, p_Call_72, 0 },
{ 83, 1, 1, p_ObjNameTail_73, 0 },
{ 83, 3, 3, p_ObjNameTail_74, 0 },
{ 72, 3, 3, p_ObjName_75, 0 },
{ 73, 3, 3, p_Expression_76, 0 },
{ 74, 3, 3, p_Array_77, 0 },
{ 74, 5, 5, p_Array_78, 0 },
{ 73, 3, 3, p_Expression_79, 0 },
{ 73, 3, 3, p_Expression_80, 0 },
{ 73, 3, 3, p_Expression_81, 0 },
{ 68, 6, 6, p_CodeInitDecl_82, 0 },
{ 69, 3, 3, p_Assignment_83, 0 },
{ 69, 5, 5, p_Assignment_84, 0 },
{ 69, 5, 5, p_Assignment_85, 0 },
{ 71, 1, 1, p_TypeName_86, 0 },
{ 71, 3, 3, p_TypeName_87, 0 },
{ 62, 6, 6, p_IfCode_88, 0 },
{ 62, 10, 10, p_IfCode_89, 0 },
{ 63, 6, 6, p_EveryCode_90, 0 }};
	return(&_reduceTable[i]);
}


#line 117 "pgl.syn"

// postlude


