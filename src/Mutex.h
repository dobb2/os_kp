#pragma once
#include <pthread.h>
#include <vector>
#include <stdexcept>
#include <string>

class Mutex {
public:
	Mutex() = default;
	Mutex(int n){
		number = n;
		init_res = pthread_mutex_init(&mutex, NULL);
		if(init_res != 0){
			throw std::runtime_error("Error erno");
		}
	}
	~Mutex(){
		pthread_mutex_destroy(&mutex);
	}
	std::string mutex_lock(std::vector<int> &v);
	std::string mutex_unlock(std::vector<int> &v);

private:
	int number;
	pthread_mutex_t mutex;
	int init_res;
};





