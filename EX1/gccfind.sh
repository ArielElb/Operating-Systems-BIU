#!/bin/bash

# get the directory of the script
script_dir=$(dirname "$0")
# get the second argument (directory path)
path="$script_dir/$1"
# get the third argument (search term)
word="$2"
# check if fourth argument is "-r"

#check if the number of arguments is less than 2
if [ "$#" -lt 2 ]; then
    echo "Not enough parameters"
    exit 1
fi

if [ "$3" = "-r" ]; then
    recursive=1
else
    recursive=0
fi

# function to compile the file
compile_file() {
    # compile the file with -w flag to ignore warnings
    outfile=${1%.c}.out
    gcc -w "$1" -o "$outfile"
}

# function to loop through the files in a directory
loop_dir() {
    local dir=$1
    for file in "$dir"/*; do
        # skip directories
        if [ -d "$file" ]; then
            if [ "$recursive" -eq 1 ]; then
                # recursively call the function with the directory
                loop_dir "$file"
            fi
            continue
        fi
        # check convert file to .c file

        # check if the file is an executable and delete it if it is
        if [[ "$file" == *.out ]]; then
            # check if the word is found in the file.c
            rm "$file"
            if [ -n "$(grep -r -i -n -E "\b"$word"\b|\b"$word"\W" "${file%.out}.c")" ]; then
                # compile the file
                compile_file "${file%.out}.c"
            fi
            continue
        fi
        # check if the file is a C file and if the search term is found in the file
        # -e flag is used to enable extended regular expressions
        # -i flag is used to ignore case
        # -n is for non empty string
        # \b is used to match the word boundary
        if [[ "$file" == *.c ]] && [ -n "$(grep -r -i -n -E "\b"$word"\b|\b"$word"\W" "$file")" ]; then
            compile_file "$file"
        fi
    done
}

## if the directory does not exist, exit the script
#if [ ! -d "$path" ]; then
#    echo "Directory $path does not exist"
#    exit 1
#fi

# loop through the files in the directory
loop_dir "$path"

# End of script
