import pytest

import mobase

m = pytest.importorskip("mobase_tests.filetree")


def test_filetype():
    FT = mobase.FileTreeEntry.FileTypes

    assert mobase.FileTreeEntry.FILE == FT.FILE
    assert m.is_file(FT.FILE)
    assert not m.is_directory(FT.FILE)
    assert m.value(FT.FILE) == FT.FILE.value

    assert mobase.FileTreeEntry.DIRECTORY == FT.DIRECTORY
    assert not m.is_file(FT.DIRECTORY)
    assert m.is_directory(FT.DIRECTORY)
    assert m.value(FT.DIRECTORY) == FT.DIRECTORY.value

    assert mobase.FileTreeEntry.FILE_OR_DIRECTORY == FT.FILE_OR_DIRECTORY
    assert m.is_file(FT.FILE_OR_DIRECTORY)
    assert m.is_directory(FT.FILE_OR_DIRECTORY)
    assert m.value(FT.FILE_OR_DIRECTORY) == FT.FILE_OR_DIRECTORY.value

    assert FT.FILE_OR_DIRECTORY == FT.FILE | FT.DIRECTORY
    assert m.is_file(FT.FILE | FT.DIRECTORY)
    assert m.is_directory(FT.FILE | FT.DIRECTORY)
    assert m.value(FT.FILE | FT.DIRECTORY) == (FT.FILE.value | FT.DIRECTORY.value)

    assert m.is_file(FT.FILE_OR_DIRECTORY & FT.FILE)
    assert not m.is_directory(FT.FILE_OR_DIRECTORY & FT.FILE)

    assert not m.is_file(FT.FILE_OR_DIRECTORY & FT.DIRECTORY)
    assert m.is_directory(FT.FILE_OR_DIRECTORY & FT.DIRECTORY)

    assert m.is_file(FT.FILE_OR_DIRECTORY & ~FT.DIRECTORY)
    assert not m.is_directory(FT.FILE_OR_DIRECTORY & ~FT.DIRECTORY)
