
NAME = writeup
EXTRADEPS = regularmodel.eps highnotes.eps
BIBFILE = piano.bib

ifneq ($(BIBFILE),)
EXTRADEPS += $(NAME).bbl
endif

$(NAME).pdf: $(NAME).dvi
	dvipdf $^

$(NAME).dvi: $(NAME).aux $(EXTRADEPS)
	latex $(NAME).tex
	latex $(NAME).tex

$(NAME).aux: $(NAME).tex $(BIBFILE)
	latex $(NAME).tex

ifneq ($(BIBFILE),)
$(NAME).bbl: $(NAME).aux $(BIBFILE)
	bibtex $(NAME)
endif

.PHONY: clean run
clean:
	-rm $(NAME).log $(NAME).aux $(NAME).dvi $(NAME).pdf $(NAME).bbl *~

view: $(NAME).pdf
	evince $^
