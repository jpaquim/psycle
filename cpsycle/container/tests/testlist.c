// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/miunit.h"
#include "../../detail/stdint.h"

#include "../src/list.h"
#include <stdio.h>

int test_run = 0;

static char * test_list_create(void)
{
	List* p;
	
	p = list_create(0);
	mu_assert("error, list create returns NULL", p != 0);
	mu_assert("error, wrong tail", p->tail == p);
	list_free(p);
	return 0;
}

static char * test_list_createfree(void)
{		
	List* p;
	
	p = list_create(0);
	mu_assert("error, list create returns NULL", p != 0);
	mu_assert("error, wrong tail", p->tail == p);
	list_free(p);
	return 0;
}

static char * test_list_read_entry(void)
{
	List* p;
	
	p = list_create((void*)(uintptr_t)10);
	mu_assert("error, list create returns NULL", p != 0);	
	mu_assert("error, wrong tail", p->tail == p);
	mu_assert("error, entry != 10", (uintptr_t) p->entry == 10);
	list_free(p);
	return 0;
}

static char * test_list_append(void)
{
	uintptr_t value;
	List* root;
	List* p;
	List* q;
	
	p = q = root = list_create((void*)(uintptr_t)10);
	mu_assert("error, list create returns NULL", p != 0);
	mu_assert("error, wrong tail", root->tail == p);
	mu_assert("error, entry != 10", ((uintptr_t) p->entry) == 10);	
	p = list_append(&root, (void*)(uintptr_t)20);
	mu_assert("error, root modified", q == root);
	mu_assert("error, new node == root", p != root);
	mu_assert("error, wrong tail", root->tail == p);
	mu_assert("error, entry != 20", ((uintptr_t) p->entry) == 20);
	list_free(root);
	return 0;
}

static char * test_list_append_entries(void)
{
	uintptr_t value;
	List* root;
	List* p;
	List* q;	
	
	p = q = root = list_create((void*)(uintptr_t)1);
	mu_assert("error, list create returns NULL", p != 0);
	mu_assert("error, wrong tail", root->tail == p);
	mu_assert("error, entry != 1", ((uintptr_t) p->entry) == 1);	
	for (value = 2; value <= 10; ++value) {		
		p = list_append(&root, (void*)(uintptr_t)value);
		mu_assert("error, root modified", q == root);
		mu_assert("error, new node == root", p != root);
		mu_assert("error, wrong tail", root->tail == p);
		mu_assert("error, wrong value", ((uintptr_t) p->entry) == value);
	}
	p = root;
	value = 1;
	while (p != 0) {
		mu_assert("error, wrong value", ((uintptr_t) p->entry) == value);
		p = p->next;
		++value;
	}
	mu_assert("error, wrong number of elements", value == 11);
	p = root->tail;
	value = 10;
	while (p != 0) {
		mu_assert("error, wrong value", ((uintptr_t) p->entry) == value);
		p = p->prev;
		--value;
	}
	mu_assert("error, wrong number of elements", value == 0);

	list_free(root);
	return 0;
}

static char * test_list_size(void)
{
	uintptr_t value;
	List* root;
	List* p;
	List* q;	
	
	p = q = root = list_create((void*)(uintptr_t)1);
	mu_assert("error, list create returns NULL", p != 0);
	mu_assert("error, wrong tail", root->tail == p);
	mu_assert("error, entry != 1", ((uintptr_t) p->entry) == 1);	
	for (value = 2; value <= 10; ++value) {		
		p = list_append(&root, (void*)(uintptr_t)value);
		mu_assert("error, root modified", q == root);
		mu_assert("error, new node == root", p != root);
		mu_assert("error, wrong tail", root->tail == p);
		mu_assert("error, wrong value", ((uintptr_t) p->entry) == value);
	}
	mu_assert("error, wrong size != 10", list_size(root) == 10);
	list_free(root);
	return 0;
}

static char * all_tests(void)
{
	printf("Testing list.c\n\n");
	mu_run_test(test_list_create);
	mu_run_test(test_list_createfree);
	mu_run_test(test_list_read_entry);
	mu_run_test(test_list_append);
	mu_run_test(test_list_append_entries);
	mu_run_test(test_list_size);
	return 0;
}

int main(int argc, char **argv)
{
	char* result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	} else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", test_run);

	return result != 0;
}
