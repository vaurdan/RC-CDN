LIB_A = Client
LIB_B = CServer
#LIB_C = folder_c

MY_TARGETS = $(LIB_A) $(LIB_B)
MAKE = make

.PHONY: all $(MY_TARGETS)
all: $(MY_TARGETS)

$(MY_TARGETS):
	@$(MAKE) -C $@;

$(LIB_C): $(LIB_A) $(LIB_B)

clean:
		@for i in $(MY_TARGETS); do \
		echo "Clearing in $$i..."; \
		(cd $$i; make clean); done