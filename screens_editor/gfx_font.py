import traceback
import json
from kivy.graphics import Rectangle, Color


# gfx coordinates are (0,0) top left
# canvas coordinates are (0,0) bottom left

class GFXFont:
    def __init__(self, json_path):
        with open(json_path, 'r') as f:
            self.font_data = json.load(f)
        # to match Display.cpp
        # find largest yoffset
        min_yoffset = 0
        for glyph in self.font_data["glyphs"]:
            if glyph["yOffset"] < min_yoffset:
                min_yoffset = glyph["yOffset"]
        max_height = 0
        for glyph in self.font_data["glyphs"]:
            h = glyph["yOffset"] - min_yoffset + glyph["height"]
            if h > max_height:
                max_height = h
        print("min_yoffset: ", min_yoffset, "max_height:", max_height)
        self.base_line = self.font_data["font"]["yAdvance"] * 2 / 3   # offset from top to baseline
        self.yclear = max_height

    def render_glyph(self, canvas, glyph, x, y, scale=1, fg=(1, 1, 1), area_height=0):
        bitmap = self.font_data["bitmap"]
        width = glyph["width"]
        height = glyph["height"]
        offset = glyph["bitmapOffset"]
        y_offset = glyph["yOffset"]
        print("render_glyph: ", glyph, width, height, offset )
        with canvas:
            # Draw each pixel of the glyph
            for row in range(height):
                for col in range(width):
                    byte_index = offset + (row * width + col) // 8
                    bit_index = (row * width + col) % 8
                    if bitmap[byte_index] & (1 << (7 - bit_index)):
                        Color(*fg)  # Foreground color
                        Rectangle(
                            pos=(x + col * scale, area_height - y + (- self.base_line - y_offset - row - 1) * scale),
                            size=(scale, scale),
                        )

    def render_text(self, canvas, text, x, y, scale=1, width=0, fg=(1, 1, 1), bg=None, area_height=0):
      try:
        glyphs = self.font_data["glyphs"]
        font_metadata = self.font_data["font"]

        # Compute text width 
        text_width = 0

        print("Text is '"+text+"'")
        for char in text:
            char_code = ord(char)
            if font_metadata["firstChar"] <= char_code <= font_metadata["lastChar"]:
                glyph_index = char_code - font_metadata["firstChar"]
                glyph = glyphs[glyph_index]
                text_width += glyph["xAdvance"]

        if width < 0:
            x += (-width - text_width) * scale
        if width == 0:
            width = text_width
        print("Calculated width is ",text_width," new x is ",x,"y is",y)

        # Draw the background if specified
        if bg:
            with canvas:
                Color(*bg)
                Rectangle(
                    pos=(x, area_height - y - self.yclear * scale),
                    size=(width * scale, self.yclear * scale),
                )

        # Render each character
        x_cursor = x
        for char in text:
            char_code = ord(char)
            if font_metadata["firstChar"] <= char_code <= font_metadata["lastChar"]:
                glyph_index = char_code - font_metadata["firstChar"]
                glyph = glyphs[glyph_index]

                # Render the glyph
                self.render_glyph(canvas, glyph, x_cursor, y, scale=scale, fg=fg, area_height=area_height)

                # Advance the cursor
                x_cursor += glyph["xAdvance"] * scale
      except Exception as e:
        print("Failed to render text")
        traceback.print_exc()

