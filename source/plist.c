/*
 *
 *  TOTP: Time-Based One-Time Password Algorithm
 *  Copyright (c) 2017, fmount <fmount9@autistici.org>
 *
 *  This software is distributed under MIT License
 *
 *  Compute the hmac using openssl library.
 *  SHA-1 engine is used by default, but you can pass another one,
 *
 *  e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plist.h"

size_t get_len(NODE *head)
{
    NODE *cur = NULL;
    cur = head;
    size_t length = 0;

    while (cur != NULL) {
        cur = cur->next;
        length++;
    }
    return length;
}

bool exists(NODE *head, NODE *target)
{
    printf("Check if the target node exists in list\n");

    NODE *cur = NULL;
    cur = head;
    while (cur != NULL) {
        if ((cur->p)->pname == (target->p)->pname)
            return 1;
        cur = cur->next;
    }
    return 0;
}

NODE *get_node(NODE *head, char *pname)
{
    NODE *cur = NULL;
    cur = head;
    while (cur != NULL) {
        if ((cur->p)->pname == pname) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

int update_value(NODE **head, char *pname, uint32_t optvalue)
{
    NODE *cur;
    cur = *head;
    uint32_t *x = &optvalue;
    while (cur != NULL) {
        if ((cur->p)->pname == pname) {
            (cur->p)->otpvalue = *x;
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

void push(NODE **head, char *pname, char *psecret, uint32_t *otpvalue)
{
    NODE *cur = (NODE *) malloc(sizeof(NODE));
    PROVIDER *p = (PROVIDER *) malloc(sizeof(PROVIDER));

    p->pname = pname;
    p->psecret = psecret;
    p->otpvalue = otpvalue;

    cur->p = p;

    cur->next = *head;
    *head = cur;
}

NODE *pop(NODE **head)
{
    NODE *tmp = *head;
    *head = (*head)->next;
    return tmp;
}

void del(char *del, NODE *head)
{
    if(head == NULL)
        fprintf(stderr, "No valid list, no head found\n");

    NODE *cur = NULL;
    NODE *prev = NULL;

    cur = prev = head;

    while(cur != NULL && (strcmp((cur->p)->pname, del) != 0)) {
        prev = cur;
        cur = cur->next;
    }
    // Reached the end, should return ..
    if(cur == NULL)
        return;
    /* Found the pname in the list, free the node and
     * modify the pointer to next
     */
    prev->next = cur->next;
    free(cur);
}

void freeProvider(PROVIDER *p)
{
    free(p->pname);
    free(p->psecret);
    free((PROVIDER *)p);
}

void freeList(NODE *head)
{
   NODE *tmp;

   while (head != NULL) {
       tmp = head;
       #ifdef DEBUG
       printf("Deleting Provider %s\n", (tmp->p)->pname);
       #endif
       freeProvider(tmp->p);
       head = head->next;
       free(tmp);
   }
}
