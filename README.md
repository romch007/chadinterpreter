# Custom language interpreter in C

Configure and build:

```bash
cmake -S . -B build
cmake --build build --parallel
```

## How to use the language

Variable declarations:

```
const title = "Hello World!";
let i = 0;
let truth = false;
let name: str;
```

Flow control:

```
let i = 8;
if (i < 7) {
    i = 0;
} else if (i == 8) {
    i = i + 1;
} else {
    i = -12;
}

while (i < 100) {
    i = i + 1;
}
```