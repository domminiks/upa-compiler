# UPA Compiler

A compiler for a custom programming language, named UPA, created to learn basic concepts regarding the compiler theory, parsers and lexical analysers. Essentially, this language is a formal translation of the C language to Brazilian Portuguese.

Operations that are implemented are:
* Variables and function declaration;
* Assign operation;
* Printing function;
* Basic math operations;

For a full documentation on the language, please, refer to the documentation under */docs* folder.

## Creating the Compiler 

To get this compiler to work, we first have to (from the *src* directory):

1. Generate headers and the syntactic analyser:

```shell
$ bison -d upa.y
```

2. Then, we must generate the lexical analyser:

```shell
$ flex upa
```

3. Finally, we are able to generate the compiler itself:

* For Linux environment:
```shell
$ gcc -g -o upa upa.tab.c lex.yy.c upa.c -lm
```

* For Windows:
```shell
$ gcc -g -o upa upa.tab.c lex.yy.c upa.c -lfl
```

* For MacOS:
```shell
$ cc -g -o upa upa.tab.c lex.yy.c upa.c -ll
```

## Running Examples

Under */src/examples* you will find some examples for better understanding of language syntax. Just like a normal compiler, the code can be compiled and executed as it follows (from the *src* directory):

```shell
$ ./upa ./examples/ex1.upa
$ ./upa ./examples/ex2.upa
```

```shell
$ ./upa ./examples/test.upa
```

# References

This work was vastly based on [**flex & bison**:  Text Processing Tools](https://books.google.com.br/books?id=3Sr1V5J9_qMC&lpg=PP1&dq=flex%20bison%20o'reilly&hl=pt-BR&pg=PP1#v=onepage&q=flex%20bison%20o'reilly&f=false), by John Levine, published by O'Reilly Media, Inc and its GitHub [repo](https://github.com/mbbill/flexbison/tree/master/flexbison).