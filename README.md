# Memory Management
A memory management module for an operating system simulation.

## How to Build and Run
1. Clone or download the project

Within the root of the project:

2. Run `make`
3. Run `oss`

## Arguments
```
 -h  Show help.
 -v  Verbose log output.
```

## Log Output
The below is what a page table looks like in the log:
```
Process n Page Table
| 12 | 14 | 23 | 30 | -- |
| *D | *- | -D | -- | -- | 

 *  - Valid bit is set
 D  - Dirty bit is set
 -- - Empty frame
```

Read `cs4760Assignment6Fall2017Hauschild.pdf` for more details.
