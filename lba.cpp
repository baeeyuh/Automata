#include <iostream>
#include <map>
#include <tuple>
#include <string>
using namespace std;

// Transition table for the LBA Language: L = { a^n b^n | n >= 1 }
// Each key: (current_state, symbol_read)
// Each value: (new_state, symbol_to_write, head_move_direction)
map<pair<string, char>, tuple<string, char, char>> transitions = {
    {{"q0", 'a'}, {"q1", 'X', 'R'}}, // In q0, mark the first 'a' as 'X' and move right to find matching 'b'
    {{"q0", 'X'}, {"q0", 'X', 'R'}}, // Skip over already marked X
    {{"q0", 'Y'}, {"q3", 'Y', 'S'}}, // If only Y’s remain, go to accept state q3

    {{"q1", 'a'}, {"q1", 'a', 'R'}}, // While in q1, skip remaining unmarked a’s
    {{"q1", 'Y'}, {"q1", 'Y', 'R'}}, // Skip over Y’s while searching for the first unmarked b
    {{"q1", 'b'}, {"q2", 'Y', 'L'}}, // When you find a b, mark it as Y and move left

    {{"q2", 'a'}, {"q0", 'a', 'S'}}, // When back at an 'a', switch to q0 to mark the next one
    {{"q2", 'X'}, {"q2", 'X', 'L'}}, // Move left over X’s to reach the next unmarked a
    {{"q2", 'Y'}, {"q2", 'Y', 'L'}}, // Move left over Y’s while returning
};

// Simulate the Linear Bounded Automaton
bool simulateLBA(string input)
{
    string tape = input;    // Tape represents the string being processed
    string state = "q0";    // Start in state q0
    int head = 0;           // Tape head starts at the first symbol
    string original = tape; // Keep the original string for output

    cout << "Initial tape: " << tape << "\n";

    int step = 1;
    while (true)
    {
        // Case 1: Head moves left past the beginning of the tape
        // If this happens, check if the string is fully marked (only X and Y)
        if (head < 0)
        {
            bool allMarked = true;
            for (char c : tape)
                if (c != 'X' && c != 'Y')
                    allMarked = false;

            if (allMarked)
            {
                // If all symbols are marked → accept
                cout << "Final tape: " << tape << endl;
                cout << "✅ Accepted: " << original << endl;
                return true;
            }
            else
            {
                // If any unmarked a or b remains → reject
                cout << "❌ Rejected (unmarked symbols left)\n";
                return false;
            }
        }

        // Case 2: Head moves past the end of the tape → reject
        if (head >= (int)tape.size())
        {
            cout << "❌ Rejected (head out of bounds)\n";
            return false;
        }

        // Read the current symbol under the head
        char read = tape[head];
        auto key = make_pair(state, read);

        // Display current configuration
        cout << "Step " << step++ << ": State=" << state
             << ", Head=" << head
             << ", Read='" << read << "', Tape=" << tape << endl;

        // Case 3: No valid transition found
        if (transitions.find(key) == transitions.end())
        {
            // If currently in accepting state (q3), accept
            if (state == "q3")
            {
                cout << "Final tape: " << tape << endl;
                cout << "✅ Accepted: " << original << endl;
                return true;
            }
            // Otherwise, reject
            cout << "❌ Rejected (no transition found)\n";
            return false;
        }

        // Apply transition rule
        auto [newState, write, move] = transitions[key];

        // Replace the current symbol with the one specified in the transition
        tape[head] = write;
        // Update state
        state = newState;

        // Move tape head based on the transition direction:
        // 'R' = right, 'L' = left, 'S' = stay
        if (move == 'R')
            head++;
        else if (move == 'L')
            head--;
        else if (move == 'S')
        {
            // If machine stays and is in accept state, accept the string
            if (state == "q3")
            {
                cout << "Final tape: " << tape << endl;
                cout << "✅ Accepted: " << original << endl;
                return true;
            }
        }
    }
}

int main()
{
    string input;
    cout << "\nLinear Bounded Automata Simulation\n";
    cout << "Language: L = { a^n b^n | n >= 1 }\n";
    cout << "\nEnter input string: ";
    cin >> input;

    // Run the LBA simulation
    simulateLBA(input);
    return 0;
}
