import tideclock
from pprint import pprint


def test_tides():
    print(tideclock.get_tides())


def test_status():
    status_json = tideclock.get_status()
    pprint(status_json)
    FIELDS = ['current_time',
              'dac_values',
              'display_enabled',
              'display_forced',
              'display_mode',
              'main_bus',
              'neopixel_colors',
              'position']
    for fld in FIELDS:
        assert fld in status_json
    for string_fld in ['current_time', 'display_mode']:
        assert isinstance(status_json[string_fld], str)
    for list_fld in ['dac_values', 'neopixel_colors']:
        assert isinstance(status_json[list_fld], list)
        assert len(status_json[list_fld]) == 24
    for bool_fld in ['display_enabled', 'display_forced']:
        assert isinstance(status_json[bool_fld], bool)
    assert isinstance(status_json['position'], int)
    assert isinstance(status_json['main_bus'], float)


def test_config():
    config_json = tideclock.get_config()
    pprint(tideclock.get_config())
    for fld in ['time_0_pos', 'time_0_pixel', 'brightness_day', 'brightness_night', 'dac_calibrations']:
        assert (fld in config_json)
        if fld != 'dac_calibrations':
            assert isinstance(config_json[fld], int)
        else:
            assert isinstance(config_json[fld], list)
            assert len(config_json[fld]) == 24


def test_seek():
    assert tideclock.seek(50, True) == 50
    assert tideclock.seek(20, False) == 70


def test_time_zero_position():
    assert tideclock.seek(100, True) == 100
    assert tideclock.set_time_zero_position() == 100
    assert tideclock.seek(200, True) == 200
    assert tideclock.get_time_zero_position() == 100
