#include "WebPack.h"



void WebPack::save() {

    string folder = "c:/webpack/";

    std::vector<std::string> files;
    std::list<std::string> subfolders;
    subfolders.push_back(folder);

    while (!subfolders.empty()) {
        std::string current_folder(subfolders.back());

        if (*(current_folder.end() - 1) != '/') {
            current_folder.append("/*");
        }
        else {
            current_folder.append("*");
        }

        subfolders.pop_back();

        struct _finddata_t file_info;
        auto file_handler = _findfirst(current_folder.c_str(), &file_info);

        while (file_handler != -1) {
            if ( (!strcmp(file_info.name, ".") || !strcmp(file_info.name, ".."))) {
                if (_findnext(file_handler, &file_info) != 0) break;
                continue;
            }

            if (file_info.attrib & _A_SUBDIR) {
                // will search sub folder
                std::string folder(current_folder);
                folder.pop_back();
                folder.append(file_info.name);

                subfolders.push_back(folder.c_str());
            }
            else {
                // it's a file
                std::string file_path;
                // current_folder.pop_back();
                file_path.assign(current_folder.c_str()).pop_back();
                file_path.append(file_info.name);

                files.push_back(file_path);
            }

            if (_findnext(file_handler, &file_info) != 0) break;
        }  // while
        _findclose(file_handler);
    }



    int outfile = _open("webpack.dat", _O_RDWR | _O_CREAT | _O_BINARY | _O_TRUNC, _S_IWRITE | _S_IREAD);
    for (int i = 0; i < files.size(); i++) {
        string name = files[i].substr(10);
        int fnlen = name.size();
        _write (outfile, &fnlen, 4);
        _write(outfile, name.c_str(), name.size());
        int datfile = _open(files[i].c_str(), _O_BINARY);
        _lseek(datfile, 0, SEEK_END);
        int size = _tell(datfile);
        _write(outfile, &size, 4);
        _lseek(datfile, 0, SEEK_SET);
        unsigned char buff[4096];
        int totalread = 0;
        while (totalread < size) {
            int numread = _read(datfile, buff, 4096);
            _write(outfile, buff, numread);
            totalread += numread;
        }        
        _close(datfile);
    }
    _close(outfile);
}


void WebPack::load() {

    //max size of any one file is 1mb
    char* buff = (char*)malloc(1024 * 1024);

    int infile = _open("webpack.dat", _O_BINARY);
    if (infile == -1)
        Log::fatalError("Cannot open ewbpack.dat");

    _lseek(infile, 0, SEEK_END);
    int size = _tell(infile);
    _lseek(infile, 0, SEEK_SET);
    int ptr = 0;
    while (ptr < size) {
        int fnlen;
        _read(infile, &fnlen, 4);
        _read(infile, buff, fnlen);
        buff[fnlen] = 0;
        string strName(buff);
        int datlen;
        _read(infile, &datlen, 4);
        _read(infile, buff, datlen);
        sFile newFile;
        newFile.data = (char*)malloc(datlen);
        memcpy(newFile.data, buff, datlen);
        newFile.len = datlen;
        pages.emplace(strName, newFile);
        ptr += 4 + fnlen + 4 + datlen;
    }
    _close(infile);

    free(buff);
}