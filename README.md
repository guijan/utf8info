# utf8info - print information about utf-8, codepoints, and grapheme clusters
It's impossible to handle Unicode with the Standard C APIs.
This is a toy program that tries to see how far you can go anyway.

If you experiment with the text inside [test.txt](test.txt), you will get a
grasp of what breaks and what doesn't.

Program output:
```Text
$ build/utf8info
Type text to get its Unicode and UTF-8 properties:
> 👨‍👩‍👧‍👧
'👨' width:  2, bytes:  4, hex: 0FF9198A
'??' width:  0, bytes:  3, hex: 2E08D8
'👩' width:  2, bytes:  4, hex: 0FF9199A
'??' width:  0, bytes:  3, hex: 2E08D8
'👧' width:  2, bytes:  4, hex: 0FF9197A
'??' width:  0, bytes:  3, hex: 2E08D8
'👧' width:  2, bytes:  4, hex: 0FF9197A
> !
Grapheme mode.
> 👨‍👩‍👧‍👧
'👨‍👩‍👧‍👧' width:  2, bytes: 25, codepoints:  7, hex: 0FF9198A2E08D80FF9199A2E08D80FF9197A2E08D80FF9197A
```
Do note that essentially every program (including this one) has broken Unicode
handling so both the input and the output will be mangled by everything between
you and your terminal.
