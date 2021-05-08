#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <array>
using namespace std;

vector<array<array<float,3>,3>> objReader(string fileName) {
    vector<array<array<float,3>,3>> tris;
    vector<array<float,3>> vecs;
    string word;
    ifstream MyReadFile(fileName);
    while (MyReadFile >> word) {
        // printf("%c\n",word[0]);
        if (word[0] == 'v' && word.length() == 1) {
            // printf("found a v");
            array<float,3> v;
            for (int i = 0; i < 3; i++) {
                MyReadFile >> word;
                // printf("%s\n",word.c_str());
                v[i] = stof(word,NULL);
            }
            // printf("(%f,%f,%f)\n",v[0],v[1],v[2]);
            vecs.push_back(v);
        }

        if (word[0] == 'f') {
            array<array<float,3>,3> t;
            for (int i = 0; i < 3; i++) {
                MyReadFile >> word;
                // printf("%s\n",word.c_str());
                int vertIndex = stoi(word.substr(0,word.find_first_of('/')),NULL) - 1;
                // printf("got /\n");
                t[i][0] = vecs[vertIndex][0];
                t[i][1] = vecs[vertIndex][1];
                t[i][2] = vecs[vertIndex][2];
            }
            tris.push_back(t);
        }
    }
    return tris;
    // for (array<float,3> v : vecs) {
    //     printf("(%f,%f,%f)\n",v[0],v[1],v[2]);
    // }

    // for (array<array<float,3>,3> t : tris) {
    //     printf("--------\n");
    //     for(int i = 0; i < 3; i++) {
    //         printf("--(");
    //         for (int j = 0; j < 3; j++) {
    //             printf(" %f ",t[i][j]);
    //         }
    //         printf(")--");
    //     }
    //     printf("\n");
    // }
}