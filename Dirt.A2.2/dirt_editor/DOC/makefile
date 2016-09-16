# $Xukc: makefile,v 1.1 91/12/17 14:38:32 rlh2 Rel $
#
# Makefile for documentation.  Requires latex, bibtex, divps (or equivalent)
# and the psfig macros.
#
PS_PREVIEWER = gspreview
PREVIEWER = xdvi
LATEX = latex
BIBTEX = bibtex
DVI2PS = dvips

FIGURES = figures/action-dialog.ps figures/app-dialog-box.ps \
	  figures/children.ps figures/cross.ps \
          figures/edit-button.ps figures/editing.ps figures/example.eps \
	  figures/exploded.eps figures/help-dialog.ps \
          figures/hier.eps figures/lock-button.ps figures/longName.ps \
	  figures/main-window.ps figures/manyOfMany.ps \
          figures/new-button.ps figures/new-dialog-box.ps \
	  figures/oneOfMany.ps figures/pressToEdit.ps \
          figures/recycle.ps figures/save-dialog.ps figures/selection.ps \
	  figures/string.ps figures/templates.ps \
          figures/tick.ps figures/translations.ps figures/value.ps \
	  figures/xaw-examples.ps

all: tech_doc

preview: dirt.dvi
	$(PREVIEWER) dirt.dvi

ps_preview: tech_doc
	$(PS_PREVIEWER) dirt.ps

tech_doc: dirt.ps 

dirt.aux: dirt.tex
	$(LATEX) dirt

dirt.bbl: dirt.aux alphathesis.bst database.bib
	$(BIBTEX) dirt; $(LATEX) dirt

dirt.dvi: dirt.bbl
	$(LATEX) dirt
	touch dirt.bbl

dirt.ps: dirt.dvi $(FIGURES)
	$(DVI2PS) dirt

clean:
	rm -f dirt.aux dirt.bbl dirt.blg dirt.dvi dirt.lof \
	      dirt.log dirt.ps dirt.toc
