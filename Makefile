# Root Makefile: convenience targets for the whole repo.

.DEFAULT_GOAL := help

EXAMPLES     := $(patsubst %/,%,$(wildcard examples/*/))
FORMAT_FILES := $(shell find drivers generic examples test \
                     \( -name '*.c' -o -name '*.h' \))

.PHONY: help examples test check docs docs-check docs-open format \
        format-check clean

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*## "} {printf "  %-14s %s\n", $$1, $$2}'

examples: ## Build every example for the target (ARM)
	@for e in $(EXAMPLES); do $(MAKE) -C $$e || exit 1; done

test: ## Run the host unit tests
	$(MAKE) -C test test

check: test examples ## Run the tests and build the examples (what CI runs)

docs: ## Generate the Doxygen HTML into docs/api
	doxygen Doxyfile

docs-check: ## Fail if Doxygen prints any warning
	@out=$$(doxygen Doxyfile 2>&1); \
	if [ -n "$$out" ]; then echo "$$out"; exit 1; fi

docs-open: docs ## Generate the docs and open them in the browser
	xdg-open docs/api/html/index.html

format: ## Format all C sources with clang-format
	clang-format -i $(FORMAT_FILES)

format-check: ## Fail if any file is not formatted
	clang-format --dry-run --Werror $(FORMAT_FILES)

clean: ## Remove build output
	@for e in $(EXAMPLES); do $(MAKE) -C $$e clean; done
	$(MAKE) -C test clean
	rm -rf docs/api
