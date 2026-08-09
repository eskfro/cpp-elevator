#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <id (0|1|2)>"
  exit 2
fi

id=$1
case "$id" in
  0) port=15657 ;;
  1) port=15658 ;;
  2) port=15659 ;;
  *) echo "Invalid id: $id. Use 0,1,2." ; exit 2 ;;
esac

clear
cmake --build build -j4

sim_bin="services/Simulator-v2/SimElevatorServer"
if [ ! -f "$sim_bin" ]; then
  echo "Simulator binary not found: $sim_bin"
  exit 1
fi
chmod +x "$sim_bin"

# start simulator in a new terminal, then start the elevator node in another terminal
gnome-terminal -- bash -c "${sim_bin} --port ${port}; exec bash" &
gnome-terminal -- bash -c "sleep 2; ./build/services/elevator-node/elevator-node ${id} 1; exec bash" &

wait