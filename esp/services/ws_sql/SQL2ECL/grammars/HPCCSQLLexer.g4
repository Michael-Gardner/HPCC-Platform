
lexer grammar HPCCSQLLexer;

tokens
{
    TOKEN_ROOT,
    TOKEN_SELECT_STATEMENT,
    TOKEN_CALL_STATEMENT,
    TOKEN_CREATE_LOAD_TABLE_STATEMENT,
    TOKEN_CREATE_INDEX_STATEMENT,
    TOKEN_CREATE_TABLE,
    TOKEN_DONOT_OVERWRITE,
    TOKEN_OVERWRITE,
    TOKEN_LOAD_TABLE,
    TOKEN_FROM_LIST,
    TOKEN_FROM_TABLE,
    TOKEN_PROC_NAME,
    TOKEN_PROC_PARAMS,
    TOKEN_ALIAS,
    TOKEN_INNER_JOIN,
    TOKEN_OUTTER_JOIN,
    TOKEN_INDEX_HINT,
    TOKEN_AVOID_INDEX,
    TOKEN_COLUMN,
    TOKEN_LISTEXP,
    TOKEN_FUNCEXP,
    TOKEN_PARAMPLACEHOLDER,
    TOKEN_COLUMNWILDCARD,
    TOKEN_TABLE_SCHEMA,
    TOKEN_COLUMN_DEF_LIST,
    TOKEN_COLUMN_DEF,
    TOKEN_LANDING_ZONE,
    TOKEN_VARIABLE_FILE,
    TOKEN_VAR_SEPERATOR,
    TOKEN_VAR_TERMINATOR,
    TOKEN_VAR_ENCLOSED,
    TOKEN_VAR_ESCAPED
}

@lexer::includes
{
}
@lexer::apifuncs
{
}
@lexer::header
{
}
@lexer::members
{
}

fragment Ai :  'a' | 'A';
fragment Bi :  'b' | 'B';
fragment Ci :  'c' | 'C';
fragment Di :  'd' | 'D';
fragment Ei :  'e' | 'E';
fragment Fi :  'f' | 'F';
fragment Gi :  'g' | 'G';
fragment Hi :  'h' | 'H';
fragment Ii :  'i' | 'I';
fragment Ji :  'j' | 'J';
fragment Ki :  'k' | 'K';
fragment Li :  'l' | 'L';
fragment Mi :  'm' | 'M';
fragment Ni :  'n' | 'N';
fragment Oi :  'o' | 'O';
fragment Pi :  'p' | 'P';
fragment Qi :  'q' | 'Q';
fragment Ri :  'r' | 'R';
fragment Si :  's' | 'S';
fragment Ti :  't' | 'T';
fragment Ui :  'u' | 'U';
fragment Vi :  'v' | 'V';
fragment Wi :  'w' | 'W';
fragment Xi :  'x' | 'X';
fragment Yi :  'y' | 'Y';
fragment Zi :  'z' | 'Z';

