#ifndef _TURING_H
#define _TURING_H

/* States: -1 for end, else between 0 and N
 * Values: -1, ... ,_num_values-1
 */

struct turing_machine {
	int N;	/* maximum state */
	int state;
	int _num_values;
	struct transition **phi;	/* transitions */
	struct cell *head;	/* initialized to 0 */

	struct cell *_zero_cell;
};

struct transition {
	int state;
	int val;
	char move;
};

struct cell {
	int val;
	struct cell *prev;
	struct cell *next;
};

struct turing_machine *tm_from_file(const char *filename);

void tm_run(struct turing_machine *tm, size_t input_len, int *input);

#endif
