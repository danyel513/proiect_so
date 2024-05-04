#!/bin/bash

# Check if arguments are provided
if [ $# -eq 0 ]; then
    exit 1
fi

# Function to evaluate the file
evaluate_file() {
    file="$1"

    # Count the number of lines, words, and characters
    num_lines=$(wc -l < "$file")
    num_words=$(wc -w < "$file")
    num_chars=$(wc -m < "$file")

    # Check the limits for number of lines, words, and characters
    if [ "$num_lines" -lt 3 ] && [ "$num_words" -gt 1000 ] && [ "$num_chars" -gt 2000 ]; then
        exit 0
    fi

    # Check for non-ASCII characters
    if grep -q -P '[^\x00-\x7F]' "$file"; then
        exit 0
    fi

    # Check for the presence of keywords
    keywords=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
    for keyword in "${keywords[@]}"; do
        if grep -q -i "$keyword" "$file"; then
            exit 0
        fi
    done
}

# Check if the file exists and is a regular file
file_path="$1"
if [ ! -f "$file_path" ]; then
    exit 1
fi

# Evaluate the file
evaluate_file "$file_path"
exit 1
