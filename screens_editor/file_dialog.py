from kivy.uix.popup import Popup
from kivy.uix.filechooser import FileChooserListView
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.button import Button


class FileDialogPopup(Popup):
    def __init__(self, on_file_selected, on_cancel, **kwargs):
        super().__init__(**kwargs)
        self.on_file_selected = on_file_selected
        self.on_cancel = on_cancel

        # Layout
        layout = BoxLayout(orientation="vertical")

        # File chooser
        self.filechooser = FileChooserListView(size_hint=(1, 0.9), path="../RX_FSK/data", filters=['*.txt'])
        layout.add_widget(self.filechooser)

        # Buttons
        button_layout = BoxLayout(size_hint=(1, 0.1))
        open_button = Button(text="Open")
        open_button.bind(on_release=self.select_file)
        cancel_button = Button(text="Cancel")
        cancel_button.bind(on_release=self.cancel)

        button_layout.add_widget(open_button)
        button_layout.add_widget(cancel_button)

        layout.add_widget(button_layout)

        self.content = layout

    def select_file(self, instance):
        """
        Handle file selection and pass it to the callback.
        """
        if self.filechooser.selection:
            selected_file = self.filechooser.selection[0]
            self.on_file_selected(selected_file)
            self.dismiss()

    def cancel(self, instance):
        """
        Handle cancel action and pass it to the callback.
        """
        self.on_cancel()
        self.dismiss()

