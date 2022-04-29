import os
from pathlib import Path


def pytest_configure():
    os.add_dll_directory(Path(os.getenv("QT_ROOT")).joinpath("bin"))
    os.add_dll_directory(Path(os.getenv("UIBASE_PATH")).parent)
