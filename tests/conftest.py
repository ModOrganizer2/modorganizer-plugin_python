# -*- encoding: utf-8 -*-

import os
import sys
from pathlib import Path


def pytest_configure():
    global app

    os.add_dll_directory(Path(os.getenv("QT_ROOT")).joinpath("bin"))
    os.add_dll_directory(Path(os.getenv("UIBASE_PATH")).parent)

    from PyQt6.QtWidgets import QApplication

    app = QApplication(sys.argv)
