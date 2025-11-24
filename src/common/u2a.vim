" ~/.vim/syntax/u2a.vim

syntax keyword u2aInstruction mov li ld st add sub mul div and or xor not shl shr cmp jmp je jne jl jg

syntax match u2aNumber /\<\d\+\>/
syntax match u2aHex /0x[0-9A-Fa-f]\+/
syntax match u2aBin /0b[0-1]\+/

syntax match u2aComment /;.*$/

highlight link u2aInstruction Keyword

highlight link u2aNumber Number
highlight link u2aHex Number
highlight link u2aBin Number
highlight link u2aComment Comment
