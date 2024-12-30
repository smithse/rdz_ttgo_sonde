import argparse
from kivy.app import App
from layout_editor import LayoutEditor
from state import State
from parse_screens import FileParser


class LayoutEditorApp(App):
    def __init__(self, file_path=None, block_name=None, **kwargs):
        super().__init__(**kwargs)
        self.file_path = file_path
        self.block_name = block_name

    def build(self):
        # Initialize the file parser and ttgodata
        parser = FileParser()
        ttgodata = State()  # Renamed for rendering variables

        # Create the layout editor with both parser and ttgodata
        editor = LayoutEditor(parser=parser, ttgodata=ttgodata)

        # Handle optional command-line parameters
        if self.file_path:
            try:
                parser.read_file(self.file_path)
                editor.load_blocks()

                # Select block if specified and exists
                if self.block_name in parser.get_block_names():
                    editor.current_block = self.block_name
                    editor.block_selector.text = self.block_name
                    editor.update_text_editor()
                    editor.drawing_area.redraw(self.block_name, editor.scale)
                else:
                    # Reset to the first block if the block doesn't exist
                    editor.current_block = parser.get_block_names()[0] if parser.get_block_names() else None
                    if editor.current_block:
                        editor.block_selector.text = editor.current_block
                        editor.update_text_editor()
                        editor.drawing_area.redraw(editor.current_block, editor.scale)
            except Exception as e:
                print(f"Error loading file or block: {e}")

        return editor


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run the Layout Editor.")
    parser.add_argument("--file", help="Path to the layout file to load.")
    parser.add_argument("--block", help="Name of the block to load (optional).")
    args = parser.parse_args()

    LayoutEditorApp(file_path=args.file, block_name=args.block).run()
