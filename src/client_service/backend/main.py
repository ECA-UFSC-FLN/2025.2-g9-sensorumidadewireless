"""
File: main.py
Project: Estufa Dashboard API
Created: Monday, 6th October 2025 11:27:04 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""


if __name__ == "__main__":
    import uvicorn

    uvicorn.run("app.main:app", host="localhost", port=8007, workers=1)
