#!/usr/bin/env bash
# doxygen-workflow.sh
# Cross-platform helper for generating Doxygen docs for C/C++
# Works on Linux and macOS.
#
# Usage: ./doxygen-workflow.sh <command> [options]
# Commands:
#   install-deps   - try to install doxygen, graphviz, fswatch/inotify-tools
#   init           - create a Doxyfile (doxygen -g) and set sensible defaults
#   build          - run doxygen to build docs (HTML by default)
#   clean          - remove generated docs (html/latex dirs)
#   serve [port]   - serve generated html via python HTTP server (default port 8000)
#   watch          - watch source and rebuild automatically (requires fswatch or inotifywait)
#   help           - show this help
#
# Notes:
#  - Script uses set -euo pipefail for safety.
#  - Edit Doxyfile or use environment variables to customize.
#  - Designed to be simple and self-contained.

set -euo pipefail
IFS=$'\n\t'

# --- Configurable defaults (change as needed) ---
PROJECT_ROOT=${PROJECT_ROOT:-$(pwd)}
SOURCE_DIRS=${SOURCE_DIRS:-"src include"} # space-separated
OUTPUT_DIR=${OUTPUT_DIR:-docs}
HTML_OUTPUT_DIR="${OUTPUT_DIR}/html"
LATEX_OUTPUT_DIR="${OUTPUT_DIR}/latex"
DOXYFILE=${DOXYFILE:-Doxyfile}
DOXYGEN_BIN=${DOXYGEN_BIN:-doxygen}
GRAPHVIZ_BIN=${GRAPHVIZ_BIN:-dot}

# --- Helpers ---
log() { printf '\033[1;34m[doctool]\033[0m %s\n' "$*"; }
err() { printf '\033[1;31m[doctool]\033[0m %s\n' "$*" >&2; }
die() {
	err "$*"
	exit 1
}

detect_os() {
	unameOut="$(uname -s)"
	case "${unameOut}" in
	Linux*) os=linux ;;
	Darwin*) os=macos ;;
	*) os=unknown ;;
	esac
	echo "$os"
}

command_exists() {
	command -v "$1" >/dev/null 2>&1
}

# --- Commands ---
cmd_install_deps() {
	os="$(detect_os)"
	log "Detected OS: $os"

	if command_exists "$DOXYGEN_BIN"; then
		log "doxygen already installed: $(command -v $DOXYGEN_BIN)"
	else
		if [ "$os" = "macos" ]; then
			if command_exists brew; then
				log "Installing doxygen via brew..."
				brew install doxygen || die "brew install doxygen failed"
			else
				die "Homebrew not found. Install Homebrew first or install doxygen manually."
			fi
		elif [ "$os" = "linux" ]; then
			if command_exists apt-get; then
				log "Installing doxygen via apt-get (requires sudo)..."
				sudo apt-get update
				sudo apt-get install -y doxygen || die "apt-get install doxygen failed"
			elif command_exists yum; then
				log "Installing doxygen via yum (requires sudo)..."
				sudo yum install -y doxygen || die "yum install doxygen failed"
			else
				die "No supported package manager found. Install doxygen manually."
			fi
		else
			die "Unsupported OS. Install doxygen manually."
		fi
	fi

	# graphviz (for call graphs/diagrams)
	if command_exists "$GRAPHVIZ_BIN"; then
		log "graphviz found: $(command -v $GRAPHVIZ_BIN)"
	else
		if [ "$os" = "macos" ]; then
			brew install graphviz || die "brew install graphviz failed"
		elif [ "$os" = "linux" ]; then
			if command_exists apt-get; then
				sudo apt-get install -y graphviz || die "apt-get install graphviz failed"
			elif command_exists yum; then
				sudo yum install -y graphviz || die "yum install graphviz failed"
			fi
		fi
	fi

	# watcher: fswatch (macOS) or inotify-tools (Linux)
	if command_exists fswatch || command_exists inotifywait; then
		log "File watcher already installed."
	else
		if [ "$os" = "macos" ]; then
			brew install fswatch || die "brew install fswatch failed"
		elif [ "$os" = "linux" ]; then
			if command_exists apt-get; then
				sudo apt-get install -y inotify-tools || die "apt-get install inotify-tools failed"
			elif command_exists yum; then
				sudo yum install -y inotify-tools || die "yum install inotify-tools failed"
			fi
		fi
	fi

	log "Dependencies installed/checked."
}