ADD_SYM     : Ai Di Di  ;
ALL         : Ai Li Li  ;
ANY         : Ai Ni Yi ;
AS_SYM      : Ai Si  ;
ASC         : Ai Si Ci  ;
ASCII_SYM   : Ai Si Ii Ii ;
AT_SYM      : Ai Ti  ;
AVG         : Ai Vi Gi ;
BETWEEN     : Bi Ei Ti Wi Ei Ei Ni  ;
BINARY_SYM  : Bi Ii Ni Ai Ri Yi ;
BIT_SYM     : Bi Ii Ti ;
BOOL_SYM    : Bi Oi Oi Li  ;
BOOLEAN_SYM : Bi Oi Oi Li Ei Ai Ni  ;
BY_SYM      : Bi Yi ;
CALL_SYM    : Ci Ai Li Li  ;
CREATE_SYM  : Ci Ri Ei Ai Ti Ei ;
COLUMN_SYM  : Ci Oi Li Ui Mi Ni  ;
COMMENT_SYM : Ci Oi Mi Mi Ei Ni Ti ;
CONTAINS_SYM : Ci Oi Ni Ti Ai Ii Ni Si  ;
COUNT       : Ci Oi Ui Ni Ti  ;
DATA_SYM    : Di Ai Ti Ai ;
DESC        : Di Ei Si Ci  ;
DISTINCT    : Di Ii Si Ti Ii Ni Ci Ti ;
EXISTS_SYM  : Ei Xi Ii Si Ti Si ;
FALSE_SYM   : Fi Ai Li Si Ei ;
FOR_SYM     : Fi Oi Ri ;
FROM        : Fi Ri Oi Mi  ;
GROUP_SYM   : Gi Ri Oi Ui Pi  ;
HAVING      : Hi Ai Vi Ii Ni Gi  ;
IF_SYM      : Ii Fi ;
IGNORE_SYM  : Ii Gi Ni Oi Ri Ei ;
IN_SYM      : Ii Ni ;
INFILE_SYM  : Ii Ni Fi Ii Li Ei ;
INTO_SYM    : Ii Ni Ti Oi ;
INT_SYM     : Ii Ni Ti ;
INTEGER_SYM : Ii Ni Ti Ei Gi Ei Ri ;
INDEX_SYM   : Ii Ni Di Ei Xi  ;
INNER_SYM   : Ii Ni Ni Ei Ri  ;
IS_SYM      : Ii Si  ;
JOIN_SYM    : Ji Oi Ii Ni  ;
KEY_SYM     : Ki Ei Yi  ;
KEYS        : Ki Ei Yi Si  ;
LAST_SYM    : Li Ai Si Ti  ;
LEFT        : Li Ei Fi Ti  ;
LIKE_SYM    : Li Ii Ki Ei  ;
LIMIT       : Li Ii Mi Ii Ti  ;
LOAD_SYM    : Li Oi Ai Di ;
LOCAL_SYM   : Li Oi Ci Ai Li ;
LOWER       : (Li Oi Wi Ei Ri) | (Li Ci Ai Si Ei) ;
MAX_SYM     : Mi Ai Xi  ;
MEDIUMINT   : Mi Ei Di Ii Ui Mi Ii Ni Ti ;
MIN_SYM     : Mi Ii Ni  ;
MOD         : Mi Oi Di  ;
NOT_SYM     : (UNDERSCORE Ni Oi Ti) | (Ni Oi Ti) | ('!') ;
NULL_SYM    : Ni Ui Li Li  ;
OFFSET_SYM  : Oi Fi Fi Si Ei Ti  ;
ON          : Oi Ni  ;
ORDER_SYM   : Oi Ri Di Ei Ri  ;
OUT_SYM     : Oi Ui Ti  ;
OUTER       : Oi Ui Ti Ei Ri  ;
POWER       : Pi Oi Wi Ei Ri  ;
REPLACE_SYM : Ri Ei Pi Li Ai Ci Ei ;
SELECT      : Si Ei Li Ei Ci Ti ;
SMALLINT    : Si Mi Ai Li Li Ii Ni Ti ;
SUM         : Si Ui Mi  ;
TABLE_SYM   : Ti Ai Bi Li Ei ;
TINYINT     : Ti Ii Ni Yi Ii Ni Ti ;
TRUE_SYM    : Ti Ri Ui Ei ;
TYPE_SYM    : Ti Yi Pi Ei ;
UPPER       : Ui Pi Pi Ei Ri ;
USE_SYM     : Ui Si Ei  ;
UTF8_SYM    : Ui Ti Fi '8' ;
UNSIGNED_SYM : Ui Ni Si Ii Gi Ni Ei Di ;
WHERE       : Wi Hi Ei Ri Ei  ;
XOR         : Xi Oi Ri  ;
AT_SIGN     : '@';
NONE_SYM    : Ni Oi Ni Ei;

BIGINT_SYM  : Bi Ii Gi Ii Ni Ti ;
REAL_SYM    : Ri Ei Ai Li ;
DOUBLE_SYM  : Di Oi Ui Bi Li Ei ;
FLOAT_SYM   : Fi Li Oi Ai Ti ;
DECIMAL_SYM : Di Ei Ci Ii Mi Ai Li ;
NUMERIC_SYM : Ni Ui Mi Ei Ri Ii Ci ;
DATE_SYM    : Di Ai Ti Ei ;
TIME_SYM    : Ti Ii Mi Ei ;
TIMESTAMP_SYM  : Ti Ii Mi Ei Si Ti Ai Mi Pi ;
DATETIME_SYM   : Di Ai Ti Ei Ti Ii Mi Ei ;
YEAR_SYM       : Yi Ei Ai Ri ;
CHAR_SYM       : Ci Hi Ai Ri ;
VARCHAR_SYM    : Vi Ai Ri Ci Hi Ai Ri ;
VARBINARY_SYM  : Vi Ai Ri Bi Ii Ni Ai Ri Yi ;
TINYBLOB_SYM   : Ti Ii Ni Yi Bi Li Oi Bi ;
BLOB_SYM       : Bi Li Oi Bi ;
MEDIUMBLOB_SYM : Mi Ei Di Ii Ui Mi Bi Li Oi Bi ;
LONGBLOB_SYM   : Li Oi Ni Gi Bi Li Oi Bi ;
TINYTEXT_SYM   : Ti Ii Ni Yi Ti Ei Xi Ti ;
TEXT_SYM       : Ti Ei Xi Ti ;
MEDIUMTEXT_SYM : Mi Ei Di Ii Ui Mi Ti Ei Xi Ti ;
LONGTEXT_SYM   : Li Oi Ni Gi Ti Ei Xi Ti ;
ENUM_SYM       : Ei Ni Ui Mi ;
SET_SYM        : Si Ei Ti ;
FLAT_SYM       : Fi Li Ai Ti ;
XML_SYM        : Xi Mi Li ;
CSV_SYM        : Ci Si Vi ;
JSON_SYM       : Ji Si Oi Ni ;
CONNECTION_SYM : Ci Oi Ni Ni Ei Ci Ti Ii Oi Ni ;
DIRECTORY_SYM  : Di Ii Ri Ei Ci Ti Oi Ri Yi ;
ENCLOSED_SYM   : Ei Ni Ci Li Oi Si Ei Di ;
LINES_SYM      : Li Ii Ni Ei Si ;
ESCAPED_SYM    : Ei Si Ci Ai Pi Ei Di ;
TERMINATED_SYM : Ti Ei Ri Mi Ii Ni Ai Ti Ei Di ;
OPTIONALLY_SYM : Oi Pi Ti Ii Oi Ni Ai Li Li Yi ;
EBCDIC_SYM     : Ei Bi Ci Di Ii Ci ;
FIELDS_SYM     : Fi Ii Ei Li Di Si ;
COLUMNS_SYM    : Ci Oi Li Ui Mi Ni Si ;

