# Metal FS Filter Operator

This operator filters the incoming stream of integer values by a range that is defined by a lower bound (inclusive) and an upper bound (inclusive).

For general information on Metal FS, please visit the [project website](https://metalfs.github.io).

## Installation
```
npm install @metalfs/filter
```

Example `image.json`:
```json
{
    "streamBytes": 8,
    "target": "SNAP/WebPACK_Sim",
    "operators": {
        "filter": "@metalfs/filter"
    }
}
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

## See also

- [Aggregate Operator](https://github.com/metalfs/aggregate)
