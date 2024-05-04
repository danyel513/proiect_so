#!/bin/bash

# Check if arguments are provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <file_path>"
    exit 1
fi

# Function to count lines, words, and characters in a file
count_file_stats()
{
    file="$1"
    num_lines=$(wc -l < "$file")
    num_words=$(wc -w < "$file")
    num_chars=$(wc -m < "$file")
    echo "Number of lines: $num_lines"
    echo "Number of words: $num_words"
    echo "Number of characters: $num_chars"
}

# Function to search for keywords in a file
search_keywords() {
    file="$1"
    keywords=("corrupt" "dangerous" "risk" "attack" "malware" "malicious")
    for keyword in "${keywords[@]}"; do
        grep -q "$keyword" "$file"
        if [ $? -eq 0 ]; then
            echo "File '$file' contains keyword '$keyword'."
            exit 0
        fi
    done
}

# Check if the file exists
file_path="$1"
if [ ! -f "$file_path" ]; then
    echo "Error: File '$file_path' does not exist or is not a regular file."
    exit 1
fi

# Count file statistics
count_file_stats "$file_path"

# Search for keywords in the file
search_keywords "$file_path"

# If no keyword is found, the file is considered safe
echo "File '$file_path' is safe."
exit 1