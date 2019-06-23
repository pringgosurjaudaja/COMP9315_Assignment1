/*
 * src/tutorial/complex.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"
#inlcude <string.h>
#include "fmgr.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

PG_MODULE_MAGIC;

typedef struct Email
{
	char *first;
	char *second;
}			Email;


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(email_in);

Datum
email_in(PG_FUNCTION_ARGS)
{
	char	   *str = PG_GETARG_CSTRING(0);
	Email    *result;

	//TODO: check if the input complies with the guideline
	char *first = strtok(str,"@");
	char *second = strtok(NULL, "@");
	//check if the email is correct,if not ereport, else continue
	if((checkLocal(first) == 1 &&checkDomain(second) == 1){
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for complex: \"%s\"",
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
}

//function to check if the domain part of the email i correct
int checkDomain(char *s){

}

int isLetterDigit(char c){
	if(isalpha(c) || isdigit(c)|| c =='.'|| c =='-'){
		return 1;
	}
	return 0;
}
/*****************************************************************************
 * Binary Input/Output functions
 *
 * These are optional.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(complex_recv);

Datum
complex_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	Complex    *result;

	result = (Complex *) palloc(sizeof(Complex));
	result->x = pq_getmsgfloat8(buf);
	result->y = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(complex_send);

Datum
complex_send(PG_FUNCTION_ARGS)
{
	Complex    *complex = (Complex *) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendfloat8(&buf, complex->x);
	pq_sendfloat8(&buf, complex->y);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * New Operators
 *
 * A practical Complex datatype would provide much more than this, of course.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(complex_add);

Datum
complex_add(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);
	Complex    *result;

	result = (Complex *) palloc(sizeof(Complex));
	result->x = a->x + b->x;
	result->y = a->y + b->y;
	PG_RETURN_POINTER(result);
}


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
complex_abs_cmp_internal(Complex * a, Complex * b)
{
	double		amag = Mag(a),
				bmag = Mag(b);

	if (amag < bmag)
		return -1;
	if (amag > bmag)
		return 1;
	return 0;
}


PG_FUNCTION_INFO_V1(complex_abs_lt);

Datum
complex_abs_lt(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) < 0);
}

PG_FUNCTION_INFO_V1(complex_abs_le);

Datum
complex_abs_le(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) <= 0);
}

PG_FUNCTION_INFO_V1(complex_abs_eq);

Datum
complex_abs_eq(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) == 0);
}

PG_FUNCTION_INFO_V1(complex_abs_ge);

Datum
complex_abs_ge(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) >= 0);
}

PG_FUNCTION_INFO_V1(complex_abs_gt);

Datum
complex_abs_gt(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_BOOL(complex_abs_cmp_internal(a, b) > 0);
}

PG_FUNCTION_INFO_V1(complex_abs_cmp);

Datum
complex_abs_cmp(PG_FUNCTION_ARGS)
{
	Complex    *a = (Complex *) PG_GETARG_POINTER(0);
	Complex    *b = (Complex *) PG_GETARG_POINTER(1);

	PG_RETURN_INT32(complex_abs_cmp_internal(a, b));
}
