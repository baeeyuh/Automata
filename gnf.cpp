#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

// Simple structure to hold grammar data
struct Grammar
{
    string start;                              // starting symbol (like "S")
    map<string, vector<vector<string>>> rules; // rules: A → α | β ...
};

// Check if a string is a terminal (lowercase single character)
bool isTerminal(const string &s) { return s.size() == 1 && islower(s[0]); }

// Check if a string is a nonterminal (uppercase single character)
bool isNonTerminal(const string &s) { return s.size() == 1 && isupper(s[0]); }

// Pretty-print the grammar so we can visualize transformations
void printGrammar(const Grammar &G, const string &title = "")
{
    if (!title.empty())
        cout << "\n"
             << title << ":\n";
    for (auto &[lhs, rhss] : G.rules)
    {
        cout << lhs << " → ";
        for (size_t i = 0; i < rhss.size(); ++i)
        {
            for (auto &sym : rhss[i])
                cout << sym; // print each symbol
            if (i + 1 < rhss.size())
                cout << " | "; // separate alternatives
        }
        cout << "\n";
    }
}

/* STEP 1: REMOVE ε-PRODUCTIONS
   If a nonterminal A can produce ε (the empty string),
   then we remove it and adjust all other rules that use A. */
void removeEpsilons(Grammar &G)
{
    set<string> nullable; // stores symbols that can produce ε

    // Find all nullable symbols (A → ε)
    for (auto &[A, rhss] : G.rules)
        for (auto &rhs : rhss)
            if (rhs.size() == 1 && rhs[0] == "ε")
                nullable.insert(A);

    // For each nullable symbol, add new rules that skip it
    for (auto &A : nullable)
        for (auto &[B, rhss] : G.rules)
        {
            vector<vector<string>> newRules;
            for (auto rhs : rhss)
                for (size_t i = 0; i < rhs.size(); ++i)
                    // If nullable symbol appears in RHS, remove it once
                    if (rhs[i] == A && rhs.size() > 1)
                    {
                        auto temp = rhs;
                        temp.erase(temp.begin() + i);
                        newRules.push_back(temp);
                    }
            // Add all newly created variations
            rhss.insert(rhss.end(), newRules.begin(), newRules.end());
        }

    // Finally, remove direct ε rules (except if it’s the start symbol)
    for (auto &A : nullable)
        if (A != G.start)
            G.rules[A].erase(remove_if(G.rules[A].begin(), G.rules[A].end(),
                                       [](auto &r)
                                       { return r.size() == 1 && r[0] == "ε"; }),
                             G.rules[A].end());

    printGrammar(G, "After Removing ε-Productions");
}

/* STEP 2: REMOVE UNIT PRODUCTIONS (A → B)
   Replace any single-variable productions with the
   productions of that variable. */
void removeUnits(Grammar &G)
{
    bool changed = true;
    while (changed)
    { // repeat until stable
        changed = false;
        for (auto &[A, rhss] : G.rules)
        {
            vector<vector<string>> add; // to store new rules to add
            for (auto &rhs : rhss)
                // If RHS is a single nonterminal, expand it
                if (rhs.size() == 1 && isNonTerminal(rhs[0]))
                {
                    string B = rhs[0];
                    // Copy all rules of B into A
                    for (auto &prod : G.rules[B])
                        if (!(prod.size() == 1 && prod[0] == A)) // avoid A → A
                            add.push_back(prod);
                }

            // Add new rules and remove old unit ones
            size_t before = rhss.size();
            rhss.insert(rhss.end(), add.begin(), add.end());
            rhss.erase(remove_if(rhss.begin(), rhss.end(),
                                 [](auto &r)
                                 { return r.size() == 1 && isNonTerminal(r[0]); }),
                       rhss.end());
            // If number of rules changed, repeat loop
            changed |= (rhss.size() != before);
        }
    }
    printGrammar(G, "After Removing Unit Productions");
}

/* HELPER: REMOVE IMMEDIATE LEFT RECURSION (A → Aα)
   If A → Aα | β
   then replace with:
      A → βA'
      A' → αA' | ε */
void removeLeftRecursion(Grammar &G, const string &A)
{
    auto &rhss = G.rules[A];
    vector<vector<string>> alpha; // recursive parts (A → Aα)
    vector<vector<string>> beta;  // non-recursive parts (A → β)

    // Separate recursive and non-recursive rules
    for (auto &rhs : rhss)
        (rhs[0] == A ? alpha : beta).push_back(rhs);

    if (alpha.empty())
        return; // nothing to fix

    // Create new variable A' for recursion
    string Aprime = A + "'";
    while (G.rules.count(Aprime))
        Aprime += "'"; // ensure uniqueness

    // Step 1: A → βA'
    G.rules[A].clear();
    for (auto &b : beta)
    {
        b.push_back(Aprime);
        G.rules[A].push_back(b);
    }

    // Step 2: A' → αA' | ε
    for (auto &a : alpha)
    {
        a.erase(a.begin()); // remove the first symbol (A)
        a.push_back(Aprime);
        G.rules[Aprime].push_back(a);
    }
    G.rules[Aprime].push_back({"ε"});
}

/* STEP 3: CONVERT TO GNF (Greibach Normal Form)
   - Each rule must start with a terminal.
   - Uses variable ordering and substitution.
   - After substitution, removes left recursion.  */
void convertToGNF(Grammar &G)
{
    // Collect and sort all variables (deterministic order)
    vector<string> vars;
    for (auto &[A, _] : G.rules)
        vars.push_back(A);
    sort(vars.begin(), vars.end());

    // Process each variable Ai in order
    for (size_t i = 0; i < vars.size(); ++i)
    {
        string Ai = vars[i];
        bool repeat = true;

        // Step 1: Substitute any leading Aj where j < i
        while (repeat)
        {
            repeat = false;
            vector<vector<string>> newR;
            for (auto &rhs : G.rules[Ai])
            {
                if (isNonTerminal(rhs[0]))
                {
                    // find the index j of that leading symbol
                    auto it = find(vars.begin(), vars.end(), rhs[0]);
                    if (it != vars.end() && (size_t)distance(vars.begin(), it) < i)
                    {
                        // Substitute Aj → γ for all γ
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

        // Step 2: Remove immediate left recursion for Ai
        removeLeftRecursion(G, Ai);
    }

    // Step 3: Cleanup (keep only terminal-leading rules)
    for (auto &[A, rhss] : G.rules)
        rhss.erase(remove_if(rhss.begin(), rhss.end(),
                             [](auto &r)
                             { return r.empty() || !isTerminal(r[0]); }),
                   rhss.end());

    printGrammar(G, "After Conversion to GNF");
}

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

    cout << "\nGreibach Normal Form.\n";

    // Step-by-step transformation
    printGrammar(G, "Example Grammar");
    removeEpsilons(G);
    removeUnits(G);
    convertToGNF(G);

    cout << "\n✅ GNF Conversion Complete.\n";
    return 0;
}
