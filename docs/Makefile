PANDOC="/opt/homebrew/bin/pandoc"

TARGET=paper.md
OUTPUT=$(TARGET:.md=.pdf)

all: $(OUTPUT)
$(OUTPUT): $(TARGET)
	@echo $@
	$(PANDOC) -o $@ $<