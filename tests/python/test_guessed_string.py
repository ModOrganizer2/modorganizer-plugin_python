import pytest

import mobase

m = pytest.importorskip("mobase_tests.guessed_string")


def test_guessed_string():
    # empty string
    gs = mobase.GuessedString()

    assert len(gs.variants()) == 0
    assert not gs.variants()
    assert str(gs) == ""

    # automatic conversion from string
    assert m.get_value("test") == "test"
    assert m.get_variants("test") == {"test"}

    # update
    gs = mobase.GuessedString("fallback", mobase.GuessQuality.FALLBACK)
    assert str(gs) == "fallback"

    gs.update("good", mobase.GuessQuality.GOOD)
    assert str(gs) == "good"
    assert gs.variants() == {"fallback", "good"}

    # back-and-forth
    gs = mobase.GuessedString()
    assert str(gs) == ""

    m.set_from_callback(gs, lambda gs: gs.update("test"))
    assert str(gs) == "test"

    assert m.get_from_callback(lambda gs: gs.update("test")) == "test"
