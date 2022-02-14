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



void WorkerThread::blockUpdateThread(int clientSocket, Global* global) {

	while (!socketError) {
		if (authDone) {
			if (lastBlockHeightSent != global->currentBlockHeight) {
				sendCurrentBlock(clientSocket, global);
				lastBlockHeightSent = global->currentBlockHeight;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

}



void WorkerThread::clientWorker(int clientSocket, Global *global) {

	vector<char> buffer;
	string wallet = "";

	difficulty = 1;
	lastBlockHeightSent = -1;

	thread blockUpdate(&WorkerThread::blockUpdateThread, this, clientSocket, global);
	blockUpdate.detach();

	extraNonce = global->getExtraNonce();

	socketError = false;
	while (!socketError) {

		const int tmpBuffLen = 4096;
		char tmpBuff[tmpBuffLen];
		memset(tmpBuff, 0, tmpBuffLen);
		int numRecv = recv(clientSocket, tmpBuff, tmpBuffLen, 0);
		if (numRecv > 0)
			for (int i = 0; i < numRecv; i++)
				buffer.push_back(tmpBuff[i]);


		//TODO - evaluate memory leaks due to return
		if (numRecv < 0) {
			socketError = true;
			return;
		}


		if (buffer.size() > 0) {
			string line;
			while (readLine(buffer, line)) {
				json msg = json::parse(line.c_str());
				const std::string& command = msg["command"];

				if (command == "auth") {
					wallet = msg["data"];
					authDone = true;
					sendDifficulty();
					sendCurrentBlock(clientSocket, global);
				}

				else if (command == "submit") {

				}
			}
		}


	}

}


void WorkerThread::sendDifficulty() {
	json diff;
	diff["command"] = "set_difficulty";
	diff["data"] = difficulty;
}

void WorkerThread::sendCurrentBlock(int clientSocket, Global* global) {
	global->lockBlockData.lock();	
	string data = global->currentBlock.dump();
	global->lockBlockData.unlock();

	int len = data.length();
	int sent = 0;
	while ((sent < len) && (!socketError)) {
		int numSent = send(clientSocket, data.c_str() + sent, len, 0);
		if (numSent <= 0)
			socketError = true;
		else
			sent += numSent;
	}

}