cmd_init() {
	if command_exists "$DOXYGEN_BIN"; then
		log "Generating default Doxyfile: $DOXYFILE"
		"$DOXYGEN_BIN" -g "$DOXYFILE"
	else
		die "doxygen not found. Run './doxygen-workflow.sh install-deps' or install doxygen yourself."
	fi

	# Make sensible defaults: set OUTPUT_DIRECTORY, INPUT, RECURSIVE, EXTRACT_ALL etc.
	log "Patching $DOXYFILE with sensible defaults..."
	# Use perl to make robust in-place replacements (compatible across macOS & Linux)
	perl -0777 -pe "
    s/^\\s*OUTPUT_DIRECTORY\\s*=.*$/OUTPUT_DIRECTORY       = $OUTPUT_DIR/m;
    s/^\\s*GENERATE_LATEX\\s*=.*$/GENERATE_LATEX           = YES/m;
    s/^\\s*GENERATE_HTML\\s*=.*$/GENERATE_HTML             = YES/m;
    s/^\\s*INPUT\\s*=.*$/INPUT                             = $SOURCE_DIRS/m;
    s/^\\s*RECURSIVE\\s*=.*$/RECURSIVE                     = YES/m;
    s/^\\s*EXTRACT_ALL\\s*=.*$/EXTRACT_ALL                 = YES/m;
    s/^\\s*EXTRACT_PRIVATE\\s*=.*$/EXTRACT_PRIVATE         = YES/m;
    s/^\\s*EXTRACT_STATIC\\s*=.*$/EXTRACT_STATIC           = YES/m;
    s/^\\s*FULL_PATH_NAMES\\s*=.*$/FULL_PATH_NAMES         = YES/m;
    s/^\\s*JAVADOC_AUTOBRIEF\\s*=.*$/JAVADOC_AUTOBRIEF     = YES/m;
    s/^\\s*GENERATE_TREEVIEW\\s*=.*$/GENERATE_TREEVIEW     = YES/m;
    s/^\\s*DOT_GRAPH_MAX_NODES\\s*=.*$/DOT_GRAPH_MAX_NODES = 50/m;
    s/^\\s*HAVE_DOT\\s*=.*$/HAVE_DOT                       = YES/m;
  " -i "$DOXYFILE"

	log "Init complete. Edit $DOXYFILE if you want custom settings."
}

cmd_build() {
	if [ ! -f "$DOXYFILE" ]; then
		die "Doxyfile not found. Run './doxygen-workflow.sh init' first or provide a Doxyfile."
	fi

	log "Running doxygen..."
	"$DOXYGEN_BIN" "$DOXYFILE"

	if [ -d "$HTML_OUTPUT_DIR" ]; then
		log "HTML docs built at: $HTML_OUTPUT_DIR"
	else
		warn="WARNING: HTML output directory not found after build."
		err "$warn"
	fi
}

cmd_clean() {
	log "Cleaning output directories..."
	rm -rf "$OUTPUT_DIR"
	log "Removed $OUTPUT_DIR"
}

cmd_serve() {
	port="${1:-8000}"
	if [ ! -d "$HTML_OUTPUT_DIR" ]; then
		die "HTML docs not found. Run './doxygen-workflow.sh build' first."
	fi
	log "Serving $HTML_OUTPUT_DIR on http://localhost:$port"
	# Use python3 -m http.server (works on both macOS & Linux)
	(cd "$HTML_OUTPUT_DIR" && python3 -m http.server "$port")
}

cmd_watch() {
	os="$(detect_os)"
	if command_exists fswatch; then
		watcher="fswatch"
	elif command_exists inotifywait; then
		watcher="inotifywait"
	else
		die "No file watcher found (fswatch or inotifywait). Run './doxygen-workflow.sh install-deps'."
	fi

	log "Starting watch mode using $watcher. Source: $SOURCE_DIRS"

	if [ "$watcher" = "fswatch" ]; then
		# macOS fswatch: produce events, rebuild on change
		fswatch -o $SOURCE_DIRS | while read -r _; do
			log "Change detected — rebuilding..."
			./doxygen-workflow.sh build
			log "Rebuild done."
		done
	else
		# Linux inotifywait: watch recursively for modifies/creates/deletes
		while true; do
			inotifywait -r -e modify,create,delete,move $SOURCE_DIRS >/dev/null 2>&1
			log "Change detected — rebuilding..."
			./doxygen-workflow.sh build
			log "Rebuild done."
		done
	fi
}

cmd_help() {
	sed -n '1,200p' "$0" | sed -n '1,120p'
	cat <<'EOF'

Examples:
  ./doxygen-workflow.sh install-deps
  ./doxygen-workflow.sh init
  ./doxygen-workflow.sh build
  ./doxygen-workflow.sh serve 9000
  ./doxygen-workflow.sh watch

Customize:
 - set environment variables before invocation to override defaults:
     PROJECT_ROOT, SOURCE_DIRS, OUTPUT_DIR, DOXYFILE

EOF
}

# --- Dispatch ---
case "${1:-help}" in
install-deps) cmd_install_deps ;;
init) cmd_init ;;
build) cmd_build ;;
clean) cmd_clean ;;
serve) cmd_serve "${2:-8000}" ;;
watch) cmd_watch ;;
help | --help | -h) cmd_help ;;
*)
	err "Unknown command: ${1:-}"
	cmd_help
	exit 2
	;;
esac
