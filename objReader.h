#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <array>
using namespace std;

//returns a vector of 4 v3 arrays, the first 3 v3s are the verticies of a triangle, the 4th is it's normal vector
vector<array<array<float,3>,4>> objReader(string fileName) {
    vector<array<array<float,3>,4>> tris;
    vector<array<float,3>> vecs;
    vector<array<float,3>> norms;
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

        if (word[0] == 'v' && word[1] == 'n') {
            // printf("found a v");
            array<float,3> v;
            for (int i = 0; i < 3; i++) {
                MyReadFile >> word;
                // printf("%s\n",word.c_str());
                v[i] = stof(word,NULL);
            }
            // printf("(%f,%f,%f)\n",v[0],v[1],v[2]);
            norms.push_back(v);
        }

        if (word[0] == 'f') {
            array<array<float,3>,4> t;
            //determine the verticies of the triangle
            for (int i = 0; i < 3; i++) {
                MyReadFile >> word;
                // printf("%s\n",word.c_str());
                int vertIndex = stoi(word.substr(0,word.find_first_of('/')),NULL) - 1;
                // printf("got /\n");
                t[i][0] = vecs[vertIndex][0];
                t[i][1] = vecs[vertIndex][1];
                t[i][2] = vecs[vertIndex][2];
            }
            //determine the normal vector of the triangle
            // printf("%s ", word.substr(word.find_last_of('/')+1).c_str());
            int normIndex = stoi(word.substr(word.find_last_of('/')+1),NULL) - 1;
            t[3][0] = norms[normIndex][0];
            t[3][1] = norms[normIndex][1];
            t[3][2] = norms[normIndex][2];
            tris.push_back(t);
        }
    }
    
    // for (array<float,3> v : norms) {
    //     printf("(%f,%f,%f)\n",v[0],v[1],v[2]);
    // }

    return tris;
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
