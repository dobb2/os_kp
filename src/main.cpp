#include "Mutex.h"
#include <iostream>
#include <optional>
#include <pthread.h>
#include <vector>
#include <string>

struct thread_params {
	std::vector<int> transfer_scheme;
	int *res;
	int *account_1;
	int *account_2;
	int *account_3;
	int *account_4;
	int money;
	Mutex m1;
	Mutex m2;
	Mutex m3;
	Mutex m4;
};

void *transactions(void *arg){
	thread_params targ = *(thread_params *) arg;
	std::vector<int> &transfer_scheme = targ.transfer_scheme;
	int *bank1 = targ.account_1;
	int *bank2 = targ.account_2;
	int *bank3 = targ.account_3;
	int *bank4 = targ.account_4;
	int *res = targ.res;
	int money = targ.money;
	Mutex m1 = targ.m1;
	Mutex m2 = targ.m2;
	Mutex m3 = targ.m3;
	Mutex m4 = targ.m4;
	std::vector<int> busy_resources;
	std::string b;
	int f, past;
	int t = 1;
	int now = 0;
	int size = 0;

	while((transfer_scheme.size() != 0) && (t == 1) ) {
		f = transfer_scheme.front();
		transfer_scheme.erase(transfer_scheme.begin());
		switch (f){
			case 1:
				b = m1.mutex_lock(busy_resources);
				if (b == "Ok" && size == 0){
					*bank1 = *bank1 + money;
					break;
				} else if ((b == "Error") || (b == "Possible deadlock") ){
					t = 0;
					break;
				}
			case 2:
				b = m2.mutex_lock(busy_resources);
				if (b == "Ok" && size == 0) {
					*bank2 = *bank2 + money;
					break;
				} else if (b == "Ok" && size != 0) {
					*bank1 = *bank1 - money;//условно это операция перевода денег с одного банка к другому
					m1.mutex_unlock(busy_resources);
					*bank2 = *bank2 + money;
					break;
				} else if ((b == "Error") || (b == "Possible deadlock") ){
					t = 0;
					break;
				}
			case 3:
				b = m3.mutex_lock(busy_resources);
				if (b == "Ok" && size == 0){
					*bank3 = *bank3 + money;
					break;
				} else if (b == "Ok" && size != 0) {
					switch(now){
						case 1:
							*bank1 = *bank1 - money;
							m1.mutex_unlock(busy_resources);
							*bank3 = *bank3 + money;
							break;
						case 2:
							*bank2 = *bank2 - money;
							m2.mutex_unlock(busy_resources);
							*bank3 = *bank3 + money;
							break;
					}
					break;
				} else if ((b == "Error") || (b == "Possible deadlock") ){
					t = 0;
					break;
				}
			case 4:
				b = m4.mutex_lock(busy_resources);
				if (b == "Ok" && size == 0){
					*bank4 = *bank4 + money;
					break;
				} else if(b == "Ok" && size != 0){
					switch(now){
						case 1:
							*bank1 = *bank1 - money;
							m1.mutex_unlock(busy_resources);
							*bank4 = *bank4 + money;
							break;
						case 2:
							*bank2 = *bank2 - money;
							m2.mutex_unlock(busy_resources);
							*bank4 = *bank4 + money;
							break;
						case 3:
							*bank3 = *bank3 - money;
							m3.mutex_unlock(busy_resources);
							*bank4 = *bank4 + money;
							break;
					}
				} else if ((b == "Error") || (b == "Possible deadlock") ){
					t = 0;
					break;
				}
		}//switch
		past = now;
		now = f;
		size++;

	}
	

	if(t != 1) {
		switch(past){
			case 1:
				*bank1 = *bank1 - money;
				m1.mutex_unlock(busy_resources);
				break;
			case 2:
				*bank2 = *bank2 - money;
				m2.mutex_unlock(busy_resources);
				break;
			case 3:
				*bank3 = *bank3 - money;
				m3.mutex_unlock(busy_resources);
				break;
			case 4:
				*bank4 = *bank4 - money;
				m4.mutex_unlock(busy_resources);
				break;
		}
		*res = *res + money;	
	}

	return NULL;	
}


int main(){
	Mutex m1(1), m2(2), m3(3), m4(4);
	int number_clients, i;
	int bank4 = 0, bank3 = 0, bank2 = 0, bank1 = 0;
	int loose_money = 0;

	std::cout << "Enter the number of clients\n";
	std::cin >> number_clients;

	std::vector<std::optional<int>> results(number_clients);
	std::vector<pthread_t> threads(number_clients);
	std::vector<thread_params> params(number_clients);


	for(i = 0; i < number_clients; i++){
		int num;
		std::cout << "Enter the transfer amount\n";
		std::cin >> params[i].money;

		std::cout << "Select transfer scheme in chronological order:\n";
		std::cout << "1 - Bank 1\n";
		std::cout << "2 - Bank 2\n";
		std::cout << "3 - Bank 3\n";
		std::cout << "4 - Bank 4\n";
		std::cout << "Enter scheme trans\n";
		

		int value;
		int j = 0;
		while (1) {
    		std::cin >> value;
    		if (value == 0) break ;
    		params[i].transfer_scheme.push_back(value);
    		j++;
		}

		params[i].account_1 = &bank1;
		params[i].account_2 = &bank2;
		params[i].account_3 = &bank3;
		params[i].account_4 = &bank4;
		params[i].res = &loose_money;
		params[i].m1 = m1;
		params[i].m2 = m2;
		params[i].m3 = m3;
		params[i].m4 = m4;
	}
	for(i = 0; i < number_clients; ++i) {
		if(pthread_create(&threads[i], NULL, 
			&transactions, &params[i]) != 0){
			std::cout << "Error create\n";
			return 1;
		}
	}

	for(i = 0; i < number_clients; ++i) {
		if(pthread_join(threads[i], NULL) != 0) {
			std::cout << "Error join\n";
			return 1;
		}
	}
	
	std::cout << "Amount on the Banks accounts\n";
	std::cout << "Bank 1: " << bank1 << "\n";
	std::cout << "Bank 2: " << bank2 << "\n";
	std::cout << "Bank 3: " << bank3 << "\n";
	std::cout << "Bank 4: " << bank4 << "\n";
	std::cout << "Amount that may have been in Bank accounts:" << loose_money << "\n";

	return 0;
}
