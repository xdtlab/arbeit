all:
	make -C ./argon2
	g++ --std=c++11 *.cpp -lpthread -Iargon2/include -Largon2 -largon2 -o ./miner
