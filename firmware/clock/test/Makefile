# Host tests: no hardware needed. `make test`
CORE = ../components/clockcore
CC ?= cc
CFLAGS = -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -I$(CORE)/include -I.
SRC = $(CORE)/civil.c $(CORE)/tz.c $(CORE)/sched.c $(CORE)/alarm_sm.c $(CORE)/dawn.c $(CORE)/storage.c test_main.c

test: expected_tz.inc
	$(CC) $(CFLAGS) $(SRC) -lm -o clock_tests
	./clock_tests
expected_tz.inc: gen_expected_tz.py
	python3 gen_expected_tz.py
clean:
	rm -f clock_tests
