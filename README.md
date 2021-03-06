# About

During the lectures on "Theoretical Computer Science" I have created a turing machine simulator.
The progarm is written in C and can be executed as a command line application.
The turing machine description (i.e. program you want to run) is specified by a text file whose structure is described in the **INPUT** section of the user [manual](#user-manual-extract). 
The ouput will show for each computation step the turing machine configuration and the tape contents.

# Installation

Check out the [releases](https://github.com/mikenoethiger/cturing/releases) for OS specific binaries and installation instructions.

# Compile

Download the the source code (`cturing.c`), compile it e.g. with [gcc](https://gcc.gnu.org/) `gcc -o cturing cturing.c` and run the program `./cturing -h`.

Or clone this repo and run

```bash
$ make
$ out/cturing -h
```

# Examples

The `example` directory contains two turing machine descriptions.
* `tm_description.txt` is a turing machine that accepts words over { 0, 1, # }* if the parts before and after the # are equal. This machine is a decider (i.e. it will halt for all possible inputs).
* `tm_description1.txt` is a turing machine that accepts words over { a }*, whose length modulo 3 is equal to 0. It will reject words if |w| % 3 = 1 and loop if |w| % 3 = 2. This machine isn't a decider.

The output for `cturing < tm_description` is:

```
Q2	  0 }01#01	init config
Q3	  1 x}1#01	(Q2,0) -> (Q3,R,x)
Q3	  2 x1}#01	(Q3,1) -> (Q3,R)
Q5	  3 x1#}01	(Q3,#) -> (Q5,R)
Q7	  4 x1}#x1	(Q5,0) -> (Q7,L,x)
Q8	  5 x}1#x1	(Q7,#) -> (Q8,L)
Q8	  6 }x1#x1	(Q8,1) -> (Q8,L)
Q2	  7 x}1#x1	(Q8,x) -> (Q2,R)
Q4	  8 xx}#x1	(Q2,1) -> (Q4,R,x)
Q6	  9 xx#}x1	(Q4,#) -> (Q6,R)
Q6	 10 xx#x}1	(Q6,x) -> (Q6,R)
Q7	 11 xx#}xx	(Q6,1) -> (Q7,L,x)
Q7	 12 xx}#xx	(Q7,x) -> (Q7,L)
Q8	 13 x}x#xx	(Q7,#) -> (Q8,L)
Q2	 14 xx}#xx	(Q8,x) -> (Q2,R)
Q9	 15 xx#}xx	(Q2,#) -> (Q9,R)
Q9	 16 xx#x}x	(Q9,x) -> (Q9,R)
Q9	 17 xx#xx}_	(Q9,x) -> (Q9,R)
Q_acc	 18 xx#x}x_	(Q9,_) -> (Q0,L)
```

# User Manual Extract

Run `cturing -h` to show the user manual.

        Usage: out/cturing_osx [-h] [-s tape_size] < tm_description.txt
        	-h            show usage
        	-s tape_size  specify a tape size (default 1024)
        
        AUTHORS
        	Mike Nöthiger (noethiger.mike@gmail.com)
        
        DESCRIPTION
        	This is a universal machine written in C that simulates a turing machine (TM).
        	Given a TM description and a word, the universal machine will run this word on the TM.
        
        OUTPUT
        	The verdict over a given input word will either be accept, reject, or loop. The verdicts can be interpreted as follows:
        	 * If the verdict is accept, than the input word is element of the language recognized by TM.
        	 * If the verdict is reject (i.e. TM halts), than the input word is not element of the language recognized by TM.
        	 * If the TM halts for all possible inputs, it shall be called a decider over S* (S being the input alphabet) and S* shall be considered a decidable language.
        	 * As long as the machine runs, the verdict is loop and we remain uncertain about the categorization of the input word.
        	Unfortunately, the universal machine can not make any of the above statements, without running the word on the TM.
        	That is, it can not detect infinite loops in the TM description without running them (at least not in the general case).
        
        	Besides the final verdict, the universal machine will print the machine configuration for each state transitions, from start state to the final verdict.
        	The transitions output can be interpreted as follows:
        		Q9       28 xxx#}xxx    (Q2,#) -> (Q9,R)
        		Q9        := current machine state.
        		28        := current iteration (i.e. number of transitions).
        		xxx#}xxx  := current word on the tape. } denotes the position of the head, which is at the character next to }.
        		(Q2,#)... := the actual state transition.
        
        INPUT
        	A TM is described by a 7-tuple (Q,S,T,d,s,a,r):
        		Q  := finite set of states
        		S  := input alphabet
        		T  := tape alphabet
        		d  := transition function: Q' x T -> Q x T x {L, R}
        		                           Q' = Q\{a,r}
        		                           L='move head left', R='move head right'
        		                           function has to be total, that is it must declare a transition for all (Q' x T) pairs.
        		Qs := start state ∈ Q
        		Qa := accept state ∈ Q
        		Qr := reject state ∈ Q
        		     r != a
        	A simplified TM description must be provided in the tm_description.txt file. The structure of tm_description.txt is as follows:
        		start_state
        		transition_0
        		transition_1
        		...
        		transition_n
        		0
        		input_word
        	Clearly one can see that this is not the complete 7-tuple as described above. But yet all information can be derived from it with some additional assumptions.
        	States are labelled using numbers >= 0. Qs is described in line 1. Qa and Qr are implicitly defined state 0 and 1 respectively.
        	The transition function is defined from line 2 to 2+n and terminated in line 3+n with s single 0. Each transition (i.e. each line) has the following structure:
        		q,s,qt,m[,qs]
        			q  := state number
        			s  := a single character
        			qt := state number
        			m  := L | R (left or right)
        			qs := (optional) a single character, omitting it denotes a transition that doesn't write to head
        	Such a transition can be interpreted as "When TM is in state q, with symbol s at head, go to state qt, write qs to the head, then move head to m."
        	All states in Q are derived from the states mentioned in the transition function (that is q and qt).
        	The input alphabet S is derived from all characters mentioned in the input_word on the last line. The tape alphabet T consists of (S | BLANK), BLANK='_'.
        	Every missing transition will be augmented by the universal machine by going to Qr and moving head to left.
        	Now the 7-tuple is complete.
        
        EXAMPLE
        	Let S be the input alphabet of a TM; S = {0,1,#}*.
        	Let B be a language; B = { w#w | w ∈ {0,1}* }.
        	In the following is a description of a TM with input alphabet S, that decides B (B is decidable because TM exists).
        	To express it in natural language, the TM will check for an input word which consists of 0, 1 and #, whether the parts before and after the # are equal.
        	2
        	2,#,9,R
        	9,x,9,R
        	9,_,0,L
        	2,1,4,R,x
        	4,0,4,R
        	4,1,4,R
        	4,#,6,R
        	6,x,6,R
        	6,1,7,L,x
        	7,x,7,L
        	7,#,8,L
        	8,0,8,L
        	8,1,8,L
        	8,x,2,R
        	2,0,3,R,x
        	3,0,3,R
        	3,1,3,R
        	3,#,5,R
        	5,x,5,R
        	5,0,7,L,x
        	0
        	101#101

# License

Copyright 2019 Mike Nöthiger (noethiger.mike@gmail.com)

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program (COPYING file.) If not, see https://www.gnu.org/licenses/.
