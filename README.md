Run with ./calc "$(<df.txt)" "$(<input.txt)"

In "input.txt" put the numeric values of variables, must be 0-9

In "df.txt" the first 2 lines are input variables and internal variables. Input variables are just characters that are correlated to the values in input.txt, internal variables are just processes. The rest of the file is the actual commands. 
"a -> p0" would, for example, set the value of p0 to the value of a. 
"+ a -> p1" would add the value of a to the value of p1.

