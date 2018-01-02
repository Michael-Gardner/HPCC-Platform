
parser grammar HPCCSQLParser;

options
   { tokenVocab = HPCCSQLLexer; }

relational_op
:
  EQ_SYM
  | LTH
  | GTH
  | NOT_EQ
  | LET
  | GET
;

strcomp_op
:
    LIKE_SYM
    | NOT_LIKE
;

list_op
:
    IN_SYM
    | NOT_IN
;

string_literal
:
   TEXT_STRING
;
number_literal
:
   (PLUS | MINUS)? (INTEGER_NUM | REAL_NUMBER)
;

hex_literal
:
  HEX_DIGIT
;

boolean_literal
:
  TRUE_SYM
  | FALSE_SYM
;

bit_literal
:
    BIT_NUM
;

literal_value
:
  (
      string_literal
      | number_literal
      | hex_literal
      | boolean_literal
      | bit_literal
  )
;

functionList
:
  group_functions
  | char_functions
;

char_functions
:
  LOWER
  | UPPER
;

group_functions
:
  AVG
  | COUNT
  | MAX_SYM
  | MIN_SYM
  | SUM
;

query_set_name      : ID ;
schema_name         : ID ;
table_name          : ABSOLUTE_FILE_ID | ID ;
quoted_table_name   : quoted_table_id;
engine_name         : ID ;
column_name         : ID ;
quoted_column_name  : quoted_id ;
index_name          : ID ;
user_name           : ID ;
function_name       : ID ;
procedure_name      : ID ;

alias
:
  ( AS_SYM )? ID  #standardAlias
  | ( AS_SYM )? quoted_id #quotedAlias
;

column_spec
:
    ( quoted_table_name DOT )? quoted_column_name
    |( table_name DOT )? column_name
;

expression:
	  LPAREN expression (COMMA expression)* RPAREN 				#expressionList
	| expression (ASTERISK|DIVIDE|MOD_SYM|POWER_OP) expression 	#binaryExpression
	| expression (PLUS|MINUS) expression 						#binaryExpression
	| expression relational_op expression						#binaryExpression
	| expression AND_SYM expression								#binaryExpression
	| expression OR_SYM expression								#binaryExpression
    | expression strcomp_op expression                          #binaryExpression
	| (NEGATION|NOT_SYM) expression								#unaryExpression
	| expression (is_not_null|is_null)							#unaryExpression
	| column_spec												#columnSpecExpression
	| literal_value												#literalValueExpression
	| function_call												#functionExpression
	| parameterPlaceHolder										#parameterExpression
;

is_not_null: IS_SYM NOT_SYM NULL_SYM;
is_null: IS_SYM NULL_SYM; 

function_call
:
      functionList ( LPAREN (functionParam (COMMA functionParam)*)? RPAREN ) ?
;

functionParam
:
    (DISTINCT)?
    (
      literal_value
      | column_spec
      | parameterPlaceHolder
      | ASTERISK
    )
;

parameterPlaceHolder
:
    QUESTION
    | DOLLAR LCURLY ID? RCURLY
    | userVariable
;

userVariable
:
  AT_SIGN ID
;

table_references
:
  table_reference ( COMMA table_reference )*
;

table_reference
:
  table_atom |
  table_atom (jointable)+
;

table_list:
  table_atom ( COMMA table_atom )*
  ;

jointable
:
    INNER_SYM JOIN_SYM table_atom join_condition #innerJoin 
    |  OUTER JOIN_SYM table_atom join_condition  #outerJoin
;

table_factor1
:
  table_factor2 (  INNER_SYM JOIN_SYM table_atom join_condition )?
;

table_factor2
:
  table_factor3 ( OUTER JOIN_SYM table_atom join_condition)?
;

table_factor3
:
  table_atom
;

table_atom
:
  table_spec (alias)? (index_hint)?
;

join_condition
:
    ON expression
;

index_hint
:
  USE_SYM INDEX_SYM LPAREN ( index_name | NONE_SYM ) RPAREN 
;

root_statement
@init
{
}
:
  ( data_manipulation_statements)(SEMI)?
;

data_manipulation_statements
:
    select_statement
  | call_statement
  | create_load_table_statement
  //| create_index_statement
;

create_index_statement
:
    CREATE_SYM INDEX_SYM index_name ON table_spec column_list
;

create_load_table_statement
:
    create_table_statement
    (SEMI?)
    load_table_statement
;

table_options
:
    table_option (( COMMA )? table_option)*
;

table_option:
     (  COMMENT_SYM (EQ_SYM?) string_literal  )
;

create_table_statement
:
    CREATE_SYM TABLE_SYM noov=IFNOTEXISTS? table_name create_table_columns_definition
    (table_options)?
;

create_table_columns_definition
:
    LPAREN create_definition (COMMA create_definition)* RPAREN
;

create_definition
:
       column_name column_definition
;

length_and_or_precision_definition
:
    LPAREN length ( COMMA number_literal)? RPAREN
;

length_and_precision_definition
:
    LPAREN length COMMA number_literal RPAREN
;

length_definition
:
    LPAREN length RPAREN
;

text_params
:
    LPAREN string_literal (COMMA string_literal)* RPAREN
