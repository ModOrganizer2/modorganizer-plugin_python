import pytest

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
