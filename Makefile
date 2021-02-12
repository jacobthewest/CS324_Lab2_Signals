# Makefile for the CS:APP Shell Lab

VERSION = 1
DRIVER = ./sdriver.pl
TESTDRIVER = ./checktsh.pl
TSH = ./tsh
TSHREF = ./tshref
TSHARGS = "-p"
CC = gcc
CFLAGS = -Wall -O2
FILES = $(TSH) ./myspin ./mysplit ./mystop ./myint ./myintgroup ./myppid

all: $(FILES)

##################
# Regression tests
##################

# Compare output from student shell and reference shell
testall1:
	$(TESTDRIVER) 1
testall2:
	$(TESTDRIVER) 2
test01:
	$(TESTDRIVER) -v -t trace01.txt
test02:
	$(TESTDRIVER) -v -t trace02.txt
test03:
	$(TESTDRIVER) -v -t trace03.txt
test04:
	$(TESTDRIVER) -v -t trace04.txt
test05:
	$(TESTDRIVER) -v -t trace05.txt
test06:
	$(TESTDRIVER) -v -t trace06.txt
test07:
	$(TESTDRIVER) -v -t trace07.txt
test08:
	$(TESTDRIVER) -v -t trace08.txt
test09:
	$(TESTDRIVER) -v -t trace09.txt
test10:
	$(TESTDRIVER) -v -t trace10.txt
test11:
	$(TESTDRIVER) -v -t trace11.txt
test12:
	$(TESTDRIVER) -v -t trace12.txt
test13:
	$(TESTDRIVER) -v -t trace13.txt
test14:
	$(TESTDRIVER) -v -t trace14.txt
test15:
	$(TESTDRIVER) -v -t trace15.txt
test16:
	$(TESTDRIVER) -v -t trace16.txt
test34:
	$(TESTDRIVER) -v -t trace34.txt
test35:
	$(TESTDRIVER) -v -t trace35.txt
test36:
	$(TESTDRIVER) -v -t trace36.txt
test37:
	$(TESTDRIVER) -v -t trace37.txt
test38:
	$(TESTDRIVER) -v -t trace38.txt
test39:
	$(TESTDRIVER) -v -t trace39.txt
test40:
	$(TESTDRIVER) -v -t trace40.txt
test41:
	$(TESTDRIVER) -v -t trace41.txt
test42:
	$(TESTDRIVER) -v -t trace42.txt

# Run tests using the student's shell program
stest01:
	$(DRIVER) -t trace01.txt -s $(TSH) -a $(TSHARGS)
stest02:
	$(DRIVER) -t trace02.txt -s $(TSH) -a $(TSHARGS)
stest03:
	$(DRIVER) -t trace03.txt -s $(TSH) -a $(TSHARGS)
stest04:
	$(DRIVER) -t trace04.txt -s $(TSH) -a $(TSHARGS)
stest05:
	$(DRIVER) -t trace05.txt -s $(TSH) -a $(TSHARGS)
stest06:
	$(DRIVER) -t trace06.txt -s $(TSH) -a $(TSHARGS)
stest07:
	$(DRIVER) -t trace07.txt -s $(TSH) -a $(TSHARGS)
stest08:
	$(DRIVER) -t trace08.txt -s $(TSH) -a $(TSHARGS)
stest09:
	$(DRIVER) -t trace09.txt -s $(TSH) -a $(TSHARGS)
stest10:
	$(DRIVER) -t trace10.txt -s $(TSH) -a $(TSHARGS)
stest11:
	$(DRIVER) -t trace11.txt -s $(TSH) -a $(TSHARGS)
stest12:
	$(DRIVER) -t trace12.txt -s $(TSH) -a $(TSHARGS)
stest13:
	$(DRIVER) -t trace13.txt -s $(TSH) -a $(TSHARGS)
stest14:
	$(DRIVER) -t trace14.txt -s $(TSH) -a $(TSHARGS)
stest15:
	$(DRIVER) -t trace15.txt -s $(TSH) -a $(TSHARGS)