;
column_definition
:
   BIT_SYM       length_definition?                                
   | TINYINT     length_definition? UNSIGNED_SYM?                  
   | SMALLINT    length_definition? UNSIGNED_SYM?                  
   | MEDIUMINT   length_definition? UNSIGNED_SYM?                  
   | INT_SYM     length_definition? UNSIGNED_SYM?                  
   | INTEGER_SYM length_definition? UNSIGNED_SYM?                  
   | BIGINT_SYM  length_definition? UNSIGNED_SYM?                  
   | REAL_SYM    length_and_precision_definition?    UNSIGNED_SYM? 
   | DOUBLE_SYM  length_and_precision_definition?    UNSIGNED_SYM? 
   | FLOAT_SYM   length_and_precision_definition?    UNSIGNED_SYM? 
   | DECIMAL_SYM length_and_or_precision_definition? UNSIGNED_SYM? 
   | NUMERIC_SYM length_and_or_precision_definition? UNSIGNED_SYM? 
   | DATE_SYM
   | TIME_SYM
   | TIMESTAMP_SYM
   | DATETIME_SYM
   | YEAR_SYM
   | CHAR_SYM      length_definition? charset_declaration?  
   | VARCHAR_SYM   length_definition  charset_declaration?  
   | BINARY_SYM    length_definition?                       
   | VARBINARY_SYM length_definition                        
   | TINYBLOB_SYM
   | BLOB_SYM
   | MEDIUMBLOB_SYM
   | LONGBLOB_SYM
   | TINYTEXT_SYM   BINARY_SYM?                      
   | TEXT_SYM       BINARY_SYM? charset_declaration? 
   | MEDIUMTEXT_SYM BINARY_SYM? charset_declaration? 
   | LONGTEXT_SYM   BINARY_SYM? charset_declaration? 
   | ENUM_SYM       text_params charset_declaration? 
   | SET_SYM        text_params charset_declaration? 
;

charset_declaration
:
    CHARACTER_SET charset_name
;
charset_name
:
    ASCII_SYM
    | UTF8_SYM
;

file_data_format_type_options
:
    LPAREN (ID EQ_SYM string_literal) (COMMA (ID EQ_SYM string_literal))* RPAREN
;

file_data_format_type
:
    FLAT_SYM
    | XML_SYM
    | CSV_SYM
    | JSON_SYM
;

file_data_format_declaration
:
    TYPE_SYM file_data_format_type file_data_format_type_options? 
;

variable_data_line_terminator
:
    TERMINATED_SYM BY_SYM t=TEXT_STRING 
;

variable_data_field_terminator
:
    TERMINATED_SYM BY_SYM t=TEXT_STRING 
;

variable_data_escaped
:
    ESCAPED_SYM BY_SYM t=TEXT_STRING 
;

variable_data_enclosed
:
    (OPTIONALLY_SYM)? ENCLOSED_SYM BY_SYM t=TEXT_STRING 
;

variable_data_format_declaration_field
:
    (FIELDS_SYM | COLUMNS_SYM)
    variable_data_field_terminator?
    variable_data_enclosed?
    variable_data_escaped?

    
;

variable_data_format_declaration_line
:
    LINES_SYM
    //(STARTING BY_SYM TEXT_STRING)?
    variable_data_line_terminator?
    
;

landing_zone_information
:
    CONNECTION_SYM EQ_SYM? conn=string_literal
    DIRECTORY_SYM EQ_SYM? dir=string_literal
    
;

load_table_statement
:
    LOAD_SYM DATA_SYM
    landing_zone_information?
    INFILE_SYM ff=string_literal landing_zone_information? file_data_format_declaration?
    INTO_SYM TABLE_SYM table_spec
    variable_data_format_declaration_field?
    variable_data_format_declaration_line?
    
;

select_statement
:
        select_portion
        ( from_portion
          ( where_clause )?
          ( groupby_clause )?
          ( having_clause )?
        )?

        ( orderby_clause )?
        ( limit_clause )?
;

select_portion
:
    SELECT ( DISTINCT )? select_list
;

from_portion
:
    FROM table_references
;

where_clause
:
  WHERE expression
;

groupby_clause
:
  GROUP_SYM BY_SYM column_spec (COMMA column_spec)*
;

having_clause
:
  HAVING expression
;

orderby_clause
:
  ORDER_SYM BY_SYM orderby_item (COMMA orderby_item)*
;

orderby_item
:
  column_spec (ASC | DESC)?
;

limit_clause
:
  LIMIT row_count (OFFSET_SYM offset)?
;

length
:
  INTEGER_NUM
;

offset
:
  INTEGER_NUM
;

row_count
:
  INTEGER_NUM;

select_list
:
    select_item
    (COMMA select_item)*
;

select_item
:
    column_spec (alias)?
    | string_literal (alias)?
    | number_literal (alias)?
    | hex_literal (alias)?
    | boolean_literal (alias)?
    | function_call (alias)?
    | column_wildcard
;

column_list
:
  LPAREN column_spec (COMMA column_spec)* RPAREN
;

subquery
:
  LPAREN select_statement RPAREN
;

table_spec
:
  schema_spec? table_fork 
;

table_fork
:
    (table_name | quoted_table_name)
;

schema_spec
:
    (ID|quoted_id) DOT 
;

column_wildcard
:
    (ID DOT)? ASTERISK 
;

callParam
:
    string_literal
    | number_literal
    | hex_literal
    | boolean_literal
    | parameterPlaceHolder
;

call_procedure_name_part
:
     (query_set_name DOT)? procedure_name 
;

call_procedure_params
:
    callParam ( COMMA callParam)* 
;

call_statement
:
    CALL_SYM call_procedure_name_part LPAREN call_procedure_params? RPAREN
;

quoted_id
:
    DQUOTE ID DQUOTE
;

quoted_table_id
:
    DQUOTE (ABSOLUTE_FILE_ID | ID) DQUOTE
;

