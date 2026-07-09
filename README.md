# Laven Language (0.2.0)

A dynamically typed small interpreted programming language, with simple syntax which is easy to learn and use.
Current version works on tree-walk interpreter, but later versions will be interpreted by VM.

## Build

```BASH
cmake -B build
cmake --build build
cd ./build
./laven "path_to_file"
```

Check Version via interpreter:

```BASH
./laven --version \\prints 0.2.0
./laven -v \\prints 0.2.0
```

Check Version in language:

```laven
println(__VERSION__);  \\prints 0.2.0
```

## Example

```laven
println("hello, world");

fn printn(msg, n){
    for(let i = 0; i<n; i= i+1){
        print(msg);
    }
}

printn("ni hao", 5);
```

## Features

- Closures
- First class functions
- Modules and importing
- Maps/Hashmaps
- Structs with methods
- Enums with values

## Future trajectory

-[ ] Better pattern matching
-[ ] Type checking
-[ ] Bytecode VM
-[ ] Package manager

This language will not have support for OOP at max this language offers struct objects with methods.

## License

MIT