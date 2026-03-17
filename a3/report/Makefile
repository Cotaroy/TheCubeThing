.PHONY : clean

project.pdf : project.tex
	pdflatex project.tex
	pdftotext -layout project.pdf -

clean :
	rm -rf - **.pdf **.aux **.log
