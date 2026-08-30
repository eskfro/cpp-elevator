#!/usr/bin/env bash
#
# simall.sh - launch N sim servers + N elevator nodes in one tiled tmux window.
#
#   +---------+---------+---------+
#   |  sim 0  |  sim 1  |  sim 2  |   top row    -> SimElevatorServer
#   +---------+---------+---------+
#   | node 0  | node 1  | node 2  |   bottom row -> elevator-node
#   +---------+---------+---------+
#
# Usage:
#   ./scripts/simall.sh          # ids 0 1 2 (default)
#   ./scripts/simall.sh 0 1 2 3  # any number of ids -> 2xN grid
#   ATTACH=1 ./scripts/simall.sh # attach here instead of a new window
#
set -euo pipefail

SESSION="${SESSION:-elevsim}"
BASE_PORT="${BASE_PORT:-15657}"   # sim port for id 0; id N -> BASE_PORT + N
NODE_DELAY="${NODE_DELAY:-2}"     # wait before starting each node
BUILD_JOBS="${BUILD_JOBS:-4}"
ATTACH="${ATTACH:-0}"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

IDS=("$@")
[ "${#IDS[@]}" -eq 0 ] && IDS=(0 1 2)
N="${#IDS[@]}"

SIM_BIN="services/Simulator-v2/SimElevatorServer"
NODE_BIN="build/services/elevator-node/elevator-node"

# Build once; abort all if it fails.
[ -d build ] || cmake -S . -B build
cmake --build build -j"$BUILD_JOBS"

[ -f "$SIM_BIN" ] || { echo "Simulator binary not found: $SIM_BIN" >&2; exit 1; }
chmod +x "$SIM_BIN"

tmux kill-session -t "$SESSION" 2>/dev/null || true
tmux new-session -d -s "$SESSION" -x 320 -y 90
tmux set-option -t "$SESSION" -g mouse on
tmux set-option -t "$SESSION" pane-border-status top

first_pane="$(tmux list-panes -t "$SESSION" -F '#{pane_id}' | head -n1)"
bottom_first="$(tmux split-window -v -l 50% -t "$first_pane" -P -F '#{pane_id}')"

# Split a starting pane into a row of N even columns; print the pane ids in order.
make_row() {
  local last="$1" k frac p
  local panes=("$1")
  for ((k = 1; k < N; k++)); do
    frac=$(( 100 * (N - k) / (N - k + 1) ))
    p="$(tmux split-window -h -l "${frac}%" -t "$last" -P -F '#{pane_id}')"
    panes+=("$p"); last="$p"
  done
  printf '%s\n' "${panes[@]}"
}

mapfile -t TOP < <(make_row "$first_pane")
mapfile -t BOT < <(make_row "$bottom_first")

for i in "${!IDS[@]}"; do
  id="${IDS[$i]}"
  port=$(( BASE_PORT + id ))

  tmux select-pane -t "${TOP[$i]}" -T "sim $id (port $port)"
  tmux send-keys -t "${TOP[$i]}" "./$SIM_BIN --port $port" C-m

  tmux select-pane -t "${BOT[$i]}" -T "node $id"
  tmux send-keys -t "${BOT[$i]}" "sleep $NODE_DELAY; ./$NODE_BIN $id 1" C-m
done

tmux select-pane -t "${BOT[0]}"

if [ "$ATTACH" = "1" ] || [ -z "${DISPLAY:-}${WAYLAND_DISPLAY:-}" ] || ! command -v gnome-terminal >/dev/null; then
  exec tmux attach -t "$SESSION"
else
  setsid gnome-terminal --maximize --title="elevator simall" -- tmux attach -t "$SESSION" >/dev/null 2>&1 &
  disown 2>/dev/null || true
  echo "Launched '$SESSION'. Kill it with: tmux kill-session -t $SESSION"
fi
