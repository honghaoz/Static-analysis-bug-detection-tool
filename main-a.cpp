//
//  main.cpp
//  ece653projec-part1
//
//  Created by Zhang Honghao on 3/12/14.
//  Copyright (c) 2014 org-honghao. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <assert.h>
#include <stdlib.h>

using namespace std;

#define skipLine(ifile) ifile.ignore(200, '\n')

class FunctionCall {
public:
    int functionId;
    string functionName;
    int supports;
    
    // callers stores funtionIds that call this function
    set<int> callers;
    FunctionCall(int id = -1, int sup = -1) {
        functionId = id;
        supports = sup;
    }
    ~FunctionCall() {
        callers.clear();
    }
};

class CallGraph {
private:
    // id is a counter, used to set id of a function
    int id;
    // map_nameToId: key is function name and value is funtion Id
    map<string, int> map_nameToId;
    // map_IdToName: key is function Id and value is funtion name
    map<int, string> map_IdToName;
    // map_functionCall: key is function Id and value is FunctionCall object
    map<int, FunctionCall> map_functionCall;
public:
    CallGraph (){
        id = 1;
    }
    ~CallGraph () {
        map_nameToId.clear();
        map_IdToName.clear();
        map_functionCall.clear();
    }
    // reset() is used for reset the CallGraph
    //  PRE:  true
    //  POST: set id to 1 clear three maps
    void reset();
    
    // readFunctionName(ifstream *ifile, string *function) is used for reading function name from next line
    //  PRE:  ifile and function are vaild
    //  POST: if function name exists, function store the name of function, otherwise, function = "".
    //        ifile read to next line
    //        Return true if read successfully, otherwise, retrun false.
    bool readFunctionName(ifstream *ifile, string *function);
    
    // readFile(ifstream *ifile) is used for reading call graph text and initialize three maps
    //  PRE:  ifile is vaild
    //  POST: map_nameToId stores key value pair: function name and Id,
    //        map_IdToName stores key value pair: function Id and name,
    //        map_functionCall stores every function's information
    void readFile(ifstream *ifile);
    
    // processFunctionPair(int t_support, double t_confidence) used to set support for each functionCall and pair every two function and call findBugs
    //  PRE:  readFunctionName is called and data is read successfully
    //  POST: support of every function is set and call findBugs
    void processFunctionPair(int t_support, double t_confidence);
    
    // findBugs(FunctionCall *f1, FunctionCall *f2, int t_support, double t_confidence) used to find bugs for f1 and f2 with threshold value of support and confidence
    // PRE:  f1 and f2 are vaild
    // POST: print out bugs for pair f1 and f2
    void findBugs(FunctionCall *f1, FunctionCall *f2, int t_support, double t_confidence);
};

// see declaration for documentation
void CallGraph::reset() {
    id = 1;
    map_nameToId.clear();
    map_IdToName.clear();
    map_functionCall.clear();
}

// see declaration for documentation
bool CallGraph::readFunctionName(ifstream *ifile, string *function) {
    assert(ifile != NULL);
    assert(function != NULL);
    string aLine;
    int left, right;
    getline(*ifile, aLine);
    left = (int)aLine.find('\'');
    right = (int)aLine.rfind('\'');
    if (left == aLine.npos) {
        return false;
    }
    *function = aLine.substr(left + 1, right - left - 1);
    return true;
}

// see declaration for documentation
void CallGraph::readFile(ifstream *ifile) {
    assert(ifile != NULL);
    if ((*ifile).is_open()) {
        // before reading call graph, need to make sure three map is empty
        reset();
        // temporary string variables
        string firstWord, secondWord, aLine, tempS, function;
        while (!(*ifile).eof()) {
            // read first two words
            (*ifile) >> firstWord >> secondWord;
            // "Call graph" section
            if (firstWord == "Call" && secondWord == "graph") {
                (*ifile) >> tempS >> tempS;
                // this function is null function, skip
                if (tempS == "<<null") {
                    continue;
                }
                // this function is normal function
                else if (tempS == "for"){
                    // if read function name is successfully
                    if (readFunctionName(ifile, &function)) {
                        // Check whether this function has a Id
                        int functionID = map_nameToId[function];
                        // if id is 0, this function is a new function, add new function
                        if (functionID == 0) {
                            map_IdToName[id] = function;
                            map_nameToId[function] = id;
                            functionID = id++;
                        }
                        // get FunctionCall object for this functionID
                        FunctionCall &aFunctionCall = map_functionCall[functionID];
                        aFunctionCall.functionId = functionID;
                        aFunctionCall.functionName = function;
                        string callee;
                        // process callee functions of this function
                        // peek the first char
                        while ((*ifile).peek() == ' ') {
                            // if read callee function name is successfuly
                            if (readFunctionName(ifile, &callee)){
                                // Check whether this function has a Id
                                int calleeID = map_nameToId[callee];
                                // if id is 0, this function is a new function, add new function
                                if (calleeID == 0) {
                                    map_IdToName[id] = callee;
                                    map_nameToId[callee] = id;
                                    calleeID = id++;
                                }
                                // get the FunctionCall for this calleeID and set id, name and callers
                                FunctionCall &calleeFunctionCall = map_functionCall[calleeID];
                                calleeFunctionCall.functionId = calleeID;
                                calleeFunctionCall.functionName = callee;
                                calleeFunctionCall.callers.insert(functionID);
                            }
                        }
                    }
                }
                // never reached
                else {
                    assert(false);
                }
                continue;
            }
            // not "Call graph" section, skip
            else {
                skipLine((*ifile));
            }
        }
    }
    else {
        cout << "Call graph file open failed!\n";
    }
}

