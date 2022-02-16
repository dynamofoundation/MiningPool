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

    time_t lastDiffCheck;
    time(&lastDiffCheck);

	while (!socketError) {
		if (authDone) {
			if (lastBlockHeightSent != global->currentBlockHeight) {
				sendCurrentBlock(clientSocket, global);
				lastBlockHeightSent = global->currentBlockHeight;
                lockNonceList.lock();
                nonceList.clear();
                lockNonceList.unlock();
			}

            time_t now;
            time(&now);
            if (now - lastDiffCheck >= 20) {
                lastDiffCheck = now;
                if (submitShareCount > 1) {
                    difficulty *= 2;
                    sendDifficulty(clientSocket);
                }
                if (submitShareCount == 0) {
                    difficulty = (difficulty * 3) / 4;
                    if (difficulty < 1)
                        difficulty = 1;
                    sendDifficulty(clientSocket);
                }
                submitShareCount = 0;
            }

		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

}


uint64_t BSWAP64(uint64_t x)
{
    return  ((x << 56) & 0xff00000000000000UL) |
        ((x << 40) & 0x00ff000000000000UL) |
        ((x << 24) & 0x0000ff0000000000UL) |
        ((x << 8) & 0x000000ff00000000UL) |
        ((x >> 8) & 0x00000000ff000000UL) |
        ((x >> 24) & 0x0000000000ff0000UL) |
        ((x >> 40) & 0x000000000000ff00UL) |
        ((x >> 56) & 0x00000000000000ffUL);
}


void WorkerThread::clientWorker(int clientSocket, Global *global) {

	Log::log("New client connection");
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
			uint32_t errorNum = GetLastError();
			Log::log("Client thread %d exiting with error %d", extraNonce, errorNum);
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
					sendExtraNonce(clientSocket);
					sendDifficulty(clientSocket);
					sendMiningWallet(clientSocket, global->settings);
					sendCurrentBlock(clientSocket, global);
				}

				else if (command == "submit") {
					string data = msg["data"];
                    uint32_t nonce;
                    string strNonce = data.substr(76 * 2, 8);
                    hex2bin((unsigned char*)&nonce, strNonce.c_str(), 4);

                    lockNonceList.lock();
                    vector<uint32_t>::iterator it;
                    it = find(nonceList.begin(), nonceList.end(), nonce);
                    if ((it == nonceList.end()) || (nonceList.size() == 0)) {
                        nonceList.push_back(nonce);
                        lockNonceList.unlock();

                        string hash = calcHash(data);

                        uint64_t localTarget = share_to_target(difficulty) * 65536;

                        unsigned char bHash[65];
                        hex2bin(bHash, hash.c_str(), 32);
                        uint64_t hash_int{};
                        memcpy(&hash_int, bHash, 8);
                        hash_int = htobe64(hash_int);

                        if (hash_int < localTarget) {
                            sendBlockStatus(clientSocket, global->settings, "accept");
                            global->db->addShare(wallet, hash);
                            submitShareCount++;
                            uint64_t networkTarget = BSWAP64(((uint64_t*)nativeTarget)[0]);
                            if (hash_int < networkTarget) {
                                json jResult = global->rpc->execRPC("{ \"id\": 0, \"method\" : \"submitblock\", \"params\" : [\"" + data + "\"] }", global->settings);

                                if (jResult["error"].is_null()) {
                                    /*
                                    //printf(" **** SUBMITTED BLOCK SOLUTION FOR APPROVAL!!! ****\n");
                                    getWork->reqNewBlockFlag = true;
                                    statDisplay->totalStats->share_count++;
                                    if (jResult["result"] == "high-hash")
                                        statDisplay->totalStats->rejected_share_count++;
                                        */
                                }
                                else {
                                    //printf("Submit block failed: %s.\n", jResult["error"]);
                                    //statDisplay->totalStats->rejected_share_count++;
                                }

                            }
                        }
                        else {
                            sendBlockStatus(clientSocket, global->settings, "high-hash");
                        }
                    }
                    else {
                        sendBlockStatus(clientSocket, global->settings, "duplicate");
                        lockNonceList.unlock();
                    }

				}
			}
		}


	}

}


