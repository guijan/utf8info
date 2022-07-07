# utf8info - print information about utf-8, codepoints, and grapheme clusters
It's impossible to handle Unicode with the Standard C APIs.
This is a toy program that tries to see how far you can go anyway.

If you experiment with the text inside [test.txt](test.txt), you will get a
grasp of what breaks and what doesn't.

Program output:
```console
$ build/utf8info
Type text to get its Unicode and UTF-8 properties:
> 👨‍👩‍👧‍👧
'👨' width:  2, bytes:  4, hex: F09F91A8
'??' width:  0, bytes:  3, hex: E2808D
'👩' width:  2, bytes:  4, hex: F09F91A9
'??' width:  0, bytes:  3, hex: E2808D
'👧' width:  2, bytes:  4, hex: F09F91A7
'??' width:  0, bytes:  3, hex: E2808D
'👧' width:  2, bytes:  4, hex: F09F91A7
> !
Grapheme mode.
> 👨‍👩‍👧‍👧
'👨‍👩‍👧‍👧' width:  2, bytes: 25, codepoints:  7, hex: F09F91A8E2808DF09F91A9E2808DF09F91A7E2808DF09F91A7
```
Do note that essentially every program (including this one) has broken Unicode
handling so both the input and the output will be mangled by everything between
you and the program.
