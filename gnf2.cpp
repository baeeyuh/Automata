#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

// Grammar representation
struct Grammar
{
    string start;                              // Start symbol of the grammar
    map<string, vector<vector<string>>> rules; // Mapping: Nonterminal -> list of RHS rules
};

// Check if a string is a terminal (lowercase)
bool isTerminal(const string &s) { return s.size() == 1 && islower(s[0]); }

// Check if a string is a nonterminal (uppercase)
bool isNonTerminal(const string &s) { return s.size() == 1 && isupper(s[0]); }

// Print the grammar
void printGrammar(const Grammar &G)
{
    for (auto &[lhs, rhss] : G.rules)
    {
        cout << lhs << " → ";
        for (size_t i = 0; i < rhss.size(); ++i)
        {
            for (auto &sym : rhss[i])
                cout << sym;
            if (i + 1 < rhss.size())
                cout << " | ";
        }
        cout << "\n";
    }
}

// Step 1: Remove ε-productions
void removeEpsilons(Grammar &G)
{
    set<string> nullable; // Store nonterminals that can produce ε

    // Find all nullable symbols
    for (auto &[A, rhss] : G.rules)
        for (auto &rhs : rhss)
            if (rhs.size() == 1 && rhs[0] == "ε")
                nullable.insert(A);

    // For each nullable symbol, create new rules in other productions
    for (auto &A : nullable)
        for (auto &[B, rhss] : G.rules)
        {
            vector<vector<string>> newRules;
            for (auto rhs : rhss)
                for (size_t i = 0; i < rhs.size(); ++i)
                    if (rhs[i] == A && rhs.size() > 1)
                    {
                        auto temp = rhs;
                        temp.erase(temp.begin() + i); // Remove nullable symbol
                        newRules.push_back(temp);      // Add new variation
                    }
            rhss.insert(rhss.end(), newRules.begin(), newRules.end());
        }

    // Remove direct ε-rules (except start symbol)
    for (auto &A : nullable)
        if (A != G.start)
            G.rules[A].erase(remove_if(G.rules[A].begin(), G.rules[A].end(),
                                       [](auto &r) { return r.size() == 1 && r[0] == "ε"; }),
                             G.rules[A].end());
}

// Step 2: Remove unit productions (A → B)
void removeUnits(Grammar &G)
{
    bool changed = true;

    // Repeat until no unit productions remain
    while (changed)
    {
        changed = false;
        for (auto &[A, rhss] : G.rules)
        {
            vector<vector<string>> add; // Rules to add
            for (auto &rhs : rhss)
                if (rhs.size() == 1 && isNonTerminal(rhs[0]))
                {
                    string B = rhs[0];
                    // Add all rules of B to A (except self-loop)
                    for (auto &prod : G.rules[B])
                        if (!(prod.size() == 1 && prod[0] == A))
                            add.push_back(prod);
                }

            size_t before = rhss.size();
            rhss.insert(rhss.end(), add.begin(), add.end());

            // Remove original unit productions
            rhss.erase(remove_if(rhss.begin(), rhss.end(),
                                 [](auto &r) { return r.size() == 1 && isNonTerminal(r[0]); }),
                       rhss.end());

            changed |= (rhss.size() != before); // Repeat if changed
        }
    }
}

// Helper: Remove immediate left recursion
void removeLeftRecursion(Grammar &G, const string &A)
{
    auto &rhss = G.rules[A];
    vector<vector<string>> alpha; // Recursive rules: A → Aα
    vector<vector<string>> beta;  // Non-recursive rules: A → β

    // Separate recursive and non-recursive rules
    for (auto &rhs : rhss)
        (rhs[0] == A ? alpha : beta).push_back(rhs);

    if (alpha.empty()) return; // Nothing to do if no recursion

    // Create new variable A' for recursion
    string Aprime = A + "'";
    while (G.rules.count(Aprime)) Aprime += "'"; // Ensure uniqueness

    // Rewrite A → βA' and A' → αA' | ε
    G.rules[A].clear();
    for (auto &b : beta)
    {
        b.push_back(Aprime);
        G.rules[A].push_back(b);
    }

    for (auto &a : alpha)
    {
        a.erase(a.begin()); // Remove leading A
        a.push_back(Aprime);
        G.rules[Aprime].push_back(a);
    }
    G.rules[Aprime].push_back({"ε"}); // Allow termination
}

// Step 3: Convert to GNF
void convertToGNF(Grammar &G)
{
    // Collect variables in deterministic order
    vector<string> vars;
    for (auto &[A, _] : G.rules)
        vars.push_back(A);
    sort(vars.begin(), vars.end());

    // Process each variable in order
    for (size_t i = 0; i < vars.size(); ++i)
    {
        string Ai = vars[i];
        bool repeat = true;

        // Substitute leading variables Aj (j < i) recursively
        while (repeat)
        {
            repeat = false;
            vector<vector<string>> newR;
            for (auto &rhs : G.rules[Ai])
            {
                if (isNonTerminal(rhs[0]))
                {
                    auto it = find(vars.begin(), vars.end(), rhs[0]);
                    if (it != vars.end() && (size_t)distance(vars.begin(), it) < i)
                    {
                        // Replace Ai → Ajα with Aj's rules
                        for (auto &gamma : G.rules[rhs[0]])
                        {
                            vector<string> combo = gamma;
                            combo.insert(combo.end(), rhs.begin() + 1, rhs.end());
                            newR.push_back(combo);
                        }
                        repeat = true;
                        continue;
                    }
                }
                newR.push_back(rhs);
            }
            G.rules[Ai] = newR;
        }

        // Remove immediate left recursion for Ai
        removeLeftRecursion(G, Ai);
    }

    // Cleanup: keep only terminal-leading rules
    for (auto &[A, rhss] : G.rules)
        rhss.erase(remove_if(rhss.begin(), rhss.end(),
                             [](auto &r) { return r.empty() || !isTerminal(r[0]); }),
                   rhss.end());
}

// Main function
int main()
{
    Grammar G;
    G.start = "S";

    // Example Grammar:
    // S → AB | b
    // A → aA | a
    // B → b
    G.rules["S"] = {{"A", "B"}, {"b"}};
    G.rules["A"] = {{"a", "A"}, {"a"}};
    G.rules["B"] = {{"b"}};

    // Convert CFG to GNF
    removeEpsilons(G);   // Step 1: Remove ε-productions
    removeUnits(G);      // Step 2: Remove unit productions
    convertToGNF(G);     // Step 3: Convert to GNF

    // Print final GNF grammar
    printGrammar(G);

    return 0;
}
