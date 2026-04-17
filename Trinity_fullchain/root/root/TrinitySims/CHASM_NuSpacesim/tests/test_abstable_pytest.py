import os
import sys
import numpy as np
import pytest

# Ensure the project 'src' is on sys.path for local test runs
_THIS_DIR = os.path.dirname(__file__)
_PROJECT_ROOT = os.path.abspath(os.path.join(_THIS_DIR, '..'))
_SRC_DIR = os.path.join(_PROJECT_ROOT, 'src')
if _SRC_DIR not in sys.path:
    sys.path.insert(0, _SRC_DIR)

import CHASM as ch  # noqa: E402
from CHASM.axis import _abstable_manager  # noqa: E402


@pytest.mark.parametrize("name", ["corsika", "m5", "m5_new"]) 
def test_switch_and_properties(name):
    # Switch to requested abstable
    ch.useAbstable(name)

    # Raw NPZ via manager
    data = _abstable_manager.get_current_data()
    assert isinstance(data, np.lib.npyio.NpzFile)

    # Required keys exist
    for key in ("ecoeff", "wavelength", "height"):
        assert key in data.files, f"Missing key '{key}' in NPZ for {name}"

    ecoeff = data["ecoeff"]
    wl = data["wavelength"]
    h = data["height"]

    # Basic shape checks
    assert ecoeff.ndim == 2, "ecoeff must be 2D (wavelength x height)"
    assert wl.ndim == 1 and h.ndim == 1
    assert ecoeff.shape == (wl.shape[0], h.shape[0]), (
        f"ecoeff shape {ecoeff.shape} must be (len(wavelength), len(height))"
    )

    # Properties exposed via manager
    assert _abstable_manager.ecoeff.shape == ecoeff.shape
    assert _abstable_manager.l_list.shape == wl.shape
    assert _abstable_manager.altitude_list.shape == h.shape


def test_invalid_abstable_raises():
    with pytest.raises(ValueError):
        ch.useAbstable("nonexistent_option")


def test_cache_identity_for_current_abstable():
    # Pick one to test caching
    ch.useAbstable("corsika")
    a = _abstable_manager.get_current_data()
    b = _abstable_manager.get_current_data()
    # Manager should cache and return the same object instance
    assert a is b


def test_switch_changes_dataset_object_identity():
    ch.useAbstable("corsika")
    data_corsika = _abstable_manager.get_current_data()

    ch.useAbstable("m5")
    data_m5 = _abstable_manager.get_current_data()

    # Different selections should not return the same cached object
    assert data_corsika is not data_m5
