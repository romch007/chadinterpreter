# Custom language interpreter in C

Configure and build:

```bash
cmake -S . -B build
cmake --build build --parallel
```

## Features

- [x] Variables
- [x] Types (`str`, `bool`, `int`, `float`, `null`)
- [x] Flow control (`if`, `else if`, `else`, `while`)
- [x] Reference counted strings
- [x] Functions

## How to use the language

Variable declarations:

```
const title = "Hello World!";
let i = 0;
let truth = false;
let null_for_the_moment;
```

Flow control:

```
let i = 8;
if (i < 7) {
    i = 0;
} else if (i == 8) {
    i += 1;
} else {
    i = -12;
}

while (i < 100) {
    i += 1;
}
```

Functions:

```
fn add(a, b) {
    return a + b;
} 
```