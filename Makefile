LIB_A = Client
LIB_B = CentralServer
LIB_C = StorageServer

MY_TARGETS = $(LIB_A) $(LIB_B) $(LIB_C)
MAKE = make
BIN_DIR = $(CURDIR)

INC=-I$(BIN_DIR) -I$(BIN_DIR)/$(LIB_A) -I$(BIN_DIR)/$(LIB_B) -I$(BIN_DIR)/$(LIB_C)
export BIN_DIR
export INC

.PHONY: all $(MY_TARGETS)
all: $(MY_TARGETS)

$(MY_TARGETS):
	@$(MAKE) -C $@;

$(LIB_C): $(LIB_A) $(LIB_B)

clean:
		@for i in $(MY_TARGETS); do \
		echo "Clearing in $$i..."; \
		(cd $$i; make clean); done