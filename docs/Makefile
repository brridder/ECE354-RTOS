
include ../Makefile.inc
SOURCE=p1.tex p2a.tex p2b.tex p3.tex p4.tex sdd.tex
all: sdd

sdd:
	$(ECHO) Making sdd.pdf
	$(LATEX) sdd.tex
	$(ECHO) Running again for references
	$(LATEX) sdd.tex
	$(ECHO) Done

p1:
	$(ECHO) Making p1.pdf
	$(LATEX) p1.tex
	$(ECHO) Done

p2a:
	$(ECHO) Making p2a.pdf
	$(LATEX) p2a.tex
	$(ECHO) Done

p2b:
	$(ECHO) Making p2b.pdf
	$(LATEX) p2b.tex
	$(ECHO) Done

p3:
	$(ECHO) Making p3.pdf
	$(LATEX) p3.tex
	$(ECHO) Done

p4:
	$(ECHO) Making p4.pdf
	$(LATEX) p4.tex
	$(ECHO) Done

everything: p1 p2a p2b p3 p4 sdd
	
	
.PHONY: clean

clean:
	rm -f *.aux *.log *.pdf

	
