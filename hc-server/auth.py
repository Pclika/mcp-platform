"""API key management for hc.pclika.com"""
import json
import os
import secrets
import time
from pathlib import Path

DEMO_KEY = "pck_demo00000000000000000000000000000"
KEYS_FILE = os.environ.get("KEYS_FILE", "keys.json")


class AuthManager:
    def __init__(self, keys_file: str = KEYS_FILE):
        self.keys_file = Path(keys_file)
        self._keys: dict = {}
        self._load()

    def _load(self):
        if self.keys_file.exists():
            with open(self.keys_file) as f:
                self._keys = json.load(f)
        else:
            self._keys = {
                DEMO_KEY: {
                    "label": "Demo Key",
                    "created": int(time.time()),
                    "demo": True,
                }
            }
            self._save()

    def _save(self):
        with open(self.keys_file, "w") as f:
            json.dump(self._keys, f, indent=2)

    def validate(self, key: str) -> bool:
        if not key or not key.startswith("pck_"):
            return False
        return key in self._keys

    def is_demo(self, key: str) -> bool:
        return key == DEMO_KEY

    def create(self, label: str = "API Key") -> str:
        key = "pck_" + secrets.token_hex(16)
        self._keys[key] = {"label": label, "created": int(time.time())}
        self._save()
        return key

    def revoke(self, key: str) -> bool:
        if key in self._keys and not self._keys[key].get("demo"):
            del self._keys[key]
            self._save()
            return True
        return False

    def list_keys(self) -> list:
        return [
            {"key": k[:8] + "..." + k[-4:], "label": v.get("label", ""), "created": v.get("created")}
            for k, v in self._keys.items()
        ]
