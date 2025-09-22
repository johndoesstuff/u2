filetype plugin indent on

" Line numbering
set number
set rnu

" C indent style
set cindent

" Highlight and incremental search
set hlsearch
set is

" Clear highlight search with double esc
nnoremap <esc><esc> :nohlsearch<CR>

" Always show at least 5 lines around cursor
set scrolloff=5

" Syntax highlighting
syntax on

" Auto-reload files edited
set autoread

" Stupid error noise
set noerrorbells
set novisualbell
set t_vb=
set tm=500

set ai "Auto indent
set si "Smart indent

" Highlight binary literals in C as numbers
augroup c_binary_literals
autocmd!
autocmd FileType c,cpp syntax match cBinary display "0[bB][01]\+"
autocmd FileType c,cpp highlight def link cBinary Number
augroup END

" Show command as typing
set showcmd
