#include <stdlib.h>
#include "queue.h"

static struct node {
	char* cmd;
	char* out;
	int flags;
	struct node *next;
} *head, *tail;

int queue_init(void) {
	//Checke nicht ganz was hier eigentlich gemacht werden sollte
	/* ==> initialisiere eine leere queue; also würde ich sagen, dass passt so */
	head = NULL;
	tail = NULL;
    return 0;
}

void queue_deinit(void) {
    while(tail != NULL)
    {
		struct node *tmp = tail;
		tail = tail->next;
		free(tmp);
	}
}

int queue_put(char *cmd, char *out, int flags) {
    struct node *new_node = malloc(sizeof(struct node));
    if(new_node == NULL)
		return -1;
	new_node->cmd = cmd;
	new_node->out = out;
	new_node->flags = flags;
	new_node->next = NULL;
	
	if(tail != NULL)
	{
		tail->next = new_node;
	}
	
	tail = new_node;
	
	if(head == NULL) {
		head = new_node;
	}
    return 0;
}

int queue_get(char **cmd, char **out, int *flags) {
    if(head == NULL)
		return -1;
	struct node* tmp = head;
	*cmd = tmp->cmd;
	*out = tmp->out;
	*flags = tmp->flags;
	
	head = head->next;
	if(head == NULL)
		tail = NULL;
	free(tmp);
    return 0;
}
