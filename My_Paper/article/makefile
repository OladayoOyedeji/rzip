OPEN = xdg-open
PDF = pdflatex --shell-escape $@.tex; pdflatex --shell-escape $@.tex; pdflatex --shell-escape $@.tex; \
	makeindex $@.idx; \
        pdflatex --shell-escape $@.tex; \
        $(OPEN) $@.pdf; 
LATEXRM = rm -f \
        main.aux main.ilg main.log main.idx main.ind main.out \
        main.py.err main.py.out \
        comment.cut comment.err comment.out \
        latex.py

main: 
	$(PDF)
	$(LATEXRM)
view:
	$(OPEN) main.pdf
v:
	$(OPEN) main.pdf
c:
	$(LATEXRM)