stest16:
	$(DRIVER) -t trace16.txt -s $(TSH) -a $(TSHARGS)
stest34:
	$(DRIVER) -t trace34.txt -s $(TSH) -a $(TSHARGS)
stest35:
	$(DRIVER) -t trace35.txt -s $(TSH) -a $(TSHARGS)
stest36:
	$(DRIVER) -t trace36.txt -s $(TSH) -a $(TSHARGS)
stest37:
	$(DRIVER) -t trace37.txt -s $(TSH) -a $(TSHARGS)
stest38:
	$(DRIVER) -t trace38.txt -s $(TSH) -a $(TSHARGS)
stest39:
	$(DRIVER) -t trace39.txt -s $(TSH) -a $(TSHARGS)
stest40:
	$(DRIVER) -t trace40.txt -s $(TSH) -a $(TSHARGS)
stest41:
	$(DRIVER) -t trace41.txt -s $(TSH) -a $(TSHARGS)
stest42:
	$(DRIVER) -t trace42.txt -s $(TSH) -a $(TSHARGS)

# Run the tests using the reference shell program
rtest01:
	$(DRIVER) -t trace01.txt -s $(TSHREF) -a $(TSHARGS)
rtest02:
	$(DRIVER) -t trace02.txt -s $(TSHREF) -a $(TSHARGS)
rtest03:
	$(DRIVER) -t trace03.txt -s $(TSHREF) -a $(TSHARGS)
rtest04:
	$(DRIVER) -t trace04.txt -s $(TSHREF) -a $(TSHARGS)
rtest05:
	$(DRIVER) -t trace05.txt -s $(TSHREF) -a $(TSHARGS)
rtest06:
	$(DRIVER) -t trace06.txt -s $(TSHREF) -a $(TSHARGS)
rtest07:
	$(DRIVER) -t trace07.txt -s $(TSHREF) -a $(TSHARGS)
rtest08:
	$(DRIVER) -t trace08.txt -s $(TSHREF) -a $(TSHARGS)
rtest09:
	$(DRIVER) -t trace09.txt -s $(TSHREF) -a $(TSHARGS)
rtest10:
	$(DRIVER) -t trace10.txt -s $(TSHREF) -a $(TSHARGS)
rtest11:
	$(DRIVER) -t trace11.txt -s $(TSHREF) -a $(TSHARGS)
rtest12:
	$(DRIVER) -t trace12.txt -s $(TSHREF) -a $(TSHARGS)
rtest13:
	$(DRIVER) -t trace13.txt -s $(TSHREF) -a $(TSHARGS)
rtest14:
	$(DRIVER) -t trace14.txt -s $(TSHREF) -a $(TSHARGS)
rtest15:
	$(DRIVER) -t trace15.txt -s $(TSHREF) -a $(TSHARGS)
rtest16:
	$(DRIVER) -t trace16.txt -s $(TSHREF) -a $(TSHARGS)
rtest34:
	$(DRIVER) -t trace34.txt -s $(TSHREF) -a $(TSHARGS)
rtest35:
	$(DRIVER) -t trace35.txt -s $(TSHREF) -a $(TSHARGS)
rtest36:
	$(DRIVER) -t trace36.txt -s $(TSHREF) -a $(TSHARGS)
rtest37:
	$(DRIVER) -t trace37.txt -s $(TSHREF) -a $(TSHARGS)
rtest38:
	$(DRIVER) -t trace38.txt -s $(TSHREF) -a $(TSHARGS)
rtest39:
	$(DRIVER) -t trace39.txt -s $(TSHREF) -a $(TSHARGS)
rtest40:
	$(DRIVER) -t trace40.txt -s $(TSHREF) -a $(TSHARGS)
rtest41:
	$(DRIVER) -t trace41.txt -s $(TSHREF) -a $(TSHARGS)
rtest42:
	$(DRIVER) -t trace42.txt -s $(TSHREF) -a $(TSHARGS)


# clean up
clean:
	rm -f $(FILES) *.o *~


