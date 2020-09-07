#include "Mutex.h"

using namespace std;

string Mutex::mutex_lock(vector<int>& v){
	if (v.size() == 0 || v.back() < (this->number) ){
		v.push_back(this->number);
		int b = pthread_mutex_lock(&(this->mutex));
		if(b == 0) {
			return "Ok";
		} else {
			return "Error: " + to_string(b);
		}
	} else if (v.back() > (this->number)) {
		return "Possible deadlock";
	}
}
string Mutex::mutex_unlock(vector<int>& v){
	vector<int>::iterator it = v.begin();

	while(it != v.end()){
		if(*it == (this->number)){
			v.erase(it);
			int b = pthread_mutex_unlock(&(this->mutex));
			if(b == 0) {
				return "Ok";
			} else {
				return "Error: " + to_string(b);
			}
		}
		it++;
	}
	if(it == v.end()){
		return "The resource did not block this mutex";
	}
}
