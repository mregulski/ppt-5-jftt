# Commands
## Assign
`id := expression;`

**Status:** DONE

Evaluate expression and store in location referred to by id.

```asm
0.  r1 <- <expression>
1.  r0 <- &id
2.  STORE 1;
```

## If
`IF <cond> THEN <do_then> ELSE <do_else> ENDIF`

**Status:** DONE

Evaluate condition and jump to the correct branch.

```asm
        0.      r1 <- <cond>
        1.      ifZero r1 jump ELSE
THEN:   2.      <then>      ; n commands
        n+2.    JUMP END
ELSE:   n+3.    <else>      ; m commands
END:    n+m.    <next>      ; next instructions
```


## While
**Status:** DONE

```asm
START:      R1 <- <cond>
        0.  ifZero R1 jump END
        1.  <body>
        n.  JUMP START
END:    n+1.
```


## For
**Status:** DONE


## Read
**Status:** DONE


## Write
**Status:** DONE

* * * * *


# Expressions
## Addition
**Status:** DONE

Straightforward.


## Subtraction
**Status:** DONE

- `var` - `const`
```asm
0.  R0 <- &tmp
1.  R1 <- `const`
2.  STORE 1
3.  R0 <- &var
4.  LOAD 1
5.  R0 <- &tmp
6.  SUB 1
```


## Multiplication
**Status:** WIP

## Division
**Status:** NOT STARTED

* * * * *

# Conditions
## Equal
`<value> a = <value> b`

**Status:** DONE

Compare values `a` and `b` by two comparisons.
`a` and `b` are equal if `a-b == b-a` (single subtraction is not enough in case `a > b` because of negative values being rounded to 0)
** Return location: `r1`.
** Return value:
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


## Not-equal
`<value> a <> <value> b`

**Status:** DONE

Same as `Equal` with flipped `R1` values.

## Greater-than
`<value> a > <value> b`

**Status**: DONE

```asm
            R2 <- a - b
        0.  ZERO 1
        1.  ifZero R2 jump END [+2]
        2.  INC 1
END:    3.  < R1 = 1 if a > b, 0 otherwise>
```

## Less-than
`<value> a > <value> b`

**Status**: WIP

Same as `Greater-than` with flipped sides.


## Less-or-Equal
`<value> a <= <value> b`

**Status**: WIP

Same as `Greater Than` with flipped `R1` values.


* * * * *