"""
File: processes.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025 9:11:23 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

from pydantic import BaseModel


class CreateProcessRequest(BaseModel):
    name: str
