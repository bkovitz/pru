LAT = latex -shell-escape
#TEX = pdflatex --shell-escape
TEXINPUTS=.:./sty:
TEX = TEXINPUTS=.:./sty: latexmk -pdf -pdflatex='pdflatex --shell-escape %O %S'
BIB = bibtex8
TEXFILES = $(wildcard *.tex)
PDFFILES = $(TEXFILES:.tex=.pdf)
BIBFILES = $(wildcard *.bib)
DOT = dot -Tpdf

all: arm-pru.pdf

$(PDFFILES): $(BIBFILES)
%.pdf: %.tex
	$(TEX) $<

%.pdf: %.dot
	$(DOT) < $< >$@

clean: commitclean
	rm -f arm-pru.pdf

# Clean only files that should not be committed to repository.
commitclean:
	rm -f *.aux *.fls *.log *.toc *.fdb_latexmk arm-pru.pdf

.PHONY: clean commitclean
