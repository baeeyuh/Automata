#include <iostream>
#include <stack>
#include <queue>
#include <unordered_map>
#include <vector>
#include <string>
using namespace std;

// Structure for PDA configuration
struct Config
{
    string stackContent; // Current stack as string (top at back)
    int inputIndex;      // Current position in input
    string path;         // Transition path for output
};

bool simulateCFGtoPDA(const string &input, unordered_map<char, vector<string>> &grammar)
{
    queue<Config> q;

    // Start with stack = S (start symbol)
    q.push({"S", 0, "[S]"});

    while (!q.empty())
    {
        Config current = q.front();
        q.pop();

        // Accept if stack empty and input fully read
        if (current.stackContent.empty() && current.inputIndex == input.size())
        {
            cout << "\nString accepted!\nTransitions:\n";
            cout << current.path << "\n";
            return true;
        }

        // Skip invalid paths
        if (current.stackContent.empty() || current.inputIndex > input.size())
            continue;

        char top = current.stackContent.back();
        string remainingStack = current.stackContent.substr(0, current.stackContent.size() - 1);

        // If top is non-terminal, expand using CFG productions
        if (grammar.count(top))
        {
            for (auto &prod : grammar[top])
            {
                // Push production in **reverse order** onto stack
                string newStack = remainingStack;
                for (int i = prod.size() - 1; i >= 0; --i)
                    newStack += prod[i];

                string newPath = current.path + " -> [" + newStack + "]";
                q.push({newStack, current.inputIndex, newPath});
            }
        }
        // If top is terminal, match with input
        else
        {
            if (current.inputIndex < input.size() && top == input[current.inputIndex])
            {
                string newPath = current.path + " -> [" + remainingStack + "]";
                q.push({remainingStack, current.inputIndex + 1, newPath});
            }
        }
    }

    cout << "\nString rejected!\n";
    return false;
}

int main()
{
    cout << "\nCFG to PDA\n";
    // Example CFG: S -> aSb | ab
    unordered_map<char, vector<string>> grammar;
    grammar['S'] = {"aSb", "ab"};

    cout << "Example CFG: S -> aSb | ab\n";
    string input;
    cout << "\nEnter a string to test: ";
    cin >> input;

    simulateCFGtoPDA(input, grammar);
}
