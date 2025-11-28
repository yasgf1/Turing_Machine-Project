#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

using namespace std;


//defining the transition function
struct Transition {
    string new_state; //state to transition to 
    vector<char> write; //new symbol to write
    vector<char> move; //head direction: LEFT, RIGHT, STAY 
    int rule_num; //printing trace
};


//defining a multi-tape TM
struct K_TM{
    string name;
    int k; //K-Turning Machine -> K tapes
    int max_len; //max lengtth of each tape... if tape contains less symbols, consider blanks
    long long max_steps; //max steps to be simulated, if exceed = HALT

   
    vector<char> input_alph; //sigma
    vector<string> states;
    string start_state;
    string accept_state;
    string reject_state;
    
    vector<vector<char>> tape_alpha; //gamma

    unordered_map<string, vector<pair<vector<char>, Transition> > > rules; //current state and respective rules

};

class Simulator {

public:
    K_TM tm;
    vector<vector<char>> tapes; //tape storage: [ [tape 1], [tape 2], [tape 3] ...]
    vector<int> head; //head directions (per tape)
    string curr_state;

    void loadMachine(string filename); // reads machine.txt
    void loadTapes(string filename); // loads tape blocks
    bool matchPattern(const vector<char>& pattern, const vector<char>& actual); 
    void runSingleInput(const vector<string>& inputLines);


private:
    bool step(long long &stepCount); //single transition step
};

//reading files
void Simulator::loadMachine(string filename) {

    ifstream fin(filename);
    if (!fin) {
        cerr << "Error opening machine file.\n";
        exit(1);
    }

    string line;

    // Line 1: name, k, maxLen, maxSteps ~ general info of the k TM 
    getline(fin, line);
    {
        stringstream line_in(line);
        string gen;
        getline(line_in, tm.name, ',');
        getline(line_in, gen, ','); tm.k = stoi(gen);
        getline(line_in, gen, ','); tm.max_len = stoi(gen);
        getline(line_in, gen, ','); tm.max_steps = stoll(gen);
    }

    // Line 2: retrieving input alphabet ~ sigma
    getline(fin, line);
    {
        stringstream line_in(line);
        string sigma;
        while (getline(line_in, sigma, ',')) tm.input_alph.push_back(sigma[0]);
    }

    // Line 3: states
    getline(fin, line);
    {
        stringstream line_in(line);
        string s;
        while (getline(line_in, s, ',')) tm.states.push_back(s);
    }

    // Line 4: start state
    getline(fin, tm.start_state);
    // Trim whitespace from start state
    tm.start_state.erase(0, tm.start_state.find_first_not_of(" \t\r\n"));
    tm.start_state.erase(tm.start_state.find_last_not_of(" \t\r\n") + 1);

    // Line 5: accept, reject
    getline(fin, line);
    {
        stringstream line_in(line);
        getline(line_in, tm.accept_state, ',');
        getline(line_in, tm.reject_state, ',');

        //for trimming whitspaces
        tm.accept_state.erase(0, tm.accept_state.find_first_not_of(" \t\r\n"));
        tm.accept_state.erase(tm.accept_state.find_last_not_of(" \t\r\n") + 1);
        tm.reject_state.erase(0, tm.reject_state.find_first_not_of(" \t\r\n"));
        tm.reject_state.erase(tm.reject_state.find_last_not_of(" \t\r\n") + 1);
    }

    // Next k lines: tape alpahebet (for each tape )
    tm.tape_alpha.resize(tm.k);
    for (int i = 0; i < tm.k; i++) {
        getline(fin, line);
        stringstream line_in(line);
        string s;
        while (getline(line_in, s, ',')) tm.tape_alpha[i].push_back(s[0]);
        tm.tape_alpha[i].push_back('_'); // blank always included
    }

    // Read transitions
    int rule_num = 1; //simply here for tracing purposes

    //begins
    while (getline(fin, line)) {
        if (line.size() < 2) continue;

        stringstream line_in(line);
        string initialState, newState;

        vector<char> read(tm.k);
        vector<char> write(tm.k);
        vector<char> move(tm.k);

        getline(line_in, initialState, ',');
        // Trim state name
        initialState.erase(0, initialState.find_first_not_of(" \t\r\n"));
        initialState.erase(initialState.find_last_not_of(" \t\r\n") + 1);

        // Read inputs
        for (int i = 0; i < tm.k; i++) {
            string t; 
            getline(line_in, t, ',');
            // Trim whitespace from symbol
            t.erase(0, t.find_first_not_of(" \t\r\n"));
            t.erase(t.find_last_not_of(" \t\r\n") + 1);
            read[i] = t.empty() ? '_' : t[0];
        }

        getline(line_in, newState, ',');
        // Trim state name
        newState.erase(0, newState.find_first_not_of(" \t\r\n"));
        newState.erase(newState.find_last_not_of(" \t\r\n") + 1);

        // New symbols
        for (int i = 0; i < tm.k; i++) {
            string t; 
            getline(line_in, t, ',');
            // Trim whitespace from symbol
            t.erase(0, t.find_first_not_of(" \t\r\n"));
            t.erase(t.find_last_not_of(" \t\r\n") + 1);
            write[i] = t.empty() ? '_' : t[0];
        }

        // Directions
        for (int i = 0; i < tm.k; i++) {
            string t; 
            getline(line_in, t, ',');
            // Trim whitespace from direction
            t.erase(0, t.find_first_not_of(" \t\r\n"));
            t.erase(t.find_last_not_of(" \t\r\n") + 1);
            move[i] = t.empty() ? 'S' : t[0];
        }

        Transition tr { newState, write, move, rule_num++ };  
        tm.rules[initialState].push_back({read, tr}); //storing transition rule
    }

    cout << "Machine loaded: " << tm.name << "\n";
   
    for (auto &state : tm.states) {
        cout << "  " << state << " (";
        if (tm.rules.count(state)) {
            cout << tm.rules[state].size() << " rules";
        } else {
            cout << "0 rules";
        }
        cout << ")\n";
    }
    cout << "Accept state: " << tm.accept_state << ", Reject state: " << tm.reject_state << "\n";
    cout << "Start state: " << tm.start_state << "\n";
}

