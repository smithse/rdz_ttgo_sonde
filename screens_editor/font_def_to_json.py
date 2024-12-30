import json
import re

def parse_fixed_width_font(file_path):
    with open(file_path, 'r') as f:
        content = f.read()

    # Extract the font array
    match = re.search(r'static const unsigned char font\[\] PROGMEM = {([^}]+)};', content, re.DOTALL)
    if not match:
        raise ValueError("Font array not found in the file.")

    # Get the array content and split it into bytes
    font_data = match.group(1)
    font_bytes = [int(byte.strip(), 16) for byte in font_data.split(',') if byte.strip()]

    # Verify size
    if len(font_bytes) != 256 * 5:  # Should include all 256 characters
        raise ValueError("Font array size mismatch. Expected 256 characters (256 * 5 bytes).")

    # Create the bitmap array (as a single flat array)
    bitmap = []
    for i in range(256):  # Process all 256 characters
        # Step 1: Get the 5 columns
        char_bytes = font_bytes[i * 5:(i + 1) * 5]  # Columns for this character
        print(f"Character {i}: Columns: {char_bytes}")

        # Step 2: Transform into 8 rows of 5 bits
        rows = [0] * 8
        for row in range(8):  # 8 rows
            for col in range(5):  # 5 columns
                if char_bytes[col] & (1 << row):  # Check bit at row in column
                    rows[row] |= (1 << (4 - col))  # Set bit in the corresponding row
        print(f"Character {i}: Rows: {rows}")

        # Step 3: Densely pack the 8 rows into 5 bytes bit by bit
        packed_bytes = densly_pack_rows_bit_by_bit(rows)
        print(f"Character {i}: Packed Bytes: {packed_bytes}")

        bitmap.extend(packed_bytes)

    # Create the glyph array
    glyphs = []
    for i in range(256):
        glyphs.append({
            "bitmapOffset": i * 5,  # Each glyph uses 5 packed bytes
            "width": 5,            # Fixed width for all glyphs
            "height": 8,           # Fixed height
            "xAdvance": 6,         # 5 pixels + 1 for spacing
            "xOffset": 0,          # No horizontal offset
            "yOffset": -5           # No vertical offset
        })

    # Font metadata
    font = {
        "font": {
            "firstChar": 0,  # Full ASCII range
            "lastChar": 255,
            "yAdvance": 8    # Line height for 5x8 font
        },
        "bitmap": bitmap,    # Densely packed row-first bitmap data
        "glyphs": glyphs     # Glyph metadata
    }

    return font

def densly_pack_rows_bit_by_bit(rows):
    """
    Pack 8 rows of 5 bits each into 5 bytes, bit by bit.
    """
    packed_bytes = [0] * 5  # Resulting 5 bytes

    bit_index = 0  # Track the overall bit position in the output
    for row in rows:  # Iterate over each row
        for col in range(5):  # Iterate over each column bit in the row
            if row & (1 << (4 - col)):  # Check if the bit is set in the row
                byte_index = bit_index // 8  # Determine which byte this bit belongs to
                bit_position = bit_index % 8  # Determine the bit position within the byte
                packed_bytes[byte_index] |= (1 << (7 - bit_position))  # Set the bit
            bit_index += 1  # Move to the next bit

    return packed_bytes

def save_as_json(data, output_file):
    with open(output_file, 'w') as f:
        json.dump(data, f, indent=2)

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Convert Arduino fixed-width font to JSON.")
    parser.add_argument("input_file", help="Input Arduino bitmap font header file")
    parser.add_argument("output_file", help="Output JSON file")
    args = parser.parse_args()

    # Parse and convert
    font_json = parse_fixed_width_font(args.input_file)
    save_as_json(font_json, args.output_file)

    print(f"Font successfully converted and saved to {args.output_file}")

