**Author:** Yasmin Guerra
**Course:** CSc 30400 — Theory of Computation  
**Professor:** Jordan Matuszewski  

**Turing_Machine-Project [MIDTERM]


## Summary
This repository contains a small multi-tape Turing Machine simulator (with machines + tapes), two
example Turing machines (string and binary palindrome checkers), test suites, and full documentation intended for coursework or
demonstration purposes.

## Machine Format

Plain CSV-like text file with:
- **Header:** name, k (number of tapes), max_len (max tape length), max_steps (execution limit)
- **Input alphabet:** comma-separated symbols
- **States:** comma-separated state names
- **Start state:** single line
- **Accept, reject states:** comma-separated
- **k tape alphabets:** one line per tape (alphabet symbols, comma-separated)
- **Transitions:** comma-separated lines in format:
  ```
  current_state, read1, read2, ..., new_state, write1, write2, ..., move1, move2, ...
  ```

## Symbols & Notation

- `_` = blank symbol
- `*` = wildcard (matches any symbol in transitions)
- `L` / `R` / `S` = move directions (left, right, stay)

## Key Data Structures

- `K_TM`: Machine metadata (name, k, max_len, max_steps, alphabets, states)
- `Transition`: Struct with new_state, write[], move[], and rule_num
- `rules`: `unordered_map<string, vector<pair<vector<char>, Transition>>>` storing transitions per state

## Execution Loop

1. Load machine definition and print numbered transition rules
2. For each test input:
   - Initialize k tapes with input, heads at position 0, current state = start state
   - Repeat: find first matching transition for current (state, symbols_under_heads)
   - Write symbols, move heads (L/R/S), update state, print trace line
   - Halt when reaching accept/reject state or no matching rule (reject)
3. Print final state (Accepted/Rejected) and final tape contents

## Output Format

**Per step:** `step#, rule#, head_positions, current_state, symbols_read, new_state, symbols_written, moves`

**Final:** Accept/Reject status and tape contents


