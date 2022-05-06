import mobase
import pytest

m = pytest.importorskip("mobase_tests.functional")


def test_guessed_string():

    # available functions:
    # - fn_0_arg, fn_1_arg, fn_2_arg
    # - fn_0_or_1_arg, fn_1_or_2_or_3_arg

    assert m.fn_0_arg(lambda: 0) == 0
    assert m.fn_0_arg(lambda: 5) == 5
    assert m.fn_0_arg(lambda x=2: x) == 2
    assert m.fn_0_arg(lambda *args: len(args)) == 0

    assert m.fn_1_arg(lambda x: x, 4) == 4
    assert m.fn_1_arg(lambda *args: sum(args), 8) == 8
    assert m.fn_1_arg(lambda x=2, y=4: x + y, 3) == 7
    assert m.fn_1_arg(lambda x, *args, **kwargs: x + len(args), 5) == 5

    assert m.fn_2_arg(lambda x, y: x * y, 4, 5) == 20
    assert m.fn_2_arg(lambda x, y=3: x * y, 4, 2) == 8
    assert m.fn_2_arg(lambda *args: sum(args), 8, 9) == 17
    assert m.fn_2_arg(lambda x=2, y=4: x + y, 3, 3) == 6
    assert m.fn_2_arg(lambda x, *args, **kwargs: x + len(args), 5, 2) == 6

    assert m.fn_0_or_1_arg(lambda: 3) == 3
    assert m.fn_0_or_1_arg(lambda x: x) == 1

    # the 0 arg is bound first, both are possible, the 0 arg is chosen
    assert m.fn_0_or_1_arg(lambda x=2: x) == 2

    with pytest.raises(TypeError):
        m.fn_1_or_2_or_3_arg(lambda: 0)

    assert m.fn_1_or_2_or_3_arg(lambda x=4: x) == 1
    assert m.fn_1_or_2_or_3_arg(lambda x, y: x + y) == 3  # 1 + 2
    assert m.fn_1_or_2_or_3_arg(lambda x, y, z: x * y * z) == 6  # 1 * 2 * 3

    # the 1 arg is bound first
    assert m.fn_1_or_2_or_3_arg(lambda *args: sum(args)) == 1
