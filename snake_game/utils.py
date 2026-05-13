import json
import os
from config import HIGH_SCORE_FILE


def load_high_score():
    if os.path.exists(HIGH_SCORE_FILE):
        try:
            with open(HIGH_SCORE_FILE, "r") as f:
                data = json.load(f)
                return data.get("high_score", 0)
        except (json.JSONDecodeError, KeyError):
            return 0
    return 0


def save_high_score(score):
    current = load_high_score()
    if score > current:
        with open(HIGH_SCORE_FILE, "w") as f:
            json.dump({"high_score": score}, f)
        return True
    return False
