#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLANK '_'
#define MOVE_L 'L'
#define MOVE_R 'R'
#define NO_WRITE '\0'
#define D_TAPE_SITE 1024

/* Program Options */
int o_help = 0;
int o_tape_size = D_TAPE_SITE;
char *prog_name;

struct transition {
	int q;
	int s;
	int qt;
	int st;
	char m;
	struct transition *next;
};

// tape  := the turing machine tape
// t1    := denotes the first transition in the linked list
//          i.e. the beginning of the linked list that contains
//          all transitions
// tl    := last executed transition
// steps := number of steps executed so far
// qc    := current state
// qa    := accept state (fixed as state 0)
// qr    := reject state (fixed as state 1)
// h     := header position on the tape; 0 <= h < TAPE_SIZE
// mw    := max width that the word on the tape ever reached
//          this is used to print the contents of the tape
//          without printing the whole tape
char *tape;
struct transition *t1 = NULL;
struct transition *tl = NULL;
int steps = 0;
int qc;
int qa = 0;
int qr = 1;
int h;
int mw;

struct transition *transition_get(int q, int s) {
	struct transition *t = t1;
	while (t != NULL && (t->q != q || t->s != s)) {
		t = t->next;
	}
	return t;
}

struct transition *transition_create(int q, char s, int qt, char st, char m) {
	struct transition *t = malloc(sizeof(struct transition));
	t->q = q;
	t->s = s;
	t->qt = qt;
	t->st = st;
	t->m = m;
	t->next = NULL;
	return t;
}

void transition_delete(struct transition *t) {
	struct transition *tmp = t->next;
	free(t);
	while (tmp != NULL) {
		t = tmp->next;
		free(tmp);
		tmp = t;
	}
}

void transition_insert(struct transition *t) {
	struct transition *tn = t1;
	if (tn == NULL) {
		t1 = t;
		return;
	}
	while (tn->next != NULL) {
		tn = tn->next;
	}
	tn->next = t;
}

