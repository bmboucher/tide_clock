import requests
import json

URL = r'http://tideclock.local/'


def get_text_endpoint(name: str) -> str:
    return requests.get(URL + name).text


def post_text_endpoint(name: str) -> str:
    return requests.post(URL + name).text


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
    ep = URL + 'seek/' + ('abs' if abs else 'rel')
    return int(requests.post(ep, data=str(pos)).text)


def get_time_zero_position() -> int:
    return int(get_text_endpoint('time_zero'))


def set_time_zero_position() -> int:
    return int(post_text_endpoint('time_zero'))
