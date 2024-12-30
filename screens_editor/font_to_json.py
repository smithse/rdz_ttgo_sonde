import re
import json
import os
import sys
import argparse

def parse_gfx_header(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # Extract the Bitmap array
    bitmap_match = re.search(r'const uint8_t \w+Bitmap(s?)\[\] (.*?)= {(.*?)};', content, re.S)
    bitmap_data = []
    if bitmap_match:
        # Remove all unwanted characters (newlines, spaces) and split by commas
        raw_bitmap_data = bitmap_match.group(3)
        print(raw_bitmap_data)
        cleaned_data = raw_bitmap_data.replace('\n', '').replace(' ', '').strip()
        bitmap_data = [int(x, 16) for x in cleaned_data.split(',') if x]
    else:
        raise("bitmap not found")

    # Extract the Glyph array
    glyphs_match = re.search(r'const GFXglyph \w+Glyphs\[\] (.*?)= {(.*?)};', content, re.S)
    if not glyphs_match:
        print("\n=== Glyph Match Failed ===")
        print("Could not match the glyph array in the input!")
        return {}

    raw_glyph_data = glyphs_match.group(2)

    # Step 3: Debug Raw Glyph Data
    print("\n=== Raw Glyph Data ===")
    print(raw_glyph_data[:500])  # Print the first 500 characters of glyph data

    # Match all glyph definitions
    glyph_data = []
    glyph_matches = list(re.finditer(
        r'\{\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(-?\d+),\s*(-?\d+)\s*\}',
        raw_glyph_data
    ))

    # Step 4: Debug Glyph Matches
    print("\n=== Glyph Matches Found ===")
    print(len(glyph_matches))  # Number of matches found
    for match in glyph_matches[:5]:  # Print first 5 matches for inspection
        print(match.group(0))

    # Process the matches
    for match in glyph_matches:
        glyph_data.append({
            "bitmapOffset": int(match.group(1)),
            "width": int(match.group(2)),
            "height": int(match.group(3)),
            "xAdvance": int(match.group(4)),
            "xOffset": int(match.group(5)),
            "yOffset": int(match.group(6)),
        })

    # Step 5: Debug Final Glyph Data
    print("\n=== Parsed Glyph Data ===")
    print(glyph_data[:5])  # Print first 5 glyphs for inspection

    # Extract the Font metadata
    font_match = re.search(
        r'const GFXfont \w+ (\w+? ?)= {\s*.*?,\s*.*?,\s*(0x[0-9A-Fa-f]+),\s*(0x[0-9A-Fa-f]+),\s*(\d+)\s*};',
        #r'const GFXfont \w+ = {\s*.*?,\s*.*?,\s*(0x[0-9A-Fa-f]+),\s*(0x[0-9A-Fa-f]+),\s*(\d+)\s*};',
        content,
        re.S
    )
    font_metadata = {}
    if font_match:
        font_metadata = {
            "firstChar": int(font_match.group(2), 16),
            "lastChar": int(font_match.group(3), 16),
            "yAdvance": int(font_match.group(4)),
        }

    # Debugging Font Metadata
    print("\n=== Font Metadata ===")
    print(font_metadata)

    return {
        "bitmap": bitmap_data,
        "glyphs": glyph_data,
        "font": font_metadata,
    }


def save_as_json(data, output_file):
    with open(output_file, 'w') as file:
        json.dump(data, file, indent=4)

def main():
    parser = argparse.ArgumentParser(description="Convert GFX header to JSON.")
    parser.add_argument("input_file", help="Input GFX header file")
    parser.add_argument("--outdir", help="Optional output directory", default=None)

    args = parser.parse_args()

    input_file = args.input_file
    output_dir = args.outdir

    # Extract the base name and replace .h with .json
    base_name = os.path.basename(input_file)
    output_file_name = os.path.splitext(base_name)[0] + ".json"

    # If --outdir is specified, use it; otherwise, use the current directory
    if output_dir:
        os.makedirs(output_dir, exist_ok=True)
        output_file = os.path.join(output_dir, output_file_name)
    else:
        output_file = output_file_name

    # Parse and save
    gfx_data = parse_gfx_header(input_file)
    save_as_json(gfx_data, output_file)

if __name__ == "__main__":
    main()


