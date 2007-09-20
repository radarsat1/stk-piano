
.PHONY: clean

NAME = piano
DIAFILES = 

all: $(NAME).pdf

clean:
	-rm $(NAME).aux $(NAME).pdf $(NAME).dvi $(NAME).log \
		$(NAME).out $(DIAFILES:%.dia=%.eps) $(NAME).bbl \
		$(NAME).blg $(NAME).tex *~ >/dev/null

$(NAME).pdf: $(NAME).dvi
	dvipdf $^

$(NAME).dvi: $(NAME).tex #$(DIAFILES:%.dia=%.eps) $(NAME).bbl
	latex $^
#	latex $^

#$(NAME).bbl: $(NAME).bib
#	latex $(NAME).tex
#	bibtex $(NAME)

$(NAME).tex: $(NAME).txt
	python txt2prosper.py $^ $@

view: $(NAME).pdf
	evince $(NAME).pdf
