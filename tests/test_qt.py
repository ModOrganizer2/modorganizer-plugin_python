import pytest
from PyQt6.QtCore import QDateTime, Qt
from PyQt6.QtWidgets import QWidget

m = pytest.importorskip("mobase_tests.qt")


def test_qstring():
    assert m.qstring_to_stdstring("test") == "test"
    assert m.stdstring_to_qstring("test") == "test"

    assert m.qstring_to_stdstring("éàüö") == "éàüö"
    assert m.stdstring_to_qstring("éàüö") == "éàüö"

    assert m.qstring_to_int("2") == 2
    assert m.int_to_qstring(2) == "2"


def test_qstringlist():
    assert m.qstringlist_join([""], "--") == ""
    assert m.qstringlist_join(["a", "b"], "") == "ab"
    assert m.qstringlist_join(["x", "y"], ";") == "x;y"

    assert m.qstringlist_at(["x", "y"], 0) == "x"
    assert m.qstringlist_at(["x", "y"], 1) == "y"


def test_qmap():
    assert m.qmap_to_length({"t1": "abc", "t2": "o", "t3": ""}) == {
        "t1": 3,
        "t2": 1,
        "t3": 0,
    }


def test_qdatetime():
    assert m.datetime_from_string("2022-03-01") == QDateTime(2022, 3, 1, 0, 0)

    date = QDateTime(1995, 5, 20, 0, 0)
    assert (
        m.datetime_from_string(
            date.toString(Qt.DateFormat.TextDate), Qt.DateFormat.TextDate
        )
        == date
    )

    assert m.datetime_to_string(date) == "1995-05-20T00:00:00"
    assert m.datetime_to_string(date, Qt.DateFormat.TextDate) == date.toString(
        Qt.DateFormat.TextDate
    )
