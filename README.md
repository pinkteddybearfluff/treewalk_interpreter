# Laven Language (0.13.2)

A dynamically typed interpreted programming language.
Current version works on tree-walk interpreter, but later versions will be interpreted by VM.

## Features

- Variables
- Functions
- Arrays

## Build

```BASH
cmake -B build
cmake --build build
cd ./build
./interpreter "path_to_file"
```

## Example

```laven
print("hello. world");

fn printn(msg, n){
    for(let i = 0; i<n; i= i+1){
        print(msg);
    }
}

printn("meow", 5);
```
