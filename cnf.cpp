#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
#include <cctype>
using namespace std;

// Structure representing a grammar
struct Grammar
{
    string startSymbol;                        // Starting nonterminal (e.g., "S")
    map<string, vector<vector<string>>> rules; // Each LHS → list of RHS vectors
};

// Check if symbol is terminal (lowercase letter)
bool isTerminal(const string &s) { return s.size() == 1 && islower(s[0]); }

// Check if symbol is nonterminal (uppercase letter)
bool isNonTerminal(const string &s) { return s.size() == 1 && isupper(s[0]); }

// Prints grammar in readable format
void printGrammar(const Grammar &G, const string &title = "")
{
    if (!title.empty())
        cout << "\n"
             << title << endl;
    for (auto &[lhs, rhss] : G.rules)
    {
        cout << lhs << " → ";
        for (size_t i = 0; i < rhss.size(); i++)
        {
            for (auto &sym : rhss[i])
                cout << sym;
            if (i != rhss.size() - 1)
                cout << " | ";
        }
        cout << endl;
    }
}

// Step 1: Remove ε-Productions =====
void removeEpsilonProductions(Grammar &G)
{
    set<string> nullable; // Stores variables that can derive ε

    // Step 1: Identify nullable variables (A → ε)
    for (auto &[lhs, rhsList] : G.rules)
        for (auto &rhs : rhsList)
            if (rhs.size() == 1 && rhs[0] == "ε")
                nullable.insert(lhs);

    // Step 2: For each nullable variable A, create new rules
    for (auto &A : nullable)
    {
        for (auto &[lhs, rhsList] : G.rules)
        {
            vector<vector<string>> newRules;

            // For each rule, try removing occurrences of A
            for (auto rhs : rhsList)
            {
                for (size_t i = 0; i < rhs.size(); i++)
                {
                    if (rhs[i] == A && rhs.size() > 1)
                    {
                        vector<string> temp = rhs;
                        temp.erase(temp.begin() + i); // Remove nullable symbol
                        newRules.push_back(temp);     // Add modified rule
                    }
                }
            }

            // Add the newly formed rules
            for (auto &r : newRules)
                G.rules[lhs].push_back(r);
        }

        // Step 3: Remove ε directly from A's rules (except start symbol)
        auto &v = G.rules[A];
        v.erase(remove_if(v.begin(), v.end(),
                          [](auto &r)
                          { return r.size() == 1 && r[0] == "ε"; }),
                v.end());
    }

    // Step 4: Keep ε only for start symbol if it was nullable
    if (nullable.count(G.startSymbol))
        G.rules[G.startSymbol].push_back({"ε"});

    printGrammar(G, "Step 1: Remove ε-Productions");
}

// Step 2: Remove Unit Productions (A → B) =====
void removeUnitProductions(Grammar &G)
{
    bool changed = true;

    // Repeat until no unit rules remain
    while (changed)
    {
        changed = false;

        // Iterate through each nonterminal
        for (auto &[lhs, rhsList] : G.rules)
        {
            vector<vector<string>> toAdd; // Rules to add from unit expansions

            for (auto &rhs : rhsList)
            {
                // Identify unit productions (single uppercase symbol)
                if (rhs.size() == 1 && isNonTerminal(rhs[0]))
                {
                    string B = rhs[0]; // Example: A → B
                    // Add all rules from B into A
                    for (auto &r2 : G.rules[B])
                    {
                        if (!(r2.size() == 1 && r2[0] == lhs))
                        { // Avoid self-loop
                            toAdd.push_back(r2);
                            changed = true;
                        }
                    }
                }
            }

            // Add expanded rules
            rhsList.insert(rhsList.end(), toAdd.begin(), toAdd.end());

            // Remove all direct unit productions (A → B)
            rhsList.erase(remove_if(rhsList.begin(), rhsList.end(),
                                    [](auto &r)
                                    { return r.size() == 1 && isNonTerminal(r[0]); }),
                          rhsList.end());
        }
    }

    printGrammar(G, "Step 2: Remove Unit Productions");
}

// Step 3: Replace Terminals in Mixed RHS =====
void replaceTerminalsInMixedRHS(Grammar &G)
{
    map<string, string> terminalMap; // Maps terminals to new variables (e.g., a → X1)
    int counter = 0;

    // Copy keys to avoid modifying while iterating
    vector<string> nonterminals;
    for (auto &[lhs, _] : G.rules)
        nonterminals.push_back(lhs);

    // Iterate through all rules
    for (auto &lhs : nonterminals)
    {
        for (auto &rhs : G.rules[lhs])
        {
            for (auto &sym : rhs)
            {
                // Replace terminal if it appears with other symbols
                if (isTerminal(sym) && rhs.size() > 1)
                {
                    if (!terminalMap.count(sym))
                    {
                        // Create new variable for this terminal
                        string newVar = "X" + to_string(++counter);
                        terminalMap[sym] = newVar;
                        G.rules[newVar].push_back({sym}); // Add rule X1 → a
                    }
                    sym = terminalMap[sym]; // Replace 'a' with 'X1'
                }
            }
        }
    }

    printGrammar(G, "Step 3: Replace Terminals in Mixed RHS");

    // Print summary of generated variables
    if (!terminalMap.empty())
    {
        cout << "(Generated terminal variables: ";
        for (auto &[t, var] : terminalMap)
            cout << var << "=" << t << " ";
        cout << ")\n";
    }
}

// Step 4: Binarize Rules (Limit RHS to 2 Symbols) =====
void binarizeGrammar(Grammar &G)
{
    int binCount = 0; // Counter for new intermediate variables
    vector<string> nonterminals;

    // Copy keys to prevent modifying while iterating
    for (auto &[lhs, _] : G.rules)
        nonterminals.push_back(lhs);

    for (auto &lhs : nonterminals)
    {
        vector<vector<string>> newRules;

        for (auto rhs : G.rules[lhs])
        {
            // If rule has more than 2 symbols (e.g., A → B C D)
            while (rhs.size() > 2)
            {
                // Create a new variable (e.g., Y1)
                string newVar = "Y" + to_string(++binCount);

                // Next two symbols after the first (C D)
                vector<string> nextTwo(rhs.begin() + 1, rhs.begin() + 3);

                // Add rule Y1 → C D
                G.rules[newVar].push_back(nextTwo);

                // Replace C D with Y1 in the current rule
                rhs.erase(rhs.begin() + 1, rhs.begin() + 3);
                rhs.push_back(newVar);
            }

            // Store the shortened binary rule
            newRules.push_back(rhs);
        }

        // Update grammar for this nonterminal
        G.rules[lhs] = newRules;
    }

    printGrammar(G, "Step 4: Binarize (Limit RHS to 2 Symbols)");
}

// ===== Main CNF Conversion Driver =====
void convertToCNF(Grammar &G)
{
    printGrammar(G, "Example Grammar:");
    removeEpsilonProductions(G);
    removeUnitProductions(G);
    replaceTerminalsInMixedRHS(G);
    binarizeGrammar(G);
    cout << "\n✅ CNF Conversion Complete.\n";
}

int main()
{
    Grammar G;
    G.startSymbol = "S"; // Define the start symbol

    // Example Grammar:
    // S → ASB
    // A → aAS | a | ε
    // B → SbS | A | bb
    G.rules["S"] = {{"A", "S", "B"}};
    G.rules["A"] = {{"a", "A", "S"}, {"a"}, {"ε"}};
    G.rules["B"] = {{"S", "b", "S"}, {"A"}, {"b", "b"}};
    cout << "\nChomsky Normal Form" << endl;

    convertToCNF(G);

    return 0;
}