//WILDCARD REQ
bool Simulator::matchPattern(const vector<char>& pattern, const vector<char>& actual) {
    for (int i = 0; i < tm.k; i++) {
        if (pattern[i] != '*' && pattern[i] != actual[i])
            return false;
    }
    return true;
}


// Executes ONE STEP of TM

bool Simulator::step(long long &stepCount) {
    if (curr_state == tm.accept_state || curr_state == tm.reject_state)
        return false;

    //under ~ under each tape head
    vector<char> under(tm.k);
    for (int i = 0; i < tm.k; i++) {
        under[i] = tapes[i][head[i]];
    }

    auto &vec = tm.rules[curr_state];

 
    for (int i = 0; i < tm.k; i++) {
       
        if (i + 1 < tm.k) cerr << ",";
    }
    

    for (auto &p : vec) {
        
        for (int i = 0; i < tm.k; i++) {
            
            if (i + 1 < tm.k) cerr << ",";
        }
        

        if (matchPattern(p.first, under)) {
            Transition &tr = p.second;

            // Print trace
        
            for (int i = 0; i < tm.k; i++) cout << head[i] << (i+1<tm.k? ",":"");
            cout << ", " << curr_state;
            for (char c : under) cout << ", " << c;
            cout << ", " << tr.new_state;
            for (char c : tr.write) cout << ", " << c;
            for (char c : tr.move) cout << ", " << c;
            cout << "\n";

            // Write
            for (int i = 0; i < tm.k; i++)
                if (tr.write[i] != '*')
                    tapes[i][head[i]] = tr.write[i];

            // Move
            for (int i = 0; i < tm.k; i++) {
                if (tr.move[i] == 'L' && head[i] > 0)
                    head[i]--;
                else if (tr.move[i] == 'R' && head[i] < tm.max_len -1)
                    head[i]++;
                // else 'S' or other = STAY (no movement)
            }

            curr_state = tr.new_state;
            return true;
        }
    }

    // No rule matched → reject
    cerr << "  NO MATCHING RULE FOUND - REJECTING\n";
    curr_state = tm.reject_state;
    return false;
}

//ONE full execution 
void Simulator::runSingleInput(const vector<string>& inputLines) {
    //init empty tapes
    tapes.assign(tm.k, vector<char>(tm.max_len, '_'));
    head.assign(tm.k, 0);
    curr_state = tm.start_state;

    // Load input into tape
    for (int i = 0; i < tm.k; i++) {
        for (int j = 0; j < (int)inputLines[i].size(); j++)
            tapes[i][j] = inputLines[i][j];
    }

    cout << "Initial Tapes:\n";
    for (int i = 0; i < tm.k; i++) {
        cout << "Tape " << i+1 << ": ";
        for (char c : tapes[i]) if (c != '_') cout << c;
        cout << "\n";
    }

    long long steps = 1;
    while (steps <= tm.max_steps) {
        if (!step(steps)) break;
        steps++;
    }

    if (curr_state == tm.accept_state) cout << "Accepted\n";
    else if (curr_state == tm.reject_state) cout << "Rejected\n";
    else cout << "Error\n";

    cout << "Final Tapes:\n";
    for (int i = 0; i < tm.k; i++) {
        cout << "Tape " << i+1 << ": ";
        for (char c : tapes[i]) if (c != '_') cout << c;
        cout << "\n\n";
    }
}

//read tape.txt file
void Simulator::loadTapes(string filename) {
    ifstream fin(filename);
    if (!fin) {
        cerr << "Error opening tape file.\n";
        exit(1);
    }

    cout << "Tape file: " << filename << "\n\n";

    vector<string> block(tm.k);
    while (true) {
        for (int i = 0; i < tm.k; i++) {
            if (!getline(fin, block[i])) return;
        }
        runSingleInput(block);
    }
}


//MAIN 
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: ./tm-emplid machine.txt tapes.txt\n";
        return 0;
    }

    Simulator sim;
    sim.loadMachine(argv[1]);
    sim.loadTapes(argv[2]);
    return 0;
}
