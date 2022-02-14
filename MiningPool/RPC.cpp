#include "RPC.h"


void RPC::init() {
	curl = curl_easy_init();
}

size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct* mem = (struct MemoryStruct*)userp;

	char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

json RPC::execRPC(string data, Settings *settings) {

	execLock.lock();

	chunk.size = 0;



	curl_easy_setopt(curl, CURLOPT_URL, settings->rpcURL.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC);
	curl_easy_setopt(curl, CURLOPT_USERNAME, settings->rpcUser.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, settings->rpcPassword.c_str());

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

	json result = json::parse(chunk.memory);

	execLock.unlock();

	return result;
}