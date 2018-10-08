#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>
#include "argon2.h"

using namespace std;

#define BLOCK_HEADER_SIZE (80)
#define NONCE_OFFSET (76)
#define DIFFICULTY_OFFSET (72)

#define ARGON2_PARALLELISM (1)
#define ARGON2_MEMORY (2345) // More than 2MiB
#define ARGON2_ITERATIONS (1)
#define ARGON2_SALT ("saltysalt")
#define ARGON2_SALT_LENGTH (9)
#define ARGON2_SIZE (32)

int index = -1;

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint32_t> dis(std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

bool hash_less_than(uint8_t const p_hash_a[32], uint8_t const p_hash_b[32]) {
  for(size_t i = 0; i < 32; i++) {
    if(p_hash_a[i] < p_hash_b[i])
      return true;
    else if(p_hash_a[i] > p_hash_b[i])
      return false;
  }
  return false;
}

void mine(uint8_t *p_data, uint8_t p_diff[32], size_t p_index) {
	uint8_t hash[32];
	uint32_t *nonce = reinterpret_cast<uint32_t *>(&p_data[NONCE_OFFSET]);
	*nonce = 0;
	while(index < 0) {
    argon2i_hash_raw(ARGON2_ITERATIONS, ARGON2_MEMORY, ARGON2_PARALLELISM, p_data, BLOCK_HEADER_SIZE, ARGON2_SALT, ARGON2_SALT_LENGTH, reinterpret_cast<void*>(hash), ARGON2_SIZE);
		if(hash_less_than(hash, p_diff))
			index = p_index;
		else
			(*nonce) = dis(gen);
	}
}

void print_hash(uint8_t p_hash[32]) {
  std::cout<<std::hex;
  for(size_t i = 0; i < 32; i++) {
    if(p_hash[i] < 16)
      std::cout<<"0";
    std::cout<<(unsigned int)p_hash[i];
  }
}

int main(int argc, char **argv) {

  uint8_t input_data[BLOCK_HEADER_SIZE];
  cin.read(reinterpret_cast<char *>(input_data), BLOCK_HEADER_SIZE);
  size_t cores = std::thread::hardware_concurrency();
	uint8_t **data = new uint8_t*[cores];
	for(size_t i = 0; i < cores; i++) {
    data[i] = new uint8_t[BLOCK_HEADER_SIZE];
    std::copy(input_data, input_data + BLOCK_HEADER_SIZE, data[i]);
  }

  uint8_t diff[ARGON2_SIZE];
  cin.read(reinterpret_cast<char *>(diff), ARGON2_SIZE);

  index = -1;
	std::vector<std::thread> threads;
	for(size_t i = 0; i < cores; i++) threads.push_back(std::thread(mine, data[i], diff, i));
	std::for_each(threads.begin(), threads.end(), [](thread &t) { t.join(); });

	cout.write(reinterpret_cast<char *>(data[index]), BLOCK_HEADER_SIZE);

  for(size_t i = 0; i < cores; i++) delete[] data[i];
	delete[] data;

  return 0;
}
