# Metal FS Filter Operator

This operator filters the incoming stream of integer values by a range that is defined by a lower bound (inclusive) and an upper bound (inclusive).

## Installation
```
npm install @metalfs/filter
```

## Operator Details

| | Description |
 -| -
Input  | Stream of 64-bit big endian unsigned integer values
Output | Dense stream of matching values, single 0-byte if no matching entries were found
Stream Width | Adaptable

### Options

| | Type | Description |
 -| -    | -
lower-bound | uint64 | Pass on values greater than or equal to this parameter
upper-bound | uint64 | Pass on values less than or equal to this parameter
