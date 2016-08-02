%prelude
%%

// character sets
sign  =
	$- | $+ .
digit =
	$0 .. $9 .
alpha =
	$A .. $Z | $a .. $z | $_ .
any = // any printable char
	#9 | #32 .. #126 .
notQuote = // any printable char except three quotes (`'")
	#9 | #32 .. #33 | #35 .. #38 | #40 .. #95 | #97 .. #126 .  
notAt = // any printable char except '@'
	#9 | #32 .. #63 | #65 .. #126 .
notSlash = // any printable char except '/'
	#9 | #32 .. #46 | #48 .. #126 .
inum =
	{ digit }+ .
fpnum =
	{ digit }+ $. { digit }* .
identifier =
	alpha { alpha | digit }* .

timeScales =
	"msec" | "sec" | "min" | "hour" | "day" | "year" .

sizeScales =
	"bit" | "Byte" | "Kb" | "KB" | "MB" | "Mb" | "Gb" | "GB".

timeScales2 =
	timeScales | "ms" | "s" | "hr" .

%ignore
// such tokens would be ignored by the generated lexer

spaces =
	{ #9 | #10 | #11 | #12 | #13 | #32 }+ .

// comments?


%tokens
// such tokens would be returned by the generated lexer

ASGN_TOKEN =	$= .
EQUAL_TOKEN =	$= $= .
NEQUAL_TOKEN =	$! $= .
LT_TOKEN =	$< .
LTE_TOKEN =	$< $= .
GT_TOKEN = $> .
GTE_TOKEN =	$> $= .
PLUS_TOKEN =	$+ .
MINUS_TOKEN = $- .
MUL_TOKEN =	$* .
DIV_TOKEN =	$/ .
MOD_TOKEN =	$% .
LEFTPARENT_TOKEN =	$( .
RIGHTPARENT_TOKEN =	$) .
LEFTBRACKET_TOKEN =	$[ .
RIGHTBRACKET_TOKEN =	$] .
LEFTBRACE_TOKEN =	${ .
RIGHTBRACE_TOKEN =	$} .
SEMICOLON_TOKEN =	$; .
PERIOD_TOKEN = $. .
COLON_TOKEN =	$: .
COMMA_TOKEN =	$, .
POUND_TOKEN =	$# .
POWER_TOKEN =	$^ .
NOT_TOKEN =	$! .
THRU_TOKEN =	$. $. .
CLONE_TOKEN =	$* $* .
BOOL_AND_TOKEN = $& $& .
BOOL_OR_TOKEN = $| $| .
BOOL_XOR_TOKEN = $^ $^ .

CMNT_LINE_TOKEN =	$/ $/ .
CMNT_OPEN_TOKEN =	$/ $* .
CMNT_CLOSE_TOKEN =	$* $/ .

INT_TOKEN = inum .
NUM_TOKEN = fpnum | (inum | fpnum) $e [sign] inum .
NUM_PERC_TOKEN = (inum | fpnum) $% .

BOOL_FALSE_TOKEN = "no" | "false" | "off" .
BOOL_TRUE_TOKEN = "yes" | "true" | "on" .

TIME_SCALE_TOKEN = timeScales .
TIME_TOKEN = [inum | fpnum] timeScales2 .

SIZE_SCALE_TOKEN = sizeScales .
SIZE_TOKEN = [inum | fpnum] sizeScales .

// keywords must preceed IDentifiers
kw_if_TOKEN = "if" .
kw_then_TOKEN = "then" .
kw_else_TOKEN = "else" .
kw_every_TOKEN = "every" .
kw_do_TOKEN = "do" .
kw_lmt_TOKEN = "lmt" .
kw_now_TOKEN = "now" .
kw_nmt_TOKEN = "nmt" .

// regular expression
// RE_DEFAULT_TOKEN = identifier $= { $~ | $= } $/ { notSlash | $\ $/ }*  $/ [ { alpha }+ ] .
// RE_CUSTOM_TOKEN  = identifier $= { $~ | $= } $@ { notAt | $\ $@ }*  $@ [ { alpha }+ ] .
RE_DEFAULT_TOKEN = $= { $~ | $= } $/ { notSlash | $\ $/ }*  $/ [ { alpha }+ ] .
RE_CUSTOM_TOKEN  = $= { $~ | $= } $@ { notAt | $\ $@ }*  $@ [ { alpha }+ ] .

ID_TOKEN = identifier .
SQW_STR_TOKEN = $' {notQuote | $` | $" | $\ $'}* $' %filter token.trim('\''); %%.
DQW_STR_TOKEN = $" {notQuote | $` | $' | $\ $"}* $" %filter token.trim('"');  %%.
BQW_STR_TOKEN = $` {notQuote | $' | $" | $\ $`}* $` %filter token.trim('`');  %%.

%postlude
// trailer
%%
