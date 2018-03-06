#include "PartObBBN.h"
#include "FullObBBN.h"
#include <windows.h>

void partObBBN(string trainFile, string testFile, int flag) {
    //generateData(1000);
    if(flag == 0)
        learn(trainFile, "./cpt");
    else if(flag == 1)
        predict(testFile, "./cpt");
    else {
        learn(trainFile, "./cpt");
        Sleep(1000);
        predict(testFile, "./cpt");
    }
}

void fullObBBN(string trainFile, string testFile, int flag) {
    //fGenerateData(1000, "test");
    if(flag == 0)
        fLearn(trainFile, "./cpt_full");
    else if(flag == 1)
        fPredict(testFile, "./cpt_full");
    else {
        fLearn(trainFile, "./cpt_full");
        Sleep(1000);
        fPredict(testFile, "./cpt_full");
    }
}

map<string, string> getParameters(int argc, char **argv) {
    map<string, string> params;
    params["-r"] = "";
    params["-t"] = "";
    params["-p"] = "1";
    char *key = nullptr, *value = nullptr;
    for(int i = 0; i < argc;) {
        if(argv[i][0] == '-') {
            key = argv[i];
            if(argv[i][1] == 'h' || argv[i][1] == 'H') {
                params[key] = "";
                i++;
            }
            else {
                value = argv[i + 1];
                params[key] = value;
                i += 2;
            }
        }
        else
            i++;
    }
    return params;
}

int main(int argc, char **argv) {
    map<string, string> params = getParameters(argc, argv);
    if(params.empty()) {
        cout << "Parameter format error!" << endl;
        cout << "Input -h or -H for help." << endl;
        return 0;
    }
    if(params.find("-h") != params.end() || params.find("-H") != params.end()) {
        cout << "Help: " << endl;
        cout << "----------------------------------------------------" << endl;
        cout << "example: bbn -r xxx -t yyy -p 1" << endl;
        cout << "         bbn -r xxx -p 1" << endl;
        cout << "         bbn -t yyy -p 0" << endl;
        cout << "-r\ttraining file path" << endl;
        cout << "-t\ttest file path" << endl;
        cout << "-p\t1 if this bbn contains any hidden node, else 0" << endl;
        return 0;
    }
    string trainFile(params["-r"]),
            testFile(params["-t"]),
            isPartOb(params["-p"]);
    if(trainFile.empty() && testFile.empty()) {
        cout << "Parameter format error!" << endl;
        cout << "Input -h or -H for help." << endl;
        return 0;
    }
    clock_t start = clock();
    if(isPartOb == "1") {
        if(testFile.empty())
            partObBBN(trainFile, "", 0);
        else if(trainFile.empty())
            partObBBN("", testFile, 1);
        else
            partObBBN(trainFile, testFile, 2);
    }
    else {
        if(testFile.empty())
            fullObBBN(trainFile, "", 0);
        else if(trainFile.empty())
            fullObBBN("", testFile, 1);
        else
            fullObBBN(trainFile, testFile, 2);
    }
    clock_t end = clock();
    cout << endl;
    cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << "s" << endl;
    return 0;
}
