#!/usr/bin/python
import argparse
import string

class Matcher:
    def __init__(self, pattern):
        self.pattern = pattern
        self.alphabet = string.ascii_letters
        self.transition = {}

    def compute_transitions(self):
        m = len(self.pattern)
        for state in range(0, m+1):
            for symbol in self.alphabet:
                k = min(m + 1, q + 2)
                print("state: {}, k: {}, m: {}".format(state, k, m))
                transition[symbol][q]


    def search(self, text):
        n = len(text)
        q = 0
        for i in range(1, n+1):
            q = self.transition[text[i]][q]
            if (q == m):
                shift = i - m
                return shift
        return -1

def main():
    args = get_cli_args()
    pattern = get_pattern()
    text = get_text()
    shift = Matcher(pattern).search(text)
    if (shift != -1):
        print("Pattern found at shift = {}".format(shift))


def find(pattern, text):
    print("looking for pattern '{}'".format(pattern))


def get_pattern():
    return input("pattern: ")


def get_text():
    return input("text: ")


def get_cli_args():
    return [];


if __name__ == '__main__':
    main()
