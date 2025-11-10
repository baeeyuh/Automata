#include <iostream>
#include <map>
#include <tuple>
#include <string>
using namespace std;

// Transition table representation:
// Key: (current_state, symbol_read)
// Value: (new_state, symbol_to_write, head_move_direction)
using Transition = tuple<string, char, char>; // new_state, write_symbol, move_dir
using StateSymbol = pair<string, char>;       // current_state, read_symbol

// Simulate any LBA given a transition table, start state, accept states, and input
bool simulateLBA(const map<StateSymbol, Transition> &transitions,
                 const string &startState,
                 const string &acceptState,
                 string input)
{
    string tape = input;  // Tape of symbols
    string state = startState;
    int head = 0;         // Head starts at the beginning of tape

    while (true)
    {
        // Head moved past left → check acceptance
        if (head < 0)
            return state == acceptState;

        // Head moved past right → reject
        if (head >= (int)tape.size())
            return false;

        char read = tape[head];
        auto key = make_pair(state, read);

        // No valid transition → accept if in accept state, else reject
        if (transitions.find(key) == transitions.end())
            return state == acceptState;

        // Apply the transition
        auto [newState, write, move] = transitions.at(key);
        tape[head] = write;  // Write the symbol
        state = newState;    // Update state

        // Move head
        if (move == 'R') head++;
        else if (move == 'L') head--;
        else if (move == 'S' && state == acceptState)
            return true; // Accept if staying in accept state
    }
}

// Example LBA: L = { a^n b^n | n >= 1 }
map<StateSymbol, Transition> exampleTransitions = {
    {{"q0", 'a'}, {"q1", 'X', 'R'}},
    {{"q0", 'X'}, {"q0", 'X', 'R'}},
    {{"q0", 'Y'}, {"q3", 'Y', 'S'}},

    {{"q1", 'a'}, {"q1", 'a', 'R'}},
    {{"q1", 'Y'}, {"q1", 'Y', 'R'}},
    {{"q1", 'b'}, {"q2", 'Y', 'L'}},

    {{"q2", 'a'}, {"q0", 'a', 'S'}},
    {{"q2", 'X'}, {"q2", 'X', 'L'}},
    {{"q2", 'Y'}, {"q2", 'Y', 'L'}},
};

// Main
int main()
{
    cout << "\nGeneric LBA Simulator\n";
    cout << "Example: Language L = { a^n b^n | n >= 1 }\n";

    string input;
    cout << "Enter input string: ";
    cin >> input;

    // Run the LBA simulation
    bool accepted = simulateLBA(exampleTransitions, "q0", "q3", input);

    cout << (accepted ? "✅ Accepted" : "❌ Rejected") << endl;

    return 0;
}