void print_usage(const char *prog_name) {
	printf("Usage: %s [-h] [-s tape_size] < tm_description.txt\n", prog_name);
	printf("\t-h            show usage\n");
	printf("\t-s tape_size  specify a tape size (default %d)\n", D_TAPE_SITE);
	printf("\nAUTHOR\n");
	printf("\tMike Nöthiger (noethiger.mike@gmail.com)\n");
	printf("\nDESCRIPTION\n");
	printf("\tThis is a universal machine written in C that simulates a turing machine (TM).\n");
	printf("\tGiven a TM description and a word, the universal machine will run this word on the TM.\n");
	printf("\nOUTPUT\n");
	printf("\tThe verdict over a given input word will either be accept, reject, or loop. The verdicts can be interpreted as follows:\n");
	printf("\t * If the verdict is accept, than the input word is element of the language recognized by TM.\n");
	printf("\t * If the verdict is accept or reject (i.e. TM halts), than the input word is element of a decidable language.\n");
	printf("\t * If the TM halts for all possible inputs, it shall be called a decider over S* and S* shall be considered a decidable language.\n");
	printf("\t * As long as the machine runs, the verdict is loop and we remain uncertain about the categorization of the input word.\n");
	printf("\tUnfortunately, the universal machine can not make any of the above statements, without running the word on the TM.\n");
	printf("\tThat is, it can not detect infinite loops in the TM description without running them (at least not in the general case).\n");
	printf("\n");
	printf("\tBesides the final verdict, the universal machine will print the machine configuration for each state transitions, from start state to the final verdict.\n");
	printf("\tThe transitions output can be interpreted as follows:\n");
	printf("\t\tQ9       28 xxx#}xxx    (Q2,#) -> (Q9,R)\n");
	printf("\t\tQ9        := current machine state.\n");
	printf("\t\t28        := current iteration (i.e. number of transitions).\n");
	printf("\t\txxx#}xxx  := current word on the tape. } denotes the position of the head, which is at the character next to }.\n");
	printf("\t\t(Q2,#)... := the actual state transition.\n");
	printf("\nINPUT\n");
	printf("\tA TM is described by a 7-tuple (Q,S,T,d,s,a,r):\n");
	printf("\t\tQ  := finite set of states\n");
	printf("\t\tS  := input alphabet\n");
	printf("\t\tT  := tape alphabet\n");
	printf("\t\td  := transition function: Q' x T -> Q x T x {L, R}\n");
	printf("\t\t                           Q' = Q\\{a,r}\n");
	printf("\t\t                           L='move head left', R='move head right'\n");
	printf("\t\t                           function has to be total, that is it must declare a transition for all (Q' x T) pairs.\n");
	printf("\t\tQs := start state ∈ Q\n");
	printf("\t\tQa := accept state ∈ Q\n");
	printf("\t\tQr := reject state ∈ Q\n");
	printf("\t\t     r != a\n");
	printf("\tA simplified TM description must be provided in the tm_description.txt file. The structure of tm_description.txt is as follows:\n");
	printf("\t\tstart_state\n");
	printf("\t\ttransition_0\n");
	printf("\t\ttransition_1\n");
	printf("\t\t...\n");
	printf("\t\ttransition_n\n");
	printf("\t\t0\n");
	printf("\t\tinput_word\n");

	printf("\tClearly one can see that this is not the complete 7-tuple as described above. But yet all information can be derived from it with some additional assumptions.\n");
	printf("\tStates are labelled using numbers >= 0. Qs is described in line 1. Qa and Qr are implicitly defined state 0 and 1 respectively.\n");
	printf("\tThe transition function is defined from line 2 to 2+n and terminated in line 3+n with s single 0. Each transition (i.e. each line) has the following structure:\n");
	printf("\t\tq,s,qt,m[,qs]\n");
	printf("\t\t\tq  := state number\n");
	printf("\t\t\ts  := a single character\n");
	printf("\t\t\tqt := state number\n");
	printf("\t\t\tm  := L | R (left or right)\n");
	printf("\t\t\tqs := (optional) a single character, omitting it denotes a transition that doesn't write to head\n");
	printf("\tSuch a transition can be interpreted as \"When TM is in state q, with symbol s at head, go to state qt, write qs to the head, then move head to m.\"\n");
	printf("\tAll states in Q are derived from the states mentioned in the transition function (that is q and qt).\n");
	printf("\tThe input alphabet S is derived from all characters mentioned in the input_word on the last line. The tape alphabet T consists of (S | BLANK), BLANK='%c'.\n", BLANK);
	printf("\tEvery missing transition will be augmented by the universal machine by going to Qr and moving head to left.\n");
	printf("\tNow the 7-tuple is complete.\n");
	printf("\nEXAMPLE\n");
	printf("\tLet A be a language; A = {0,1,#}*.\n");
	printf("\tLet B be a language; B = { w#w | w ∈ {0,1}* }.\n");
	printf("\tIn the following is a description of a TM that recognizes B and decides A. To express it in natural language, the TM will check for all words that consist of 0, 1 and #, whether the parts before and after the # are equal.\n");
	printf("\t2\n");
	printf("\t2,#,9,R\n");
	printf("\t9,x,9,R\n");
	printf("\t9,_,0,L\n");
	printf("\t2,1,4,R,x\n");
	printf("\t4,0,4,R\n");
	printf("\t4,1,4,R\n");
	printf("\t4,#,6,R\n");
	printf("\t6,x,6,R\n");
	printf("\t6,1,7,L,x\n");
	printf("\t7,x,7,L\n");
	printf("\t7,#,8,L\n");
	printf("\t8,0,8,L\n");
	printf("\t8,1,8,L\n");
	printf("\t8,x,2,R\n");
	printf("\t2,0,3,R,x\n");
	printf("\t3,0,3,R\n");
	printf("\t3,1,3,R\n");
	printf("\t3,#,5,R\n");
	printf("\t5,x,5,R\n");
	printf("\t5,0,7,L,x\n");
	printf("\t0\n");
	printf("\t101#101\n");
}