// see declaration for documentation
void CallGraph::processFunctionPair(int t_support, double t_confidence) {
    // pair every two functions
    for (map<int, FunctionCall>::iterator it1 = map_functionCall.begin(); it1 != map_functionCall.end(); it1++) {
        // set support of the first functionCall
        if (it1 == map_functionCall.begin()) {
            it1->second.supports = (int)it1->second.callers.size();
        }
        for (map<int, FunctionCall>::iterator it2(it1); it2 != map_functionCall.end();) {
            it2++;
            if (it2 == map_functionCall.end()) {
                break;
            }
            // set supports for rest functionCall, only set once for each functionCall
            if (it1 == map_functionCall.begin()) {
                it2->second.supports = (int)it2->second.callers.size();
            }
            // pair two functions, if these two function both have none 0 support, findBugs for this pair
            if (it1->second.supports != 0 && it2->second.supports != 0) {
                findBugs(&(it1->second), &(it2->second), t_support, t_confidence);
            }
        }
    }
}

// see declaration for documentation
void CallGraph::findBugs(FunctionCall *f1, FunctionCall *f2, int t_support, double t_confidence) {
    assert(f1 != NULL);
    assert(f2 != NULL);
    // result set, used for storing intersection of callers of f1 and f2
    set<int> funtionsIntersection;
    set_intersection(f1->callers.begin(), f1->callers.end(), f2->callers.begin(), f2->callers.end(), inserter(funtionsIntersection, funtionsIntersection.end()));
    // calculate support for f1 f2 pair
    int f1f2_supports = (int)funtionsIntersection.size();
    
    // if bug's support is above threshold
    if (f1f2_supports >= t_support && f1->supports > 0 && f2->supports > 0) {
        // f1Name and f2Name is used to print pair: (f1, f2)
        string f1Name = f1->functionName;
        string f2Name = f2->functionName;
        
        // sort the pair function names alphabetically
        if (strcmp(f1Name.c_str(), f2Name.c_str()) > 0) {
            string tempName = f1Name;
            f1Name = f2Name;
            f2Name = tempName;
        }
        
        // calculate confidence for f1 f2 pair / f1
        float confidence = (float)f1f2_supports / f1->supports;
        // functionIds stores difference between callers of f1 and f2
        set<int> functionIds;
        // if bug's confidence is above threshold
        if (confidence >= t_confidence) {
            set_difference(f1->callers.begin(), f1->callers.end(), f2->callers.begin(), f2->callers.end(), inserter(functionIds, functionIds.end()));
            // print out bugs
            for (set<int>::iterator it = functionIds.begin(); it != functionIds.end(); it++) {
                printf("bug: %s in %s, pair: (%s, %s), support: %d, confidence: %.2f%%\n", f1->functionName.c_str(), map_IdToName[*it].c_str(), f1Name.c_str(), f2Name.c_str(), f1f2_supports, confidence * 100);
            }
        }
        // calculate confidence for f1 f2 pair / f2
        confidence = (float)f1f2_supports / f2->supports;
        // if bug's confidence is above threshold
        if (confidence >= t_confidence) {
            functionIds.clear();
            set_difference(f2->callers.begin(), f2->callers.end(), f1->callers.begin(), f1->callers.end(), inserter(functionIds, functionIds.end()));
            // print out bugs
            for (set<int>::iterator it = functionIds.begin(); it != functionIds.end(); it++) {
                printf("bug: %s in %s, pair: (%s, %s), support: %d, confidence: %.2f%%\n", f2->functionName.c_str(), map_IdToName[*it].c_str(), f1Name.c_str(), f2Name.c_str(), f1f2_supports, confidence * 100);
            }
        }
    }
}

int main(int argc, char *argv[]){
    // arguments
    char *bc_file;
    int support = 3;
    double confidence = 65;
    
    // read arguments with bc file, supports and confidence
    bc_file = argv[1];
    if (argc > 2) {
        support = atoi(argv[2]);
        confidence = atof(argv[3]);
    }
    confidence /= 100;

    // output the call graph to "**_cg.txt"
    string cs_bc_file(bc_file);
    string call_graph_file_name = cs_bc_file.substr(0, cs_bc_file.rfind('.')) + "_cg.txt";
    
    char *command = (char *)malloc(sizeof(char) * (strlen("opt -print-callgraph  2> >/dev/null") + strlen(bc_file) + strlen(call_graph_file_name.c_str())) + 1);
    sprintf(command, "opt -print-callgraph %s 2>%s >/dev/null", bc_file, call_graph_file_name.c_str());
    system(command);
    free(command);
    
    // new callGraph
    CallGraph aCallGraph = CallGraph();
    // open call graph text file generated before
    ifstream ifile(call_graph_file_name.c_str());
    
    // read from call graph text file
    aCallGraph.readFile(&ifile);
    // process pairs and find bugs
    aCallGraph.processFunctionPair(support, confidence);
    
    // close file
    ifile.close();
    
    return 0;
}
