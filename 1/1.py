#!/usr/bin/python
import argparse
import string
        
alphabet = string.printable

def main():
    # print(alphabet)
    args = get_cli_args()
    pattern = get_pattern()
    text = get_text()
    for char in pattern:
        if char not in alphabet
    find(pattern, text)
    # if (shift != -1):
    #     print("Pattern found at shift = {}".format(shift))


def find(pattern, text):
    transitions = compute_transitions(pattern)
    current_state = 0
    for i in range(len(text)):
        # print("current state: {}\ni: {}\ntext[i]: {}\n".format(current_state, i,text[i]))
        current_state = transitions[current_state][text[i]]
        if (current_state == len(pattern)):
            shift = i - len(pattern) + 1
            print("found at position = {}".format(shift))
        
    return -1


def compute_transitions(pattern):
    global alphabet
    transitions = [{} for state in range(len(pattern) + 1)]
    for current_state in range(len(pattern) + 1):
        for symbol in alphabet:
            next_state = min(len(pattern) + 1, current_state + 2) - 1
            while (not suffix(pattern[:next_state], pattern[:current_state] + symbol)):
                next_state -= 1
            transitions[current_state][symbol] = next_state
    return transitions

# Check if part is a suffix of the whole
def suffix(part, whole):
    result = whole[::-1][:len(part)] == part[::-1]
    return result


def get_pattern():
    return input("pattern: ")


def get_text():
    return input("text: ")


def get_cli_args():
    return [];


if __name__ == '__main__':
    main()
