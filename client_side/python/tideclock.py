import requests
import json

URL = r'http://tideclock.local/'


def get_text_endpoint(name: str) -> str:
    return requests.get(URL + name).text


def post_text_endpoint(name: str, data: str = "") -> str:
    return requests.post(URL + name, data=data).text


def get_tides() -> str:
    return get_text_endpoint('tides')


def get_status() -> dict:
    return json.loads(get_text_endpoint('status'))


def get_config() -> dict:
    return json.loads(get_text_endpoint('config'))


def reset_config() -> dict:
    return json.loads(post_text_endpoint('config/reset'))


def save_config() -> dict:
    return json.loads(post_text_endpoint('config/save'))


def seek(pos: int, abs: bool = True) -> int:
    ep = 'seek/' + ('abs' if abs else 'rel')
    return int(post_text_endpoint(ep, str(pos)))


def seek_time(hr: float) -> int:
    return int(post_text_endpoint('seek/time', str(hr)))


def enable_motion() -> str:
    return post_text_endpoint('motion/enable')


def disable_motion() -> str:
    return post_text_endpoint('motion/disable')


def get_time_zero_position() -> int:
    return int(get_text_endpoint('time_zero'))


def set_time_zero_position() -> int:
    return int(post_text_endpoint('time_zero'))


def parse_dac_set(response: str) -> int:
    return int(response[response.index('>') + 2:], 16)


def set_dac(num: int, value: int) -> int:
    headers = dict(num=str(num), value=str(value))
    response = requests.post(URL + 'dac', headers=headers).text
    return parse_dac_set(response)


def get_dac_calibrate(num: int, pt: int) -> int:
    headers = dict(num=str(num), pt=str(pt))
    response = requests.get(URL + 'dac/calibrate', headers=headers).text
    return parse_dac_set(response)


def set_dac_calibrate(num: int, pt: int) -> int:
    headers = dict(num=str(num), pt=str(pt))
    response = requests.post(URL + 'dac/calibrate', headers=headers).text
    return parse_dac_set(response)


def set_neopixel_brightness(is_day: bool, brightness: int) -> int:
    headers = dict(time=('day' if is_day else 'night'), value=str(brightness))
    response = requests.post(URL + 'neopixel/brightness', headers=headers).text
    return parse_dac_set(response)


def get_neopixel_offset() -> int:
    return int(get_text_endpoint('neopixel/offset'))


def set_neopixel_offset(offset: int) -> int:
    return int(post_text_endpoint('neopixel/offset', str(offset)))


def get_neopixel_direction() -> int:
    return int(get_text_endpoint('neopixel/direction'))


def set_neopixel_direction(dir: int) -> int:
    return int(post_text_endpoint('neopixel/direction', str(dir)))


def disable_display() -> str:
    return post_text_endpoint('display/disable')


def enable_display() -> str:
    return post_text_endpoint('display/enable')


def release_display() -> str:
    return post_text_endpoint('display/release')


def force_display(mode: str) -> str:
    return post_text_endpoint('display/' + mode.lower())
