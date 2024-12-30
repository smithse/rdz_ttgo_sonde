import re
import json
import os
import sys

def parse_u8x8_font(c_file_path, output_dir=None):
    def rotate_bitmap(bitmap, tiles_x, tiles_y):
        """
        Rotate the bitmap data for each character from row-major to column-major order
        and flip it horizontally by adjusting the row indexing.
        :param bitmap: The raw bitmap data.
        :param tiles_x: Number of tiles in the x-dimension (width / 8).
        :param tiles_y: Number of tiles in the y-dimension (height / 8).
        :return: The rotated and flipped bitmap.
        """
        char_size = 8 * tiles_y  # Number of bytes per character
        num_chars = len(bitmap) // (char_size * tiles_x)
        rotated_bitmap = []

        for char_index in range(num_chars):
            char_bitmap = bitmap[char_index * char_size * tiles_x : (char_index + 1) * char_size * tiles_x]

            for tile_y in range(tiles_y):
                for tile_x in range(tiles_x):
                    # Extract the current tile
                    tile_start = (tile_y * tiles_x + tile_x) * 8
                    tile = char_bitmap[tile_start : tile_start + 8]

                    # Rotate and flip the tile by flipping row indices
                    rotated_tile = [0] * 8
                    for row in range(8):
                        for col in range(8):
                            rotated_tile[col] |= ((tile[7 - row] >> col) & 1) << row

                    rotated_bitmap.extend(rotated_tile)

        return rotated_bitmap

    with open(c_file_path, 'r') as file:
        content = file.read()

    # Match the array declaration and body
    match = re.search(
        r'const\s+uint8_t\s+(.*?)=[^"]*(".*?);',
        content,
        re.DOTALL
    )
    if not match:
        raise ValueError("Could not find the font array in the provided C file.")

    # Extract and clean the array body
    array_body = match.group(2)
    lines = array_body.splitlines()
    cleaned_lines = [line.strip() for line in lines if line.strip()]  # Remove surrounding whitespace
    cleaned_data = ''.join(cleaned_lines).replace('"', '')  # Concatenate lines and remove quotes

    # Decode octal escape sequences
    cleaned_data = re.sub(r'\\([0-9]{1,3})', lambda m: chr(int(m.group(1), 8)), cleaned_data)

    # Convert cleaned data into raw bytes
    raw_bytes = [ord(char) for char in cleaned_data]

    # Parse the header (first 4 bytes)
    first_char, last_char, tiles_x, tiles_y = raw_bytes[:4]
    bitmap = raw_bytes[4:]  # The rest is bitmap data

    # Rotate the bitmap data
    rotated_bitmap = rotate_bitmap(bitmap, tiles_x, tiles_y)

    # Generate glyphs data with `bitmapOffset`
    glyphs = []
    char_width = tiles_x * 8
    char_height = tiles_y * 8
    x_advance = char_width
    offset = 0

    for char_code in range(first_char, last_char + 1):
        glyph = {
            "height": char_height,
            "width": char_width,
            "xAdvance": x_advance,
            "xOffset": 0,
            "yOffset": -8 * tiles_y * 2 / 3,
            "bitmapOffset": offset
        }
        glyphs.append(glyph)
        offset += char_width * tiles_y

    # Generate font metadata
    font_metadata = {
        "firstChar": first_char,
        "lastChar": last_char,
        "yAdvance": char_height,
    }

    # Combine into JSON structure
    json_data = {
        "bitmap": rotated_bitmap,
        "glyphs": glyphs,
        "font": font_metadata,
    }

    # Determine output file name
    base_name = os.path.basename(c_file_path)
    json_file_name = os.path.splitext(base_name)[0] + ".json"
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        json_file_path = os.path.join(output_dir, json_file_name)
    else:
        json_file_path = json_file_name

    # Write to JSON file
    with open(json_file_path, 'w') as json_file:
        json.dump(json_data, json_file, indent=4)

    print(f"Font successfully converted and saved to {json_file_path}.")

# Main entry point for command-line usage
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 font_u8x8_to_json.py <input_file.c> [output_directory]")
        sys.exit(1)

    input_file = sys.argv[1]
    output_directory = sys.argv[2] if len(sys.argv) > 2 else None

    parse_u8x8_font(input_file, output_directory)

