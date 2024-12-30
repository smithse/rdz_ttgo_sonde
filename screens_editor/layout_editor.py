from kivy.uix.boxlayout import BoxLayout
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.uix.dropdown import DropDown
from kivy.uix.label import Label
from kivy.uix.scrollview import ScrollView
from kivy.graphics import Color, Rectangle
from drawing_area import DrawingArea, DrawingContainer
from file_dialog import FileDialogPopup


class LayoutEditor(BoxLayout):
    def __init__(self, parser, ttgodata, **kwargs):
        super().__init__(**kwargs)
        self.parser = parser
        self.ttgodata = ttgodata
        self.current_block = None
        self.scale = 1
        self.orientation = 'horizontal'
        self.selected_size = (220, 176)

        # Right Panel Layout
        self.right_panel = BoxLayout(orientation="vertical", size_hint=(0.6, 1))

        # File Input Button (Top of the panel)
        self.file_input_button = Button(text="Select File", size_hint=(1, None), height=40)
        self.file_input_button.bind(on_press=self.open_file_dialog)
        self.right_panel.add_widget(self.file_input_button)

        # Block Selector (Dropdown) # Block Selector Navigation
        block_nav_layout = BoxLayout(orientation="horizontal", size_hint=(1, None), height=40)
        prev_block_btn = Button(text="<-", size_hint=(None, None), width=40, height=40)
        prev_block_btn.bind(on_press=self.select_prev_block)
        self.block_selector_button = Button(text="Block selection", size_hint=(1, None), height=40)
        self.block_selector_button.bind(on_release=self.open_block_dropdown)
        next_block_btn = Button(text="->", size_hint=(None, None), width=40, height=40)
        next_block_btn.bind(on_press=self.select_next_block)
        block_nav_layout.add_widget(prev_block_btn)
        block_nav_layout.add_widget(self.block_selector_button)
        block_nav_layout.add_widget(next_block_btn)
        self.right_panel.add_widget(block_nav_layout)

        self.dropdown = DropDown(auto_width=True, size_hint=(None, None))
        #self.block_selector_button = Button(
        #    text="Block selection",
        #    size_hint=(1, None),
        #    height=40,
        #)
        #self.block_selector_button.bind(on_release=self.dropdown.open)
        #self.right_panel.add_widget(self.block_selector_button)

        # Save Button
        self.save_button = Button(text="Save", size_hint=(1, None), height=40)
        self.save_button.bind(on_press=self.save_file)
        self.right_panel.add_widget(self.save_button)


        # Re-Parse Button
        self.reparse_button = Button(text="Re-Parse", size_hint=(1, None), height=40)
        self.reparse_button.bind(on_press=self.reparse_file)
        self.right_panel.add_widget(self.reparse_button)

        # Size Selector Dropdown
        self.size_dropdown = DropDown(auto_width=True, size_hint=(None, None))
        self.size_selector_button = Button(
            text="Select Size: TFT 320x240",
            size_hint=(1, None),
            height=40,
        )
        self.size_selector_button.bind(on_release=self.size_dropdown.open)

        # Define size options
        size_options = {
            "LCD 128x64": (128, 64),
            "TFT 220x176": (220, 176),
            "TFT 176x220 (landscape)": (176, 220),
            "TFT 320x240": (320, 240),
            "TFT 240x320 (landscape)": (240, 320),
        }

        # Populate the size dropdown
        for size_name, size_values in size_options.items():
            btn = Button(
                text=size_name,
                size_hint_y=None,
                height=40,
                background_normal="",
                background_color=(1, 1, 1, 1),  # White background
                color=(0, 0, 0, 1),  # Black text
            )
            btn.bind(on_release=lambda instance, name=size_name, values=size_values: self.on_size_selected(name, values))
            self.size_dropdown.add_widget(btn)

        self.right_panel.add_widget(self.size_selector_button)


        # Scale Buttons
        self.scale_buttons = BoxLayout(orientation="horizontal", size_hint=(1, None), height=40)
        for scale in range(1, 5):
            btn = Button(text=f"Scale {scale}", size_hint=(1, None), height=40)
            btn.bind(on_press=lambda instance, s=scale: self.set_scale(s))
            self.scale_buttons.add_widget(btn)
        self.right_panel.add_widget(self.scale_buttons)

        # Drawing Area Container (Full Space Below Buttons)
        self.drawing_area_container = DrawingContainer(parser, ttgodata)
        self.right_panel.add_widget(self.drawing_area_container)

        # Text Editor
        self.text_editor = TextInput(text="", multiline=True, size_hint=(0.4, 1))
        self.text_editor.bind(text=self.on_text_change)

        # Add Widgets to Main Layout
        self.add_widget(self.text_editor)
        self.add_widget(self.right_panel)


    def select_prev_block(self, instance):
        blocks = self.parser.get_block_names()
        if blocks and self.current_block in blocks:
            idx = blocks.index(self.current_block)
            if idx > 0:
                self.on_block_selected(blocks[idx - 1])

    def select_next_block(self, instance):
        blocks = self.parser.get_block_names()
        if blocks and self.current_block in blocks:
            idx = blocks.index(self.current_block)
            if idx < len(blocks) - 1:
                self.on_block_selected(blocks[idx + 1])

    def open_block_dropdown(self, instance):
        self.load_blocks()
        self.dropdown.open(instance)

    def load_blocks(self):
        blocks = self.parser.get_block_names()
        self.dropdown.clear_widgets()
        for block in blocks:
            btn = Button(text=block, size_hint_y=None, height=40)
            btn.bind(on_release=lambda instance: self.on_block_selected(instance.text))
            self.dropdown.add_widget(btn)


    def on_size_selected(self, size_name, size_values):
        self.size_selector_button.text = f"Select Size: {size_name}"
        self.selected_size = size_values
        width, height = size_values
        self.drawing_area_container.update_size(width, height, self.scale)
        self.drawing_area_container.redraw(self.current_block)
        self.size_dropdown.dismiss()

    def set_scale(self, scale):
        self.scale = scale
        width, height = self.selected_size
        self.drawing_area_container.update_size(width, height, scale)
        self.drawing_area_container.redraw(self.current_block)

    def update_block_selector_bg(self, instance, value):
        """
        Update the block selector background when resized or moved.
        """
        self.bg_rect.size = instance.size
        self.bg_rect.pos = instance.pos

    def open_file_dialog(self, instance):
        """
        Open the file selection dialog.
        """
        popup = FileDialogPopup(self.on_file_selected, self.on_file_dialog_cancel)
        popup.open()

    def on_file_selected(self, file_path):
        """
        Handle file selection and load its content.
        """
        self.parser.read_file(file_path)
        self.load_blocks()

    def on_file_dialog_cancel(self):
        """
        Handle cancel action in file dialog.
        """
        print("File dialog canceled.")

    def load_blocks(self):
        """
        Load block names into the dropdown.
        """
        try:
            blocks = self.parser.get_block_names()
            self.dropdown.clear_widgets()

            if not blocks:
                self.block_selector_button.text = "Select block: No Blocks"
                return

            # Populate dropdown with block names
            for block in blocks:
                btn = Button(
                    text=block,
                    size_hint_y=None,
                    height=40,
                    background_normal="",
                    background_color=(1, 1, 1, 1),  # White background for items
                    color=(0, 0, 0, 1),  # Black text color
                )
                btn.bind(on_release=lambda instance: self.on_block_selected(instance.text))
                self.dropdown.add_widget(btn)

            # Set the first block as default
            self.current_block = blocks[0]
            self.block_selector_button.text = "Select block: " + self.current_block
            self.update_text_editor()

        except Exception as e:
            print(f"Error loading blocks: {e}")
            self.block_selector_button.text = "Select block: Error"

    def on_block_selected(self, block_name):
        """
        Handle block selection from the dropdown.
        """
        self.current_block = block_name
        self.block_selector_button.text = "Select block: " + block_name
        self.dropdown.dismiss()
        self.update_text_editor()
        self.drawing_area_container.redraw(self.current_block)

    def update_text_editor(self):
        """
        Update the text editor with the content of the currently selected block.
        """
        if self.current_block:
            self.text_editor.text = self.parser.get_block_content(self.current_block)
        else:
            self.text_editor.text = ""

    def on_text_change(self, instance, value):
        """
        Handle changes to the text editor content.
        """
        if self.current_block:
            block_lines = value.splitlines(keepends=True)
            self.parser.replace_block_content(self.current_block, block_lines)

    def save_file(self, instance):
        """
        Save the in-memory content back to the file.
        """
        self.parser.write_file(self.parser.file_path)

    def reparse_file(self, instance):
        """
        Re-parse the file content and refresh the drawing area.
        """
        self.drawing_area_container.redraw(self.current_block)

