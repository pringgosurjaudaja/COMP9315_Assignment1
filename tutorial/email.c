/*
 * src/tutorial/complex.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */
#include <ctype.h>
PG_MODULE_MAGIC;

typedef struct Email
{
	char *first;
	char *second;
}			Email;
Datum	email_in(PG_FUNCTION_ARGS);
Datum	email_out(PG_FUNCTION_ARGS);
int checkLocal(char *s);
int checkDomain(char *s);
int isLetterDigit(char c);
Datum	email_lt(PG_FUNCTION_ARGS);
Datum	email_le(PG_FUNCTION_ARGS);
Datum	email_eq(PG_FUNCTION_ARGS);
Datum	email_neq(PG_FUNCTION_ARGS);
Datum	email_gt(PG_FUNCTION_ARGS);
Datum	email_ge(PG_FUNCTION_ARGS);
Datum   email_domainNEq(PG_FUNCTION_ARGS);
Datum   email_domainEq(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(email_in);

Datum
email_in(PG_FUNCTION_ARGS)
{
	char	   *str = PG_GETARG_CSTRING(0);
	Email    *result;

	for(int x = 0; str[x];x++){
		str[x] = tolower(str[x]);
	}
	//TODO: check if the input complies with the guideline
	char *first = strtok(str,"@");
	char *second = strtok(NULL, "@");
	//check if the email is correct,if not ereport, else continue
	if(checkLocal(first) == 1 &&checkDomain(second) == 1){
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for email: \"%s\"",
						str)));
	}

	//result = (Email *) palloc(sizeof(Email));
	result = (Email *) palloc(sizeof(char[strlen(first)]) + sizeof(char[strlen(second)]));
	result->first = first;
	result->second = second;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_out);

Datum
email_out(PG_FUNCTION_ARGS)
{
	Email    *email = (Email *) PG_GETARG_POINTER(0);
	char	   *result;

	result = psprintf("%s@%s", email->first, email->second);
	PG_RETURN_CSTRING(result);
}

//function to check if the local part of the email is correct
int checkLocal(char *s){

	int length = strlen(s);
	char previous = s[0]; //previous char to check .
	//if first character is not a letter, fail
	if(isalpha(s[0])==0){
		return 1;
	}
	for(int x = 0; x < length; x++){
		if(isLetterDigit(s[x] == 0)){
			return 1;
		}
		//check if the previous char is . then the first char after must be a letter (e.g. john.2li@unsw.edu.au is false)
		if(previous == '.' && isalpha(s[x]) == 0){
			return 1;
		}
		//TODO: check case when end of word isn't a letter or  digit

		//keeps track of the previous char
		previous = s[x];
	}
	if(isalpha(s[length])==0){
		return 1;
	}
	return 0;
}

//function to check if the domain part of the email i correct
int checkDomain(char *s){
	int length = strlen(s);
	int period = 0;
		char previous = s[0]; //previous char to check .
		//if first character is not a letter, fail
		if(isalpha(s[0])==0){
			return 1;
		}
		for(int x = 0; x < length; x++){
			if(isLetterDigit(s[x] == 0)){
				return 1;
			}
			//check if the previous char is . then the first char after must be a letter (e.g. john.2li@unsw.edu.au is false)
			if(previous == '.' && isalpha(s[x]) == 0){
				period = 1;
				return 1;
			}
			//TODO: check case when end of word isn't a letter or  digit

			//keeps track of the previous char
			previous = s[x];
		}
	if(period ==0){
		return 1;
	}
	return 0;
}

int isLetterDigit(char c){
	if(isalpha(c) || isdigit(c)|| c =='.'|| c =='-'){
		return 1;
	}
	return 0;
}
/*****************************************************************************
 * New Operators
 *
 * A practical Complex datatype would provide much more than this, of course.
 *****************************************************************************/



/*****************************************************************************
 * Operator class for defining B-tree index
 *
 * It's essential that the comparison operators and support function for a
 * B-tree index opclass always agree on the relative ordering of any two
 * data values.  Experience has shown that it's depressingly easy to write
 * unintentionally inconsistent functions.  One way to reduce the odds of
 * making a mistake is to make all the functions simple wrappers around
 * an internal three-way-comparison function, as we do here.
 *****************************************************************************/

#define Mag(c)	((c)->x*(c)->x + (c)->y*(c)->y)

static int
email_cmp_internal(Email a, Email b)
{
	if(strcmp(a->first,b->first)&&strcmp(a->second,b->second)){
		return 0;
	}
	if(strcmp(a->first,b->first) != 0){
		return strcmp(a->first,b->first);
	}
	return 1;
}


PG_FUNCTION_INFO_V1(email_lt);

Datum
email_lt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(email_le);

Datum
email_le(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
		Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(email_neq);

Datum
email_neq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) != 0);
}

PG_FUNCTION_INFO_V1(email_eq);

Datum
email_eq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) == 0);
}

PG_FUNCTION_INFO_V1(email_ge);

Datum
email_ge(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(email_gt);

Datum
email_gt(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) > 0);
}

PG_FUNCTION_INFO_V1(email_domainEq);

Datum
email_domainEq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(email_cmp_internal(a->second, b->second)==0);
}

PG_FUNCTION_INFO_V1(email_domainNEq);

Datum
email_domainNEq(PG_FUNCTION_ARGS)
{
	Email    *a = (Email *) PG_GETARG_POINTER(0);
	Email    *b = (Email *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(email_cmp_internal(a->second, b->second)!=0);
}
