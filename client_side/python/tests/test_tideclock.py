import tideclock
from pprint import pprint


def test_tides():
    print(tideclock.get_tides())


def test_status():
    status_json = tideclock.get_status()
    pprint(status_json)
    FIELDS = ['current_time',
              'dac_values',
              'displayed_time',
              'motion_enabled',
              'display_enabled',
              'display_forced',
              'display_mode',
              'main_bus',
              'neopixel_colors',
              'position',
              'avg_loop_time']
    for fld in FIELDS:
        assert fld in status_json
    for string_fld in ['current_time', 'display_mode']:
        assert isinstance(status_json[string_fld], str)
    for list_fld in ['dac_values', 'neopixel_colors']:
        assert isinstance(status_json[list_fld], list)
        assert len(status_json[list_fld]) == 24
    for bool_fld in ['motion_enabled', 'display_enabled', 'display_forced']:
        assert isinstance(status_json[bool_fld], bool)
    for int_fld in ['position', 'avg_loop_time']:
        assert isinstance(status_json[int_fld], int)
    for float_fld in ['main_bus', 'displayed_time']:
        assert isinstance(status_json[float_fld], float)


def test_config():
    config_json = tideclock.get_config()
    pprint(tideclock.get_config())
    for fld in ['time_0_pos', 'time_0_pixel', 'pixel_dir',
                'brightness_day', 'brightness_night', 'dac_calibrations']:
        assert (fld in config_json)
        if fld != 'dac_calibrations':
            assert isinstance(config_json[fld], int)
        else:
            assert isinstance(config_json[fld], list)
            assert len(config_json[fld]) == 24


def test_motion_enable():
    assert tideclock.disable_motion() == 'Motion disabled'
    assert not tideclock.get_status()['motion_enabled']
    assert tideclock.enable_motion() == 'Motion enabled'
    assert tideclock.get_status()['motion_enabled']


def test_seek():
    assert tideclock.disable_motion() == 'Motion disabled'
    assert tideclock.seek(50, True) == 50
    assert tideclock.get_status()['position'] == 50
    assert tideclock.seek(20, False) == 70
    assert tideclock.get_status()['position'] == 70
    time_pos = tideclock.seek_time(5.0)
    status_json = tideclock.get_status()
    assert status_json['position'] == time_pos
    assert 4.995 < status_json['displayed_time'] < 5.005
    assert tideclock.enable_motion() == 'Motion enabled'


def test_time_zero_position():
    assert tideclock.disable_motion() == 'Motion disabled'
    assert tideclock.seek(100, True) == 100
    assert tideclock.set_time_zero_position() == 100
    assert tideclock.seek(200, True) == 200
    assert tideclock.get_time_zero_position() == 100
    assert tideclock.enable_motion() == 'Motion enabled'


def test_set_dac():
    assert tideclock.set_dac(0, 100) == 100


def test_dac_calibrate():
    assert tideclock.disable_display() == 'Display disabled'
    assert tideclock.set_dac(3, 150) == 150
    assert tideclock.set_dac_calibrate(3, 0) == 150
    assert tideclock.set_dac(3, 200) == 200
    assert tideclock.get_dac_calibrate(3, 0) == 150
    assert tideclock.enable_display() == 'Display enabled'


def test_neopixel_brightness():
    assert tideclock.set_neopixel_brightness(True, 10) == 10
    assert tideclock.set_neopixel_brightness(False, 20) == 20
    config_json = tideclock.get_config()
    assert config_json['brightness_day'] == 10
    assert config_json['brightness_night'] == 20


def test_neopixel_offset():
    assert tideclock.set_neopixel_offset(3) == 3
    assert tideclock.get_neopixel_offset() == 3
    assert tideclock.get_config()['time_0_pixel'] == 3
    assert tideclock.set_neopixel_offset(24) == 0
    assert tideclock.get_neopixel_offset() == 0
    assert tideclock.get_config()['time_0_pixel'] == 0


def test_neopixel_direction():
    assert tideclock.set_neopixel_direction(-3) == -1
    assert tideclock.get_neopixel_direction() == -1
    assert tideclock.get_config()['pixel_dir'] == -1
    assert tideclock.set_neopixel_direction(3) == 1
    assert tideclock.get_neopixel_direction() == 1
    assert tideclock.get_config()['pixel_dir'] == 1


def test_display_enable():
    assert tideclock.disable_display() == 'Display disabled'
    assert not tideclock.get_status()['display_enabled']
    assert tideclock.enable_display() == 'Display enabled'
    assert tideclock.get_status()['display_enabled']


def test_force_display():
    for mode in ['NORMAL', 'DEPOISON', 'WAVE', 'ALTERNATING', 'RANDOM', 'PULSE', 'DEMO']:
        assert tideclock.force_display(mode) == 'Setting display mode to ' + mode
        status_json = tideclock.get_status()
        assert status_json['display_forced']
        assert status_json['display_mode'] == mode
    assert tideclock.release_display() == 'Display released'
    assert not tideclock.get_status()['display_forced']
