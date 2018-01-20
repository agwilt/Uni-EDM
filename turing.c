#include <stdio.h>
#include <stdlib.h>

#include "turing.h"

struct turing_machine *tm_from_file(const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "Error: Can't open file.\n");
		exit(1);
	}

	struct turing_machine *tm = malloc(sizeof(struct turing_machine));
	
	if (fscanf(fp, "%d\n", &(tm->N)) != 1) {
		fprintf(stderr, "Error: Invalid file format.\n");
		exit(1);
	}
	if (fscanf(fp, "%d\n", &(tm->_num_values)) != 1) {
		fprintf(stderr, "Error: Invalid file format.\n");
		exit(1);
	}

	tm->phi = malloc((tm->N+1) * sizeof(struct transition*));
	for (int i=0; i<=tm->N; ++i) {
		tm->phi[i] = malloc(((tm->_num_values + 1) * sizeof(struct transition)));
	}

	int old_state, old_entry, new_state, new_entry, move;
	while (fscanf(fp, "%d %d %d %d %d\n", &old_state, &old_entry, &new_state, &new_entry, &move) == 5) {
		if (old_state>=0 && old_state<=tm->N && old_entry>=-1 && old_entry<tm->_num_values && new_state >= -1 && new_state <=tm->N && new_entry>=-1 && new_entry<tm->_num_values && move>=-1 && move<=1) {
			tm->phi[old_state][old_entry+1] = (struct transition) {.state=new_state, .val=new_entry, .move = move};
		}
	}

	tm->state = 0;
	tm->head = tm->_zero_cell = calloc(sizeof(struct cell), 1);
	tm->head->val = -1;

	fclose(fp);

	return tm;
}

void move_head(struct turing_machine *tm, char move)
{
	switch (move) {
		case 0:
			return;
		case 1:
			if (tm->head->next == NULL) {
				tm->head->next = malloc(sizeof(struct cell));
				tm->head->next->prev = tm->head;
				tm->head = tm->head->next;
				tm->head->next = NULL;
				tm->head->val = -1;
			} else {
				tm->head = tm->head->next;
			}
			return;
		case -1:
			if (tm->head->prev == NULL) {
				tm->head->prev = malloc(sizeof(struct cell));
				tm->head->prev->next = tm->head;
				tm->head = tm->head->prev;
				tm->head->prev = NULL;
				tm->head->val = -1;
			} else {
				tm->head = tm->head->prev;
			}
			return;
		default:
			printf("Invalid move: %d\n", move);
			exit(2);
	}
}

void tm_run(struct turing_machine *tm, size_t input_len, int *input)
{
	/* First copy over input */
	tm->head = tm->_zero_cell;
	for (unsigned int i=0; i<input_len; ++i) {
		tm->head->val = input[i];
		move_head(tm, 1);
	}
	tm->head->val = -1;
	tm->head = tm->_zero_cell;

	/* Run machine */
	struct transition *trans;
	while (tm->state != -1) {
		trans = tm->phi[tm->state]+(tm->head->val+1);
		tm->state = trans->state;
		tm->head->val = trans->val;
		move_head(tm, trans->move);
	}

	/* Now output tape */
	for (struct cell *head = tm->_zero_cell; head!=NULL && head->val!=-1; head=head->next)
		printf("%d", head->val);
	printf("\n");

	return;
}

int main(int argc, char *argv[])
{
	if (argc <= 1) return 1;

	struct turing_machine *tm = tm_from_file(argv[1]);
	int size = argc - 2;
	int *input = malloc(size * sizeof(int));
	for (int i=0; i<size; ++i) input[i] = atoi(argv[i+2]);

	tm_run(tm, size, input);
}
