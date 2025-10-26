#
# Copyright (C) 2025 TheProgxy <theprogxy@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#

#!/bin/bash

# TODO: Implement decompression/compression testing

echo "-------------------------"
echo "| ZLIB automatic tester |"
echo "-------------------------"

# test_folders=("./testdata/good" "./testdata/large" "./testdata/golden-decompression" "./testdata/decodecorpus_files")
# bad_test_folders=("./testdata/bad")

# Count total .zst files
total_files=0
# for folder in "${test_folders[@]}"; do
#     (( total_files += $(find "$folder" -maxdepth 1 -type f -name "*.zst" | wc -l) ))
# done

# for folder in "${bad_test_folders[@]}"; do
# 	(( total_files += $(find "$folder" -maxdepth 1 -type f -name "*.zst" | wc -l) ))
# done

tested=0
passed=0
failed=0

# Function to print progress bar
print_progress() {
    local progress=$(( tested * 100 / total_files ))
    echo -ne "Processing: $tested / $total_files [$progress%]\r"
}

current_time=0
# Capture the time before execution
if date +%s%N 2>/dev/null | grep -q '[0-9]N'; then
    # macOS: Use an alternative approach
	current_time=$(date +%s$(printf "%09d" $(( $(date +%N 2>/dev/null || echo 0) ))))
else
    # Linux: Use the standard command
	current_time=$(date +%s%N)
fi

for folder in "${test_folders[@]}"; do
    for f in "$folder"/*.zst; do
        # Skip if no files exist
        [[ -e "$f" ]] || continue

        (( tested++ ))
        print_progress

        # Perform an operation (example: decompress the .zst file)
        rm -f resf
        out=$(./zlib_tester $f resf 2>&1)
        if [[ $? -ne 0 ]]; then
            echo "Error decoding: $f"
            ((failed++))
            continue
        fi

        # Check if corresponding file exists and test for diff
        want="${f%%.zst}"
        if [[ -e "$want" ]]; then            
            diff=$(diff -u $want resf)
            if [[ $? -ne 0 ]]; then
                echo "Failed to decode: $f"
                ((failed++))
                continue
            fi
        fi

        ((passed++))
    done
done

for folder in "${bad_test_folders[@]}"; do
    for f in "$folder"/*.zst; do
        # Skip if no files exist
        [[ -e "$f" ]] || continue

		((tested++))
		rm -f resf
		out=$(./zlib_tester $f resf 2>&1)
		if [[ $? -eq 0 ]]; then
			echo "Failed to recognize invalid stream: $f"
			((failed++))
			continue
		fi
	
		((passed++))
	done
done

end_time=0
# Capture the time after execution
if date +%s%N 2>/dev/null | grep -q '[0-9]N'; then
    # macOS: Use an alternative approach
	end_time=$(date +%s$(printf "%09d" $(( $(date +%N 2>/dev/null || echo 0) ))))
else
    # Linux: Use the standard command
	end_time=$(date +%s%N)
fi

execution_time=$((end_time - current_time))  # Calculate elapsed time in nanoseconds

# Format execution time dynamically
if [ "$execution_time" -ge 1000000000 ]; then
    execution_time_str=$(printf "%.3f sec" "$(bc -l <<< "$execution_time / 1000000000")")
elif [ "$execution_time" -ge 1000000 ]; then
    execution_time_str=$(printf "%.3f ms" "$(bc -l <<< "$execution_time / 1000000")")
elif [ "$execution_time" -ge 1000 ]; then
    execution_time_str=$(printf "%.3f µs" "$(bc -l <<< "$execution_time / 1000")")
else
    execution_time_str="${execution_time} ns"
fi

rm -f resf

# Colors using tput
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
RESET=$(tput sgr0)

# Apply color to failed count
if (( failed > 0 )); then
    failed_colored="${RED}${failed}${RESET}"
else
    failed_colored="${GREEN}${failed}${RESET}"
fi

# Print Table
printf "╔══════════╤══════════╤══════════╤════════════╗\n"
printf "║  Tested  │  Passed  │  Failed  │   Time     ║\n"
printf "╟──────────┼──────────┼──────────┼────────────╢\n"
printf "║    %2d   │    %2d   │    %s     │  %8s ║\n" "$tested" "$passed" "$failed_colored" "$execution_time_str"
printf "╚══════════╧══════════╧══════════╧════════════╝\n"