void WorkerThread::sendString(int clientSocket, string data) {
	data = data + "\n";
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

void WorkerThread::sendDifficulty(int clientSocket) {
	json msg;
	msg["command"] = "set_difficulty";
	msg["data"] = difficulty;
	sendString(clientSocket, msg.dump());
}

void WorkerThread::sendExtraNonce(int clientSocket) {
	json msg;
	msg["command"] = "set_extranonce";
	msg["data"] = extraNonce;
	sendString(clientSocket, msg.dump());
}

void WorkerThread::sendMiningWallet(int clientSocket, Settings* settings) {
	json msg;
	msg["command"] = "set_mining_wallet";
	msg["data"] = settings->miningWallet;
	sendString(clientSocket, msg.dump());
}


void WorkerThread::sendBlockStatus(int clientSocket, Settings* settings, string status) {
    json msg;
    msg["command"] = "block_status";
    msg["data"] = status;
    sendString(clientSocket, msg.dump());
}

void WorkerThread::sendCurrentBlock(int clientSocket, Global* global) {
	global->lockBlockData.lock();	
	string blockData = global->currentBlock.dump();
    memcpy(nativeTarget, global->nativeTarget, 32);
    lockProgram.lock();
	string strProgram = global->currentBlock["data"]["program"];

    bool programDone = false;    
    std::stringstream stream(strProgram);
    std::string line;

    vProgram.clear();
    while (!programDone) {
        getline(stream, line);
        if (line.length() == 0)
            programDone = true;
        else {
            vProgram.push_back(line);
        }
    }
    vProgram.push_back("ENDPROGRAM");


    lockProgram.unlock();
	global->lockBlockData.unlock();

	sendString(clientSocket, blockData);
}




string WorkerThread::calcHash(string data) {
	string header = data.substr(0, 160);
	unsigned char blockHeader[80];
	hex2bin(blockHeader, header.c_str(), 80);

	for (int i = 0; i < 16; i++) {
		unsigned char swap = blockHeader[4 + i];
        blockHeader[4 + i] = blockHeader[35 - i];
        blockHeader[35 - i] = swap;
	}

    CSHA256 ctx;

    uint32_t iResult[8];


    //hex2bin(blockHeader, "40000000000000023864150D435518426A0E938EA34A6D3483AC82F08EBD19DE9498B8B8B828786343EAE185A6DD8F807831494C30C58A6B1A0D4B45B5462E378A2BD235C63EEC619694051D00000000", 80); 

    char cPrevBlockHash[65];
    bin2hex(cPrevBlockHash, &blockHeader[4], 32);

    char cMerkleRoot[65];
    bin2hex(cMerkleRoot, &blockHeader[36], 32);

    string prevBlockHash(cPrevBlockHash);
    string merkleRoot(cMerkleRoot);

    /*
    memset(blockHeader, 0, 80);
    prevBlockHash = "0000000000000000000000000000000000000000000000000000000000000000";
    merkleRoot = "0000000000000000000000000000000000000000000000000000000000000000";
    */

    ctx.Write(blockHeader, 80);
    ctx.Finalize((unsigned char*)iResult);


    int line_ptr = 0;       //program execution line pointer
    int loop_counter = 0;   //counter for loop execution
    unsigned int memory_size = 0;    //size of current memory pool
    uint32_t* memPool = NULL;     //memory pool

    int loop_line_ptr = 0;      //to mark return OPCODE for LOOP command
    unsigned int loop_opcode_count = 0;  //number of times to run the LOOP

    uint32_t temp[8];

    uint32_t prevHashSHA[8];
    uint32_t iPrevHash[8];
    parseHex(prevBlockHash, (unsigned char*)iPrevHash);


    ///////////////memset(iPrevHash, 0, 32);


    ctx.Reset();
    ctx.Write((unsigned char*)iPrevHash, 32);
    ctx.Finalize((unsigned char*)prevHashSHA);

    ////////////int c = 0;

    lockProgram.lock();
    vector<string> program = vProgram;
    lockProgram.unlock();

    while (line_ptr < program.size()) {
        std::istringstream iss(program[line_ptr]);
        std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{} };     //split line into tokens


        
        //printf("%s\n", program[line_ptr].c_str());
        //printf("%08X%08X%08X%08X%08X%08X%08X%08X\n",  iResult[0], iResult[1], iResult[2], iResult[3], iResult[4], iResult[5], iResult[6], iResult[7]);
        


        //simple ADD and XOR functions with one constant argument
        if (tokens[0] == "ADD") {
            uint32_t arg1[8];
            parseHex(tokens[1], (unsigned char*)arg1);
            for (int i = 0; i < 8; i++)
                iResult[i] += arg1[i];
        }

        else if (tokens[0] == "XOR") {
            uint32_t arg1[8];
            parseHex(tokens[1], (unsigned char*)arg1);
            for (int i = 0; i < 8; i++)
                iResult[i] ^= arg1[i];
        }

        //hash algo which can be optionally repeated several times
        else if (tokens[0] == "SHA2") {
            if (tokens.size() == 2) { //includes a loop count
                loop_counter = atoi(tokens[1].c_str());
                for (int i = 0; i < loop_counter; i++) {
                    if (tokens[0] == "SHA2") {
                        unsigned char output[32];
                        ctx.Reset();
                        ctx.Write((unsigned char*)iResult, 32);
                        ctx.Finalize(output);
                        memcpy(iResult, output, 32);
                    }
                }
            }

            else {                         //just a single run
                if (tokens[0] == "SHA2") {
                    unsigned char output[32];
                    ctx.Reset();
                    ctx.Write((unsigned char*)iResult, 32);
                    ctx.Finalize(output);
                    memcpy(iResult, output, 32);
                }
            }
        }

        //generate a block of memory based on a hashing algo
        else if (tokens[0] == "MEMGEN") {
            if (memPool != NULL)
                free(memPool);
            memory_size = atoi(tokens[2].c_str());
            memPool = (uint32_t*)malloc(memory_size * 32);
            for (int i = 0; i < memory_size; i++) {
                if (tokens[1] == "SHA2") {
                    unsigned char output[32];
                    ctx.Reset();
                    ctx.Write((unsigned char*)iResult, 32);
                    ctx.Finalize(output);
                    memcpy(iResult, output, 32);
                    memcpy(memPool + i * 8, iResult, 32);
                }
            }
        }

        //add a constant to every value in the memory block
        else if (tokens[0] == "MEMADD") {
            if (memPool != NULL) {
                uint32_t arg1[8];
                parseHex(tokens[1], (unsigned char*)arg1);

                for (int i = 0; i < memory_size; i++) {
                    for (int j = 0; j < 8; j++)
                        memPool[i * 8 + j] += arg1[j];
                }
            }
        }

        //add the sha of the prev block hash with every value in the memory block
        else if (tokens[0] == "MEMADDHASHPREV") {
            if (memPool != NULL) {
                for (int i = 0; i < memory_size; i++) {
                    for (int j = 0; j < 8; j++) {
                        memPool[i * 8 + j] += iResult[j];
                        memPool[i * 8 + j] += prevHashSHA[j];
                    }
                }
            }
        }


        //xor a constant with every value in the memory block
        else if (tokens[0] == "MEMXOR") {
            if (memPool != NULL) {
                uint32_t arg1[8];
                parseHex(tokens[1], (unsigned char*)arg1);

                for (int i = 0; i < memory_size; i++) {
                    for (int j = 0; j < 8; j++)
                        memPool[i * 8 + j] ^= arg1[j];
                }
            }
        }

        //xor the sha of the prev block hash with every value in the memory block
        else if (tokens[0] == "MEMXORHASHPREV") {
            if (memPool != NULL) {

                for (int i = 0; i < memory_size; i++) {
                    for (int j = 0; j < 8; j++) {
                        memPool[i * 8 + j] += iResult[j];
                        memPool[i * 8 + j] ^= prevHashSHA[j];
                    }
                }
            }
        }

        //read a value based on an index into the generated block of memory
        else if (tokens[0] == "READMEM") {
            if (memPool != NULL) {
                unsigned int index = 0;

                if (tokens[1] == "MERKLE") {
                    uint32_t arg1[8];
                    parseHex(merkleRoot, (unsigned char*)arg1);
                    index = arg1[0] % memory_size;
                    memcpy(iResult, memPool + index * 8, 32);
                }

                else if (tokens[1] == "HASHPREV") {
                    uint32_t arg1[8];
                    parseHex(prevBlockHash, (unsigned char*)arg1);
                    index = arg1[0] % memory_size;
                    memcpy(iResult, memPool + index * 8, 32);
                }
            }
        }

        else if (tokens[0] == "READMEM2") {
            if (memPool != NULL) {
                unsigned int index = 0;

                if (tokens[1] == "XOR") {
                    if (tokens[2] == "HASHPREVSHA2") {
                        for (int i = 0; i < 8; i++)
                            iResult[i] ^= prevHashSHA[i];

                        for (int i = 0; i < 8; i++)
                            index += iResult[i];

                        index = index % memory_size;
                        memcpy(iResult, memPool + index * 8, 32);
                    }
                }

                else if (tokens[1] == "ADD") {
                    if (tokens[2] == "HASHPREVSHA2") {
                        for (int i = 0; i < 8; i++)
                            iResult[i] += prevHashSHA[i];

                        for (int i = 0; i < 8; i++)
                            index += iResult[i];

                        index = index % memory_size;
                        memcpy(iResult, memPool + index * 8, 32);
                    }
                }
            }
        }


        else if (tokens[0] == "LOOP") {
            loop_line_ptr = line_ptr;
            loop_opcode_count = 0;
            for (int i = 0; i < 8; i++)
                loop_opcode_count += iResult[i];
            loop_opcode_count = loop_opcode_count % atoi(tokens[1].c_str()) + 1;
            //printf("loop %d\n", loop_opcode_count);
        }

        else if (tokens[0] == "ENDLOOP") {
            loop_opcode_count--;
            if (loop_opcode_count > 0)
                line_ptr = loop_line_ptr;
        }

        else if (tokens[0] == "IF") {
            uint32_t sum = 0;
            for (int i = 0; i < 8; i++)
                sum += iResult[i];
            if ((sum % atoi(tokens[1].c_str())) == 0)
                line_ptr++;
        }

        else if (tokens[0] == "STORETEMP") {
            for (int i = 0; i < 8; i++)
                temp[i] = iResult[i];
        }

        else if (tokens[0] == "EXECOP") {
            uint32_t sum = 0;
            for (int i = 0; i < 8; i++)
                sum += iResult[i];


            if (sum % 3 == 0) {
                for (int i = 0; i < 8; i++)
                    iResult[i] += temp[i];
            }
            else if (sum % 3 == 1) {
                for (int i = 0; i < 8; i++)
                    iResult[i] ^= temp[i];
            }
            else if (sum % 3 == 2) {
                unsigned char output[32];
                ctx.Reset();
                ctx.Write((unsigned char*)iResult, 32);
                ctx.Finalize(output);
                memcpy(iResult, output, 32);
            }
        }

        line_ptr++;



        ///printf("end   %08X%08X%08X%08X%08X%08X%08X%08X\n",  iResult[0], iResult[1], iResult[2], iResult[3], iResult[4], iResult[5], iResult[6], iResult[7] );



    }


    if (memPool != NULL)
        free(memPool);

    return makeHex((unsigned char*)iResult, 32);

}