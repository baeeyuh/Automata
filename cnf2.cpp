#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
using namespace std;

// Structure to represent a grammar
struct Grammar {
    string startSymbol; // Starting nonterminal
    map<string, vector<vector<string>>> rules; // Nonterminal -> list of RHS rules
};

// Check if a symbol is a terminal (lowercase) or nonterminal (uppercase)
bool isTerminal(const string &s) { return s.size() == 1 && islower(s[0]); }
bool isNonTerminal(const string &s) { return s.size() == 1 && isupper(s[0]); }

// Step 1: Remove ε-productions (rules producing empty string)
void removeEpsilonProductions(Grammar &G) {
    set<string> nullable; // Nonterminals that can produce ε

    // Find nullable nonterminals
    for (auto &[lhs, rhss] : G.rules)
        for (auto &rhs : rhss)
            if (rhs.size() == 1 && rhs[0] == "ε") 
                nullable.insert(lhs);

    // For each nullable symbol, adjust all rules containing it
    for (auto &A : nullable) {
        for (auto &[lhs, rhss] : G.rules) {
            vector<vector<string>> newRules;
            for (auto rhs : rhss)
                for (size_t i = 0; i < rhs.size(); i++)
                    if (rhs[i] == A && rhs.size() > 1) { 
                        // Remove nullable symbol from RHS
                        vector<string> temp = rhs;
                        temp.erase(temp.begin() + i);
                        newRules.push_back(temp);
                    }
            // Add the new rules to the grammar
            rhss.insert(rhss.end(), newRules.begin(), newRules.end());
        }
        // Remove direct ε-productions
        auto &v = G.rules[A];
        v.erase(remove_if(v.begin(), v.end(), [](auto &r){ return r.size() == 1 && r[0] == "ε"; }), v.end());
    }

    // Keep ε for start symbol if it was nullable
    if (nullable.count(G.startSymbol))
        G.rules[G.startSymbol].push_back({"ε"});
}

// Step 2: Remove unit productions (A → B)
void removeUnitProductions(Grammar &G) {
    bool changed = true;

    // Repeat until no unit rules remain
    while (changed) {
        changed = false;
        for (auto &[lhs, rhss] : G.rules) {
            vector<vector<string>> toAdd;
            for (auto &rhs : rhss)
                if (rhs.size() == 1 && isNonTerminal(rhs[0])) {
                    string B = rhs[0];
                    // Add all rules from B to A (except self-loop)
                    for (auto &r2 : G.rules[B])
                        if (!(r2.size() == 1 && r2[0] == lhs)) 
                            toAdd.push_back(r2), changed = true;
                }
            rhss.insert(rhss.end(), toAdd.begin(), toAdd.end());

            // Remove the original unit productions
            rhss.erase(remove_if(rhss.begin(), rhss.end(), [](auto &r){ return r.size() == 1 && isNonTerminal(r[0]); }), rhss.end());
        }
    }
}

// Step 3: Replace terminals in mixed RHS with new variables
void replaceTerminalsInMixedRHS(Grammar &G) {
    map<string, string> terminalMap; // Map terminals to new variables
    int counter = 0;
    vector<string> nonterminals;
    for (auto &[lhs, _] : G.rules) nonterminals.push_back(lhs);

    for (auto &lhs : nonterminals)
        for (auto &rhs : G.rules[lhs])
            for (auto &sym : rhs)
                if (isTerminal(sym) && rhs.size() > 1) {
                    // Create a new variable for this terminal if it doesn't exist
                    if (!terminalMap.count(sym)) {
                        string newVar = "X" + to_string(++counter);
                        terminalMap[sym] = newVar;
                        G.rules[newVar].push_back({sym}); // Add X → terminal
                    }
                    sym = terminalMap[sym]; // Replace terminal with variable
                }
}

// Step 4: Binarize rules (ensure RHS has ≤ 2 symbols)
void binarizeGrammar(Grammar &G) {
    int binCount = 0;
    vector<string> nonterminals;
    for (auto &[lhs, _] : G.rules) nonterminals.push_back(lhs);

    for (auto &lhs : nonterminals) {
        vector<vector<string>> newRules;
        for (auto rhs : G.rules[lhs]) {
            // While RHS has more than 2 symbols, break it into binary rules
            while (rhs.size() > 2) {
                string newVar = "Y" + to_string(++binCount);
                vector<string> nextTwo(rhs.begin() + 1, rhs.begin() + 3); // Take 2 symbols
                G.rules[newVar].push_back(nextTwo); // Add new intermediate rule
                rhs.erase(rhs.begin() + 1, rhs.begin() + 3); // Remove from original RHS
                rhs.push_back(newVar); // Add new variable
            }
            newRules.push_back(rhs); // Add the final binary rule
        }
        G.rules[lhs] = newRules; // Update rules for this nonterminal
    }
}

// CNF conversion driver
void convertToCNF(Grammar &G) {
    removeEpsilonProductions(G);
    removeUnitProductions(G);
    replaceTerminalsInMixedRHS(G);
    binarizeGrammar(G);
}

// Utility: Print grammar rules
void printGrammar(const Grammar &G) {
    for (auto &[lhs, rhss] : G.rules) {
        cout << lhs << " → ";
        for (size_t i = 0; i < rhss.size(); i++) {
            for (auto &sym : rhss[i]) cout << sym;
            if (i != rhss.size() - 1) cout << " | ";
        }
        cout << endl;
    }
}

int main() {
    Grammar G;
    G.startSymbol = "S";

    // Example CFG
    G.rules["S"] = {{"A","S","B"}};
    G.rules["A"] = {{"a","A","S"},{"a"},{"ε"}};
    G.rules["B"] = {{"S","b","S"},{"A"},{"b","b"}};

    convertToCNF(G);  // Convert the CFG to CNF
    printGrammar(G);   // Print the CNF grammar

    return 0;
}
