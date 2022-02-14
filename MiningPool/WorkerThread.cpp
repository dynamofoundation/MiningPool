#include "WorkerThread.h"


bool readLine(vector<char>& buffer, string& line) {

	bool found = false;
	int i = 0;
	while ((!found) && (i != buffer.size()))
		if (buffer[i] == '\n')
			found = true;
		else
			i++;

	if (found) {
		line = string(buffer.data(), i);
		buffer.erase(buffer.begin(), buffer.begin() + i + 1);
	}

	return found;
}

void WorkerThread::clientWorker(int clientSocket) {


	//send difficulty
	//


	vector<char> buffer;


	int error = false;
	while (!error) {

		const int tmpBuffLen = 4096;
		char tmpBuff[tmpBuffLen];
		memset(tmpBuff, 0, tmpBuffLen);
		int numRecv = recv(clientSocket, tmpBuff, tmpBuffLen, 0);
		if (numRecv > 0)
			for (int i = 0; i < numRecv; i++)
				buffer.push_back(tmpBuff[i]);


		//TODO - evaluate memory leaks due to return
		if (numRecv < 0) {
			error = true;
			return;
		}


		if (buffer.size() > 0) {
			string line;
			while (readLine(buffer, line)) {
				json msg = json::parse(line.c_str());
			}
		}


	}

}



