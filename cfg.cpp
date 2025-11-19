#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
using namespace std;

bool simulateCFG(const string &input)
{
    // Step 1: Define the grammar rules
    unordered_map<char, vector<string>> grammar;
    grammar['S'] = {"aSb", "ab"}; // Non-terminal S → aSb | ab

    // Step 2: Initialize a BFS queue
    // Each queue element stores:
    //   - current derived string
    //   - list of derivation steps leading to it
    queue<pair<string, vector<string>>> q;

    // Start with the start symbol 'S'
    q.push({"S", {"S"}});

    // Step counter for readability (not functionally used)
    int step = 1;

    // Step 3: Begin BFS to explore all possible derivations
    while (!q.empty())
    {
        auto [current, steps] = q.front();
        q.pop();

        // Step 4: If the current string exactly matches the input,
        // the string is accepted by the grammar.
        if (current == input)
        {
            cout << "\n✅ String accepted!\n";
            cout << "Derivation steps:\n";
            for (int i = 0; i < steps.size(); i++)
            {
                cout << "Step " << i + 1 << ": " << steps[i] << endl;
            }
            return true;
        }

        // Step 5: Avoid expanding if the current string already exceeds
        // the input's length (to prevent unnecessary expansions)
        if (current.size() > input.size())
            continue;

        // Step 6: Find and expand the first non-terminal symbol (A–Z)
        for (int i = 0; i < current.size(); i++)
        {
            char symbol = current[i];
            if (isupper(symbol))
            {
                // For each production rule of this non-terminal
                for (string prod : grammar[symbol])
                {
                    // Replace the non-terminal with the production
                    string next = current.substr(0, i) + prod + current.substr(i + 1);

                    // Record this derivation step for output
                    auto newSteps = steps;
                    newSteps.push_back(next);

                    // Add the new derived string to the queue for further expansion
                    q.push({next, newSteps});
                }
                // Expand only one non-terminal at a time (leftmost derivation)
                break;
            }
        }
    }

    // Step 7: If BFS finishes and no match is found, reject the string
    cout << "\n❌ String rejected. Cannot be derived from the grammar.\n";
    return false;
}

int main()
{
    cout << "\nContext-Free Grammar Simulator\n";
    cout << "Grammar: ";
    cout << "S → aSb | ab\n\n";

    // Step 8: Get input string from user
    string input;
    cout << "Enter input string: ";
    cin >> input;

    // Step 9: Run the CFG simulation
    simulateCFG(input);

    return 0;
}