void print_usage_error(char *msg, char *prog_name) {
	printf("%s\nShow Usage: %s -h", msg, prog_name);
	exit(EXIT_FAILURE);
}

void print_configuration() {
	if (qc == qa) printf("Q_acc\t%3d ", steps);
	else if (qc == qr) printf("Q_rej\t%3d ", steps);
	else printf("Q%d\t%3d ", qc, steps);

	int i = 0;
	for (i = 0; i < mw; i++) {
		if (i == h) printf("}");
		if (tape[i] != BLANK) printf("%c", tape[i]);
		else printf("%c", BLANK);
	}
}

void print_transition() {
	if (tl == NULL) return;
	if (tl->st != NO_WRITE) printf("(Q%d,%c) -> (Q%d,%c,%c)", tl->q, tl->s, tl->qt, tl->m, tl->st);
	else printf("(Q%d,%c) -> (Q%d,%c)", tl->q, tl->s, tl->qt, tl->m);
}

/**
 * Write a symbol to current head position.
 *
 * @param s the symbol to write or NO_WRITE if
 *          nothing shall be written to the tape.
 */
void write_head(char s) {
	if (s != NO_WRITE) {
		tape[h] = s;
	}
}

/**
 * Move header position to left (MOVE_L) or
 * right (MOVE_R). If the header is at the
 * first position, a MOVE_L will have no
 * effect. If it is at the last position
 * a MOVE_R will produce an error.
 *
 * Using another move character than MOVE_L
 * or MOVE_R will produce an error.
 *
 * @param m move direction (MOVE_L | MOVE_R)
 */
void move_head(char m) {
	if (m == MOVE_L && h > 0) h--;
	else if (m == MOVE_R) {
		if (h + 1 < o_tape_size) {
			h++;
			if (h == mw) mw++;
		} else {
			printf("tape too small\n");
			exit(EXIT_FAILURE);
		}
	} else {
		printf("invalid move '%c'\n", m);
		exit(EXIT_FAILURE);
	}
}

void step() {
	steps++;
	tl = transition_get(qc, tape[h]);
	// if transition not defined, go to qr and move left
	if (tl == NULL) {
		qc = qr;
		move_head(MOVE_L);
		return;
	}

	write_head(tl->st);
	move_head(tl->m);
	qc = tl->qt;
}

void read_machine() {
	char buf, m, s, st;
	int q, qt, i;
	struct transition *t;

	h = 0;
	scanf("%d", &qc);
	while (scanf("%d", &q) && q != 0) {
		scanf(",%c,%d,%c", &s, &qt, &m);
		scanf("%c", &buf);
		if (buf == ',') {
			scanf("%c", &st);
		} else {
			st = NO_WRITE;
		}
		transition_insert(transition_create(q, s, qt, st, m));
	}

	scanf("%c%s", &buf, tape);
	mw = strlen(tape);
	// override terminating null byte '\0'
	tape[mw] = BLANK;
}

void init_tape() {
	int i;
	for (i = 0; i < o_tape_size; i++) {
		tape[i] = BLANK;
	}
}

void parse_opt(int argc, char *argv[]) {
	int opt;
	while ((opt = getopt(argc, argv, "hs:")) != -1) {
		switch (opt) {
			case 'h':
				o_help = 1;
				break;
			case 's':
				o_tape_size = atoi(optarg);
				break;
			default:
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char *argv[]) {
	prog_name = argv[0];
	parse_opt(argc, argv);
	if (o_help) {
		print_usage(argv[0]);
		return 0;
	}
	tape = malloc(o_tape_size* sizeof(char));
	init_tape();
	read_machine();

	print_configuration();
	printf("\tinit config\n");
	//run the machine until it accepts or rejects
	while (qc != qa && qc != qr) {
		step();
		print_configuration();
		printf("\t");
		print_transition();
		printf("\n");
	}

	printf("Show Usage: %s -h\n", argv[0]);

	transition_delete(t1);
	free(tape);

	return 0;
}