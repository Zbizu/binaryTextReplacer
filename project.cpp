/*
    Text replacer 1.0
    replaces the specified text in files using a set of instructions

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdlib.h>

using namespace std;

//print line of text
void pLine (auto param) {
    cout << param << endl;
}

enum tokenizerState {
    STATE_NEWCOMMAND,
    STATE_ERROR,

    STATE_LINECOMMENT,
    STATE_EXPECTDEFINE,
    STATE_EXPECTSTRING,
    STATE_READSTRING,
};

//see last character in buffer
char peek_buffer(string line) {
    if (line.length() > 0) {
        return line[line.length() - 1];
    }
}

//translate next char
tokenizerState getNextState(char symbol, string buffer, tokenizerState currentState) {
    if (currentState == STATE_LINECOMMENT) {
        //process the comment
        return currentState;
    } else if (currentState == STATE_READSTRING) {
        //read whatever is inside ""
        if (symbol == '"' && peek_buffer(buffer) != '\\') {
            //react to unescaped "
            return STATE_NEWCOMMAND;
        } else {
            //continue reading string
            return currentState;
        }
    } else if (currentState == STATE_NEWCOMMAND) {
        //read variable name
        switch(symbol) {
            case ' ':
                //expect ":" next
                return STATE_EXPECTDEFINE;
            case ':':
                //expect a string next
                return STATE_EXPECTSTRING;
            default:
                return currentState;
        }
    } else if (currentState == STATE_EXPECTDEFINE) {
        //escape spaces before ":"
        switch(symbol) {
            case ' ':
                return currentState;
            case ':':
                return STATE_EXPECTSTRING;
            default:
                return STATE_ERROR;
        }
    } else if (currentState == STATE_EXPECTSTRING) {
        //escape spaces before "
        switch(symbol) {
            case ' ':
                return currentState;
            case '"':
                return STATE_READSTRING;
            default:
                return STATE_ERROR;
        }
    }

    return STATE_ERROR;
}

//to do: vector of instructions as a stack
//to do: second stage that will build concrete instruction (or just interpret)
//instructions
vector< pair<string,string> > instructionStack;

//build the instructions
void tokenize (string line) {
    stringstream buffer;
    tokenizerState state = STATE_NEWCOMMAND;
    char symbol;

    //to do: stack
    //instruction
    string key;
    string value;

    for (auto i = 0; i < line.size(); i++) {
        symbol = line[i];
        tokenizerState newState = getNextState(symbol, buffer.str(), state);

        //handle comment
        if (state != STATE_READSTRING && symbol == '-' && i+1 < line.size() && line[i+1] == '-') {
            return;
        }

        switch(state) {
            case STATE_NEWCOMMAND:
                //read the command
                if (newState == state){
                    //add to buffer
                    buffer << symbol;
                } else {
                    //push the variable
                    key = buffer.str();

                    //clear the buffer
                    buffer.str("");
                }
                break;
            case STATE_EXPECTDEFINE:
                //skip spaces and : symbol
                break;
            case STATE_EXPECTSTRING:
                //skip spaces and "
                break;
            case STATE_READSTRING:
                //read string
                 if (newState == state){
                    //add to buffer
                    buffer << symbol;
                    } else {
                        //push the variable
                        value = buffer.str();

                        //clear the buffer
                        buffer.str("");

                        //build the instruction
                        pair<string,string> command;
                        command.first = key;
                        command.second = value;

                        //push the instruction
                        instructionStack.push_back(command);
                    }
                break;
            case STATE_LINECOMMENT:
                //ignore the comment
                return;
            case STATE_ERROR:
                return;
            default:
                break;
        }

        state = newState;
    }
}

enum replaceOption {
    REPLACE_NONE,
    REPLACE_NORMAL,
    REPLACE_OFFSET
};

//main
int main(int argc, char *argv[]) {
    pLine("==========================================================================");
    pLine("Text replacer 1.0");
    pLine("==========================================================================");
    pLine("This program is free software; you can redistribute it and/or modify");
    pLine("it under the terms of the GNU General Public License as published by");
    pLine("the Free Software Foundation; either version 2 of the License, or");
    pLine("(at your option) any later version.");
    pLine("");
    pLine("This program is distributed in the hope that it will be useful,");
    pLine("but WITHOUT ANY WARRANTY; without even the implied warranty of");
    pLine("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the");
    pLine("GNU General Public License for more details.");
    pLine("");
    pLine("You should have received a copy of the GNU General Public License along");
    pLine("with this program; if not, write to the Free Software Foundation, Inc.,");
    pLine("51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.");
    pLine("==========================================================================");

    if(argc == 1) {
        //no arguments given
        pLine("Welcome!");
        cout << "To run the program use script name as an argument, for example:" << endl << argv[0] << " exampleScript.txt" << endl << endl;
        pLine("example can be found in exampleScript.txt");
    } else {
        //loading the script
        cout << "Loading " << argv[1] << "...";

        ifstream scriptFile;
        scriptFile.open(argv[1], ios::binary);

        if(scriptFile) {
            cout << " [Done!]" << endl;

            string line;
            while(std::getline(scriptFile, line)) {
                //translate the script lines
                tokenize(line);
            }

            scriptFile.close();

            //understand the loaded script
            string fileName;
            replaceOption option;
            string replaceAmount;
            string replaceKey;
            string newValue;

            vector< pair<string, string> > regularReplace;
            vector< pair<int, pair<string, string>> > amountReplace;

            for ( const pair<string,string> &command : instructionStack ) {
                if (command.first == "fileName") {
                    fileName = command.second;
                } else if (command.first == "replace") {
                    option = REPLACE_NORMAL;
                    replaceKey = command.second;

                    stringstream newKey;
                    if (replaceKey.find("%%%%") == 0) {
                        size_t headPos = 4;
                        size_t endPos = replaceKey.size();

                        while (headPos < endPos) {
                            stringstream converter;
                            unsigned int converted;
                            converter << std::hex << replaceKey.substr(headPos, 2);
                            converter >> converted;
                            converter.str("");
                            newKey << static_cast<char>(converted);
                            headPos += 3;
                        }
                        replaceKey = newKey.str();
                    }
                } else if (command.first == "replaceChars") {
                    option = REPLACE_OFFSET;
                    replaceAmount = command.second;
                } else if (command.first == "at") {
                    if(option != REPLACE_OFFSET) {
                        pLine("Error: \"at\" is not supported for non-offset replace. Program will exit now.");
                        return 0;
                    }

                    replaceKey = command.second;
                } else if (command.first == "to") {
                        newValue = command.second;

                        //add to stack
                        if (option == REPLACE_NORMAL) {
                            stringstream newKey;
                            if (newValue.find("%%%%") == 0) {
                                size_t headPos = 4;
                                size_t endPos = newValue.size();

                                while (headPos < endPos) {
                                    stringstream converter;
                                    unsigned int converted;
                                    converter << std::hex << newValue.substr(headPos, 2);
                                    converter >> converted;
                                    converter.str("");
                                    newKey << static_cast<char>(converted);
                                    headPos += 3;
                                }
                                newValue = newKey.str();
                            }

                            regularReplace.push_back(make_pair(replaceKey, newValue));
                        } else if (option == REPLACE_OFFSET) {
                            int amount = atoi(replaceAmount.c_str());
                            if (amount > 0) {
                                pair<string, string> rule;
                                rule.first = replaceKey;
                                rule.second = newValue;

                                amountReplace.push_back(make_pair(amount, rule));
                            } else {
                                pLine("Error: invalid amount to replace. Program will exit now.");
                                return 0;
                            }
                        } else {
                            pLine("Error: unsupported replace type. Program will exit now.");
                            return 0;
                        }

                        //clear
                        option = REPLACE_NONE;
                        replaceAmount = "";
                        replaceKey = "";
                        newValue = "";
                }
            }

            //run the loaded script
            if (!fileName.empty()) {
                cout << "Loading " << fileName << "...";

                //check if we can write
                stringstream outputFileName;

                //filename
                outputFileName << fileName.substr(0, fileName.find("."));

                //filename_output
                outputFileName << "_output";

                //filename_output.txt
                outputFileName << fileName.substr(fileName.find("."), -1);

                ofstream outputFile(outputFileName.str(), ios::binary);
                if (!outputFile) {
                    pLine("Error: Write access denied. Program will exit now.");
                    return 0;
                }

                ifstream workingFile;
                workingFile.open(fileName, ios::binary);

                if(workingFile) {
                    cout << " [Done!]" << endl;

                    //iterate over file lines
                    bool firstLine = true;
                    while(std::getline(workingFile, line)) {
                        //iterate over vectors to find things to replace
                        size_t found;

                        //regular replace
                        for ( const pair<string,string> &command : regularReplace ) {
                            int starting = command.first.length();
                            int ending = command.second.length();

                            if (starting >= ending) {
                                //match first and replace
                                found = line.find(command.first);
                                if (found!=std::string::npos) {
                                    //pad missing amount with spaces
                                    string replacement = command.second;
                                    if (ending < starting) {
                                        stringstream ss;
                                        ss << command.second;
                                        for (int i = ending; i < starting; i++) {
                                            ss << " ";
                                        }

                                        replacement = ss.str();
                                    }

                                    line = line.replace(found, starting, replacement);
                                    //cout << line << endl;
                                }
                            } else {
                                cout << "Error: output string longer than the string to replace. Program will exit now." << endl;
                                return 0;
                            }
                        }

                        //replace value
                        for ( const pair<int, pair<string, string>> &command : amountReplace ) {
                            if (command.first > 0) {
                                int baseLength = command.second.second.length();
                                int targetLength = command.first;

                                if (targetLength >= baseLength) {
                                    pair<string, string> details = command.second;

                                    //match first and replace the selected amount of chars after selected string
                                    found = line.find(details.first);
                                    if (found!=std::string::npos) {
                                        //pad missing amount with spaces
                                        string replacement = details.second;
                                        if (baseLength < targetLength) {
                                            stringstream ss;
                                            ss << details.second;
                                            for (int i = baseLength; i < targetLength; i++) {
                                                ss << " ";
                                            }

                                            replacement = ss.str();
                                        }

                                        cout << details.first << replacement << endl;
                                        line = line.replace(found + details.first.length(), targetLength, replacement);
                                    }
                                } else {
                                    cout << "Error: the string to replace is longer than replaceChars. Program will exit now." << endl;
                                    return 0;
                                }
                            } else {
                                cout << "Error: replaceChars value must be a number. Program will exit now." << endl;
                                return 0;
                            }
                        }
                        if(firstLine) {
                            firstLine = false;
                        } else {
                            outputFile << endl;
                        }
                        outputFile << line;
                    }
                    outputFile.close();
                    workingFile.close();
                } else {
                    cout << " File not found!" << endl;
                }
            } else {
                pLine("Error: Input file was not declared.");
            }
        } else {
            cout << " File not found!" << endl;
        }
    }

    return 0;
}
