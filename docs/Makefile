PANDOC="/opt/homebrew/bin/pandoc"

TARGET=paper.md
OUTPUT=$(TARGET:.md=.pdf)

all: 
	python3 diagrams.py
	$(PANDOC) --csl=apa.csl --bibliography=RTSTFT.bib -C -o $(OUTPUT) $(TARGET)
clean:
	@rm paper.pdf
	@rm phi_omega.png
	@rm window.png