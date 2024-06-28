import os
import sys
from pathlib import Path
from typing import cast


def pytest_configure():
    global app

    os.add_dll_directory(str(Path(cast(str, os.getenv("QT_ROOT"))).joinpath("bin")))
    os.add_dll_directory(str(os.getenv("UIBASE_PATH")))

    from PyQt6.QtWidgets import QApplication

    app = QApplication(sys.argv)
