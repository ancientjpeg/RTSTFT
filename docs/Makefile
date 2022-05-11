PANDOC="/opt/homebrew/bin/pandoc"

TARGET=paper.md
OUTPUT=$(TARGET:.md=.pdf)

all: 
	@echo $@
	$(PANDOC)  --csl=apa.csl --bibliography=RTSTFT.bib -C -o $(OUTPUT) $(TARGET)