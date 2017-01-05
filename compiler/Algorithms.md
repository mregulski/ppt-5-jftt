# Commands

## Assign
Straightforward.

## If
`IF <cond> THEN <do_then> ELSE <do_else> ENDIF`

Evaluate condition and jump to the correct branch.

```asm
        0.      r1 <- <cond>
        1.      ifZero r1 jump ELSE
THEN:   2.      <then>      ; n commands
        n+2.    JUMP END
ELSE:   n+3.    <else>      ; m commands
END:    n+m.    <next>      ; next instructions
```

------------------------

# Expressions

## Addition

Straightforward.

## Subtraction

* `var` - `const`
```
0.  R0 <- &tmp
1.  R1 <- `const`
2.  STORE 1
3.  R0 <- &var
4.  LOAD 1
5.  R0 <- &tmp
6.  SUB 1
```

------------------------

# Conditions

## Equal

Compare values `a` and `b` by two comparisons.
`a` and `b` are equal if `a-b == b-a` (single subtraction is not enough in case `a > b` because of negative values being rounded to 0)
* Return location: `r1`.
* Return value:
    - `1` if `a == b`
    - `0` otherwise
```asm
            ZERO r1
            r2 <- a - b
            JUMP START              ; save ref, update jump after sub2
SUB2:       r2 <- b-a               ; keep length
            JUMP MAYBE [+3]
START:  0.  ifZero r2 jump SUB2
        1.  jump END [+4]
MAYBE:  2.  ifZero r2 jump TRUE [+2]
        3.  JUMP END [+2]
TRUE:   4.  INC 1
END:    5.  ; r1 = (a == b) ? 1 : 0
```

## Not Equal
Same as Equal with flipped `r1` values. The initial value of `r1` is 1, if `a` equals `b` it gets decremented to 0.

## Greater Than

