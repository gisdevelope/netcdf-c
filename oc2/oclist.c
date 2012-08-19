/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "oclist.h"

int oclistnull(void* e) {return e == NULL;}

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define DEFAULTALLOC 16
#define ALLOCINCR 16

OClist* oclistnew(void)
{
  OClist* l;
/*
  if(!ocinitialized) {
    memset((void*)&ocDATANULL,0,sizeof(void*));
    ocinitialized = 1;
  }
*/
  l = (OClist*)malloc(sizeof(OClist));
  if(l) {
    l->alloc=0;
    l->length=0;
    l->content=NULL;
  }
  return l;
}

int
oclistfree(OClist* l)
{
  if(l) {
    l->alloc = 0;
    if(l->content != NULL) {free(l->content); l->content = NULL;}
    free(l);
  }
  return TRUE;
}

int
oclistsetalloc(OClist* l, unsigned long sz)
{
  void** newcontent;
  if(l == NULL) return FALSE;
  if(sz <= 0) {sz = (l->length?2*l->length:DEFAULTALLOC);}
  if(l->alloc >= sz) {return TRUE;}
  newcontent=(void**)calloc(sz,sizeof(void*));
  if(l->alloc > 0 && l->length > 0 && l->content != NULL) {
    memcpy((void*)newcontent,(void*)l->content,sizeof(void*)*l->length);
  }
  if(l->content != NULL) free(l->content);
  l->content=newcontent;
  l->alloc=sz;
  return TRUE;
}

int
oclistsetlength(OClist* l, unsigned long sz)
{
  if(l == NULL) return FALSE;
  if(sz > l->alloc && !oclistsetalloc(l,sz)) return FALSE;
  l->length = sz;
  return TRUE;
}

void*
oclistget(OClist* l, unsigned long index)
{
  if(l == NULL || l->length == 0) return NULL;
  if(index >= l->length) return NULL;
  return l->content[index];
}

int
oclistset(OClist* l, unsigned long index, void* elem)
{
  if(l == NULL) return FALSE;
  if(index >= l->length) return FALSE;
  l->content[index] = elem;
  return TRUE;
}

/* Insert at position i of l; will push up elements i..|seq|. */
int
oclistinsert(OClist* l, unsigned long index, void* elem)
{
  int i; /* do not make unsigned */
  if(l == NULL) return FALSE;
  if(index > l->length) return FALSE;
  oclistsetalloc(l,0);
  for(i=(int)l->length;i>index;i--) l->content[i] = l->content[i-1];
  l->content[index] = elem;
  l->length++;
  return TRUE;
}

int
oclistpush(OClist* l, void* elem)
{
  if(l == NULL) return FALSE;
  if(l->length >= l->alloc) oclistsetalloc(l,0);
  l->content[l->length] = elem;
  l->length++;
  return TRUE;
}

void*
oclistpop(OClist* l)
{
  if(l == NULL || l->length == 0) return NULL;
  l->length--;  
  return l->content[l->length];
}

void*
oclisttop(OClist* l)
{
  if(l == NULL || l->length == 0) return NULL;
  return l->content[l->length - 1];
}

void*
oclistremove(OClist* l, unsigned long i)
{
  unsigned long len;
  void* elem;
  if(l == NULL || (len=l->length) == 0) return NULL;
  if(i >= len) return NULL;
  elem = l->content[i];
  for(i+=1;i<len;i++) l->content[i-1] = l->content[i];
  l->length--;
  return elem;  
}

/* Duplicate and return the content (null terminate) */
void**
oclistdup(OClist* l)
{
    void** result = (void**)malloc(sizeof(void*)*(l->length+1));
    memcpy((void*)result,(void*)l->content,sizeof(void*)*l->length);
    result[l->length] = (void*)0;
    return result;
}

int
oclistcontains(OClist* list, void* elem)
{
    unsigned long i;
    for(i=0;i<oclistlength(list);i++) {
	if(elem == oclistget(list,i)) return 1;
    }
    return 0;
}

/* Remove element by value; only removes first encountered */
int
oclistelemremove(OClist* l, void* elem)
{
  unsigned long len;
  unsigned long i;
  int found = 0;
  if(l == NULL || (len=l->length) == 0) return 0;
  for(i=0;i<oclistlength(l);i++) {
    void* candidate = l->content[i];
    if(elem == candidate) {
      for(i+=1;i<len;i++) l->content[i-1] = l->content[i];
      l->length--;
      found = 1;
      break;
    }
  }
  return found;
}




/* Extends oclist to include a unique operator 
   which remove duplicate values; NULL values removed
   return value is always 1.
*/

int
oclistunique(OClist* list)
{
    unsigned long i,j,k,len;
    void** content;
    if(list == NULL || list->length == 0) return 1;
    len = list->length;
    content = list->content;
    for(i=0;i<len;i++) {
        for(j=i+1;j<len;j++) {
	    if(content[i] == content[j]) {
		/* compress out jth element */
                for(k=j+1;k<len;k++) content[k-1] = content[k];	
		len--;
	    }
	}
    }
    list->length = len;
    return 1;
}

OClist*
oclistclone(OClist* list)
{
    OClist* clone = oclistnew();
    *clone = *list;
    clone->content = oclistdup(list);
    return clone;
}
