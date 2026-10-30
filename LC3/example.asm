; LC3 assembly code works in this format
; [LABEL] OPCODE [OPERANDS] ; [COMMENT]

.ORIG x3000 ; mem addr where the program will be loaded
LEA R0, HELLO_STR ; load addr to register
string into R0
PUTS
HALT
HELLO_STR .STRINGZ "HELLO WORLD!"

.END
