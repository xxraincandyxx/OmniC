# Makefile

# Format all Python files recursively
style:
	@echo "Formatting C++ files with clang-format..."
	@find src/. \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \
		-o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} +
	@find include/. \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \
		-o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} +
	@find examples/. \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" \
		-o -name "*.h" -o -name "*.hpp" \) -exec clang-format -i -style=file {} +
	@echo "C++ formatting complete."

# Bugged: Clang-tidy currently not runnable
tidy:
	@echo "Fixing fixable clang-tidy issues..."
	@if [ -f compile_commands.json ]; then \
		clang-tidy --fix -p compile_commands.json; \
	else \
		echo "Error: 'compile_commands.json' not found. \
					Please generate it first (e.g., with CMake)."; \
		exit 1; \
	fi
	@echo "clang-tidy fixes complete."

# Check formatting without making changes
check-style:
	@echo "Checking Python formatting..."
	@find . -name '*.py' -exec ruff format
		--config pyproject.toml --diff --quiet {} +

clean:
	@echo "Cleaning temporary files and build artifacts..."

	# Remove Python cache directories and bytecode
	rm -rf __pycache__ */__pycache__ .pytest_cache .mypy_cache .coverage .tox
	find . -type d -name '__pycache__' -exec rm -rf {} +
	find . -name '*.pyc' -delete -o -name '*.pyo' -delete
	
	# Remove Vim swap files and directories
	find . -name '*.swp' -delete -o -name '*.swo' -delete
	find . -type d -name ".vim" -exec rm -rf {} +
	
	# Remove build artifacts and package directories
	rm -rf dist/ build/ *.egg-info/ .eggs/
	
	# Remove IDE/editor-specific files
	# rm -rf .idea/ .vscode/ .cache/
	
	# Remove macOS-specific files
	find . -name '.DS_Store' -delete
	
	# Remove temporary files
	rm -f *~ .*~ *.bak .*.bak

	# Remove debug executables
	@find . -name 'test' -delete
	@find . -name 'workflow' -delete

	@echo "Clean complete!"

.PHONY: style tidy check-style clean
