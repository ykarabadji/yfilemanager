BIN = fmanager
PREFIX = /usr

all:
	@echo "Nothing to build. Run 'make install' to copy $(BIN) to $(PREFIX)/bin/."

install:
	sudo cp $(BIN) $(PREFIX)/bin/

uninstall:
	sudo rm -f $(PREFIX)/bin/$(BIN)

clean:
	@echo "Nothing to clean. Only the binary exists."
