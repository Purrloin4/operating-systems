/**
 * \author Cédric Josse
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

/*
 * definition of error codes
 * */
#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)


/*
 * The real definition of struct list / struct node
 */

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;

    void *(*element_copy)(void *src_element);

    void (*element_free)(void **element);

    int (*element_compare)(void *x, void *y);
};


dplist_t *dpl_create(// callback functions
        void *(*element_copy)(void *src_element),
        void (*element_free)(void **element),
        int (*element_compare)(void *x, void *y)
) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_copy = element_copy;
    list->element_free = element_free;
    list->element_compare = element_compare;
    return list;
}

void dpl_free(dplist_t **list, bool free_element) {

    if( *list == NULL) return;
    if (dpl_size(*list) != 0 ){
        dplist_node_t *current_n = (*list)->head;
        dplist_node_t *next_n = current_n->next;
        while(current_n != NULL) {
            if (free_element && current_n->element != NULL) {
                (*list)->element_free(&(current_n->element));
                (current_n->element) = NULL;
            }
            free(current_n);
            current_n = next_n;
            if (next_n != NULL) next_n = next_n->next;
        }
    }

    free(*list);
    *list = NULL;

}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index, bool insert_copy) {

    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));
    DPLIST_ERR_HANDLER(list_node == NULL, DPLIST_MEMORY_ERROR);
    if (insert_copy) {
        list_node->element = list->element_copy(element);
    } else {
        list_node->element = element;
    }
    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1 (als de lijst nog geen node heeft en dus NULL is)
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2 (we inserten vanvoor in de lijst)
        list_node->prev = NULL; //eerste dus vorige op NULL zetten
        list_node->next = list->head; //next wordt die dat eerst was
        list->head->prev = list_node; //we zetten de nieuwe vanvoor
        list->head = list_node; //we zetten de pointer van de lijst naar de nieuwe eerste
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4 (ergens tussenin)
            list_node->prev = ref_at_index->prev; //de oude op onze index komt achter de nieuwe dus moet de prev naar de vorige pointen
            list_node->next = ref_at_index; //de next wordt dus de oude
            ref_at_index->prev->next = list_node; //waar de oude stond nu de nieuwe
            ref_at_index->prev = list_node; // de oude zijn prev moet nu de nieuwe zijn
            // pointer drawing breakpoint
        } else { // covers case 3 (aan het einde toevoegen)
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;


}

dplist_t *dpl_remove_at_index(dplist_t *list, int index, bool free_element) {

    dplist_node_t *ref_at_index, *prev_head, *node;
    if (list == NULL) return NULL;
    node = list ->head;
    if (free_element) {
        list->element_free(&node->element);
    }
    if (list->head == NULL) { // covers case 1
        return list;
    }
    else if (dpl_size(list)==1){
        prev_head = list -> head;
        free(prev_head);
        list -> head = NULL;
        return list;
    }
    else if (index <= 0) { // covers case 2
        prev_head = list->head;
        list->head = prev_head ->next;
        free(prev_head);
        return list;

    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        if (index < dpl_size(list)-1) { // covers case 4

            ref_at_index ->prev ->next = ref_at_index -> next;
            ref_at_index ->next->prev = ref_at_index ->prev;
            free(ref_at_index);



        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            ref_at_index -> prev -> next = NULL;
            free(ref_at_index);

        }
    }
    return list;

}

int dpl_size(dplist_t *list) {

    int size =0;
    if(list == NULL)
    {return -1;}
    if(list -> head != NULL) {
        dplist_node_t *current_n = list->head;
        while (current_n != NULL) {
            size++;
            current_n = current_n->next;
        }
    }
    return size;

}

void *dpl_get_element_at_index(dplist_t *list, int index) {

    /** Returns the list element contained in the list node with index 'index' in the list.
 * - return is not returning a copy of the element with index 'index', i.e. 'element_copy()' is not used.
 * - If 'index' is 0 or negative, the element of the first list node is returned.
 * - If 'index' is bigger than the number of elements in the list, the element of the last list node is returned.
 * - If the list is empty, NULL is returned.
 * - If 'list' is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position of the node for which the element is returned
 * \return a pointer to the element at the given index or NULL
 */
    if (list == NULL) return NULL;
    dplist_node_t *ref_at_index = dpl_get_reference_at_index(list, index);
    if (ref_at_index == NULL) return NULL;
    return ref_at_index->element;

}

int dpl_get_index_of_element(dplist_t *list, void *element) {
    /** Returns an index to the first list node in the list containing 'element'.
 * - the first list node has index 0.
 * - Use 'element_compare()' to search 'element' in the list, a match is found when 'element_compare()' returns 0.
 * - If 'element' is not found in the list, -1 is returned.
 * - If 'list' is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param element the element to look for
 * \return the index of the element that matches 'element'
 */
    if (list == NULL) return -1;
    dplist_node_t *current_n = list->head;
    int index = 0;
    while (current_n != NULL) {
        if (list->element_compare(current_n->element, element) == 0) {
            return index;
        }
        index++;
        current_n = current_n->next;
    }
    return -1;

}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {

    /** Returns a reference to the list node with index 'index' in the list.
 * - If 'index' is 0 or negative, a reference to the first list node is returned.
 * - If 'index' is bigger than the number of list nodes in the list, a reference to the last list node is returned.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * \param list a pointer to the list
 * \param index the position of the node for which the reference is returned
 * \return a pointer to the list node at the given index or NULL
 */
    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    if (index <= 0) return list->head;
    dplist_node_t *current_n = list->head;
    while (current_n->next != NULL && index > 0) {
        current_n = current_n->next;
        index--;
    }
    return current_n;

}

void *dpl_get_element_at_reference(dplist_t *list, dplist_node_t *reference) {

    /** Returns the element contained in the list node with reference 'reference' in the list.
 * - If the list is empty, NULL is returned.
 * - If 'list' is is NULL, NULL is returned.
 * - If 'reference' is NULL, NULL is returned.
 * - If 'reference' is not an existing reference in the list, NULL is returned.
 * \param list a pointer to the list
 * \param reference a pointer to a certain node in the list
 * \return the element contained in the list node or NULL
 */

    if (list == NULL) return NULL;
    if (list->head == NULL) return NULL;
    if (reference == NULL) return NULL;
    dplist_node_t *current_n = list->head;
    while (current_n != NULL) {
        if (current_n == reference) return current_n->element;
        current_n = current_n->next;
    }
    return NULL;
}







