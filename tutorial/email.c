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

typedef struct _email
{
	char first[256];
	char second[256];
}			EmailAddr;

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
Datum   email_deq(PG_FUNCTION_ARGS);
Datum   email_dneq(PG_FUNCTION_ARGS);
Datum	email_cmp(PG_FUNCTION_ARGS);
Datum   email_hv(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(email_in);

Datum
email_in(PG_FUNCTION_ARGS)
{
	char	   *str;
	str = PG_GETARG_CSTRING(0);
	EmailAddr    *result;
	//result = (EmailAddr *) palloc(sizeof(EmailAddr));
	char *first;
	char *second;
	char copy[strlen(str)];
	strcpy(copy, str);
	int x =0;
	char *parse;
	while(x<strlen(str)){
		str[x] = tolower(str[x]);
		x++;	
	}
	first = strtok(copy,"@");
	second = strtok(NULL, "@");
	//check if the email is correct,if not ereport, else continue
	if(checkLocal(first) == 1 &&checkDomain(second) == 1){
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for email: \"%s\"",
						str)));
	}
	result = (EmailAddr *) palloc(sizeof(EmailAddr));
	//char firstbuf[strlen(first)] = first
	//result->first = palloc(sizeof(char)*strlen(first));
	//result->second = palloc(sizeof(char)*strlen(second));
	
	//result->first = (char *) (result+1);
	//result->first = first;
	parse = strtok(str,"@");
	//char first2[strlen(first)];
	//char second2[strlen(second)];
	strcpy(result->first, parse);
	parse = strtok(NULL, "@");
	//strcpy(second2, parse);
	strcpy(result->second, parse);
	//result->second = second;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(email_out);

Datum
email_out(PG_FUNCTION_ARGS)
{
	EmailAddr    *email = (EmailAddr *) PG_GETARG_POINTER(0);
	char	   *result;
	result = (char *) palloc(sizeof(EmailAddr));
	snprintf(result, sizeof(EmailAddr), "%s@%s", email->first, email->second);
	//result = psprintf("%s@%s", email->first, email->second);	
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
	int x = 1;
	while(x < length){
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
		x++;	
	}
	if(isalpha(s[length-1])==0){
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
		int x = 1;
		while(x < length){
			if(isLetterDigit(s[x] == 0)){
				return 1;
			}
			//check if the previous char is . then the first char after must be a letter (e.g. john.2li@unsw.edu.au is false)
			if(previous == '.' && isalpha(s[x]) == 0){
				return 1;
			}
			if(previous == '.' && isalpha(s[x]) == 1){
				period = 1;
			}
			//TODO: check case when end of word isn't a letter or  digit

			//keeps track of the previous char
			previous = s[x];
			x++;
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
email_cmp_internal(EmailAddr *a, EmailAddr *b)
{
	if (strcmp(a->first, b->first) != 0 ) {
	    return strcmp(a->first, b->first);
	}
	if (strcmp(a->second, b->second) != 0 ) {
	    return strcmp(a->second, b->second);
	}
    return 0;
}


PG_FUNCTION_INFO_V1(email_lt);

Datum
email_lt(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(email_le);

Datum
email_le(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
		EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(email_neq);

Datum
email_neq(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) != 0);
}

PG_FUNCTION_INFO_V1(email_eq);

Datum
email_eq(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) == 0);
}

PG_FUNCTION_INFO_V1(email_ge);

Datum
email_ge(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(email_gt);

Datum
email_gt(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(email_cmp_internal(a, b) > 0);
}

PG_FUNCTION_INFO_V1(email_deq);

Datum
email_deq(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a->second, b->second)==0);
}

PG_FUNCTION_INFO_V1(email_dneq);

Datum
email_dneq(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(strcmp(a->second, b->second)!=0);
}
PG_FUNCTION_INFO_V1(email_hv);
Datum email_hv(PG_FUNCTION_ARGS)
{
	PG_RETURN_INT32(1);
}
PG_FUNCTION_INFO_V1(email_cmp);
Datum
email_cmp(PG_FUNCTION_ARGS)
{
	EmailAddr    *a = (EmailAddr *) PG_GETARG_POINTER(0);
	EmailAddr    *b = (EmailAddr *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(email_cmp_internal(a, b));
}
