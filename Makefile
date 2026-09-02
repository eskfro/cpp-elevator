
.PHONY: sim0 sim1 sim2 simall clean

simall:
	./scripts/simall.sh $(IDS)

sim0:
	clear
	./scripts/node_and_server.sh 0

sim1:
	clear
	./scripts/node_and_server.sh 1

sim2:
	clear
	./scripts/node_and_server.sh 2

server:
	clear
	chmod +x services/Simulator-v2/SimElevatorServer
	./services/Simulator-v2/SimElevatorServer
	
clean: 
	rm -rf build