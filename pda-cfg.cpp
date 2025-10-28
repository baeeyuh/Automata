#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
using namespace std;

// Struct to store a derivation step
struct Step
{
    string derived; // Current string derived
    string path;    // Derivation path (used internally)
};

bool simulateCFG(const string &input, unordered_map<char, vector<string>> &grammar)
{
    queue<Step> q;
    q.push({"S", "S"}); // Start symbol

    while (!q.empty())
    {
        Step current = q.front();
        q.pop();

        // Accept if fully expanded string matches input
        if (current.derived == input)
        {
            cout << "\nString accepted!\n";
            cout << "Derivation: " << current.path << "\n";
            return true;
        }

        // Skip strings that are too long
        if (current.derived.size() > input.size())
            continue;

        // Expand the first non-terminal
        for (int i = 0; i < current.derived.size(); ++i)
        {
            char c = current.derived[i];
            if (grammar.count(c))
            {
                for (auto &prod : grammar[c])
                {
                    // Generate next derived string
                    string next = current.derived.substr(0, i) + prod + current.derived.substr(i + 1);
                    // Build new path
                    string nextPath = current.path + " -> " + next;
                    q.push({next, nextPath});
                }
                break; // Only expand first non-terminal at a time
            }
        }
    }

    // If BFS finishes without finding the input, it's rejected
    cout << "\nString rejected!\n";
    return false;
}

int main()
{
    cout << "\nPDA to CFG\n";
    // Define CFG rules
    unordered_map<char, vector<string>> grammar;
    grammar['S'] = {"aSb", "ab"}; // Example CFG

    // Print example CFG
    cout << "Example CFG: S -> aSb | ab\n";
    string input;
    cout << "\nEnter a string to test: ";
    cin >> input;

    simulateCFG(input, grammar);
}