CHARACTER_SET : ('CHARACTER SET' | 'character set');
IFNOTEXISTS : ('IF NOT EXISTS' | 'if not exists');
ISNOTNULL   : ('IS NOT NULL' | 'is not null');
ISNULL      : ('IS NULL' | 'is null');
NOT_IN      : ('NOT IN' | 'not in');
NOT_LIKE    : ('NOT LIKE' | 'not like');

DIVIDE      : (  Di Ii Vi ) | '/' ;
MOD_SYM     : (  Mi Oi Di ) | '%' ;
OR_SYM      : (  Oi Ri ) | '||';
AND_SYM     : (  Ai Ni Di ) | '&&';

ARROW       : '=>' ;
EQ_SYM      : '=' | '<=>' ;
NOT_EQ      : '<>' | '!=' | '~='| '^=';
LET         : '<=' ;
GET         : '>=' ;
SET_VAR     : ':=' ;
SHIFT_LEFT  : '<<' ;
SHIFT_RIGHT : '>>' ;
//ALL_FIELDS  : '.*' ;
SQUOTE      : '\'' ;
//DQUOTE      : '\"' ;
DQUOTE      : '"' ;
DOLLAR      : '$' ;
QUESTION    : '?' ;
SEMI        : ';' ;
COLON       : ':' ;
DOT         : '.' ;
COMMA       : ',' ;
ASTERISK    : '*' ;
RPAREN      : ')' ;
LPAREN      : '(' ;
RBRACK      : ']' ;
LBRACK      : '[' ;
LCURLY      : '{' ;
RCURLY      : '}' ;
PLUS        : '+' ;
MINUS       : '-' ;
NEGATION    : '~' ;
VERTBAR     : '|' ;
BITAND      : '&' ;
POWER_OP    : '^' ;
GTH         : '>' ;
LTH         : '<' ;

fragment UNDERSCORE  : '_' ;

fragment DCOLON      : '::' ;

fragment DEFSCOPE    : DOT DCOLON ;

fragment LETTER_FRAGMENT
:
    ( 'A'..'Z' | 'a'..'z')
;

INTEGER_NUM
:
    DIGIT_FRAGMENT+
;

fragment DIGIT_FRAGMENT
:
    ( '0'..'9')
;

fragment HEX_DIGIT_FRAGMENT
:
   ( 'a'..'f' | 'A'..'F' | DIGIT_FRAGMENT )
;

HEX_DIGIT
:
  (  '0x'     (HEX_DIGIT_FRAGMENT)+  )
  |
  (  'X' '\'' (HEX_DIGIT_FRAGMENT)+ '\''  )
;

BIT_NUM
:
  (  '0b'    ('0'|'1')+  )
  |
  (  Bi '\'' ('0'|'1')+ '\''  )
;

REAL_NUMBER
:
  (  INTEGER_NUM DOT INTEGER_NUM | INTEGER_NUM DOT | DOT INTEGER_NUM | INTEGER_NUM  )
  (  (Ei) ( PLUS | MINUS )? INTEGER_NUM  )?
;

TEXT_STRING
:
    SQUOTE
    (
        (SQUOTE SQUOTE)
        | ('\\''\'')
        | ~('\'')
    )*
    SQUOTE
;

//Cannot rewrite lexer rules see above parser rule
//QUOTED_ID
//:
//  DQUOTE ID DQUOTE
//;

fragment ABSOLUTE_FILE_ID_PREFIX
:
    NEGATION |
    DEFSCOPE |
    NEGATION DEFSCOPE
;

ABSOLUTE_FILE_ID
:
    ABSOLUTE_FILE_ID_PREFIX ID_FRAGMENT+
;

ID
:
    LETTER_FRAGMENT ( ID_FRAGMENT )*
;

fragment ID_FRAGMENT
:
    LETTER_FRAGMENT | UNDERSCORE | INTEGER_NUM | DCOLON
;

WHITE_SPACE
:
  ( ' '|'\r'|'\t'|'\n' ) -> skip
;
