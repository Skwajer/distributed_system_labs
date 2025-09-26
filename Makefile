SRC_DIR := src
BUILD_DIR := build
CC := clang
CFLAGS := -Wall -Wextra -O2
DEBUGGER_CMD := gdb
ARGS := # Arguments to pass to run/valgrind

# Цвета для вывода
COLOR_HEADER := \033[38;2;72;61;139m    # #483D8B (DarkSlateBlue) - заголовки
COLOR_PROGRAM := \033[38;2;106;90;205m  # #6A5ACD (SlateBlue) - вывод программ
COLOR_ERROR := \033[38;2;128;0;0m       # #800000 (Maroon) - ошибки
COLOR_RESET := \033[0m

ifeq ($(V),1)
	Q :=
else
	Q := @
endif

TASK ?=

.PHONY: all clean run pwn valgrind help

all:
	$(Q)echo "Nothing to build. Use 'make run TASK=<n>' to compile and run src/<n>.c"

$(BUILD_DIR)/%: $(SRC_DIR)/%.c
	$(Q)echo "Compiling $< -> $@"
	$(Q)mkdir -p $(BUILD_DIR)
	$(Q)$(CC) $(CFLAGS) -o $@ $<

run:
	@if [ -z "$(TASK)" ]; then \
		echo -e "$(COLOR_ERROR)Error: TASK is not set. Usage: make run TASK=<n>$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(MAKE) $(BUILD_DIR)/$(TASK)
	$(Q)echo -e "$(COLOR_HEADER)Running task $(TASK)...$(COLOR_RESET)"
	$(Q)echo -e "$(COLOR_PROGRAM)"
	$(Q)./$(BUILD_DIR)/$(TASK) $(ARGS) || true
	$(Q)echo -e "$(COLOR_RESET)"

pwn:
	@if [ -z "$(TASK)" ]; then \
		echo -e "$(COLOR_ERROR)Error: TASK is not set. Usage: make pwn TASK=<n>$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(MAKE) $(BUILD_DIR)/$(TASK)
	$(Q)echo -e "$(COLOR_HEADER)Starting debug session for task $(TASK)...$(COLOR_RESET)"
	$(Q)echo -e "$(COLOR_PROGRAM)"
	$(Q)$(DEBUGGER_CMD) ./$(BUILD_DIR)/$(TASK)
	$(Q)echo -e "$(COLOR_RESET)"

valgrind:
	@if [ -z "$(TASK)" ]; then \
		echo -e "$(COLOR_ERROR)Error: TASK is not set. Usage: make valgrind TASK=<n>$(COLOR_RESET)"; \
		exit 1; \
	fi
	$(MAKE) $(BUILD_DIR)/$(TASK)
	$(Q)echo -e "$(COLOR_HEADER)Running task $(TASK) with Valgrind...$(COLOR_RESET)"
	$(Q)echo -e "$(COLOR_PROGRAM)"
	$(Q)valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --error-exitcode=1 \
		./$(BUILD_DIR)/$(TASK) $(ARGS) || true
	$(Q)echo -e "$(COLOR_RESET)"

clean:
	$(Q)echo "Cleaning build artifacts..."
	$(Q)rm -rf $(BUILD_DIR)

help:
	$(Q)echo "Laboratory Works Build System"
	$(Q)echo "Targets:"
	$(Q)echo "  run TASK=<n>       - Compile and run src/<n>.c"
	$(Q)echo "  pwn TASK=<n>       - Debug build/<n> with pwndbg"
	$(Q)echo "  valgrind TASK=<n>  - Run build/<n> under Valgrind"
	$(Q)echo "  clean              - Remove build artifacts"
	$(Q)echo ""
	$(Q)echo "Variables:"
	$(Q)echo "  TASK          - Task number (e.g., 1, 2, ...)"
	$(Q)echo "  V=1           - Verbose output"
	$(Q)echo "  ARGS          - Arguments for run/valgrind"